#include "Presentation/ChronicleDialogueDefaultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Presentation/ChronicleDialogueChoiceButton.h"
#include "Presentation/ChronicleDialoguePresentationController.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueDefaultWidget"

void UChronicleDialogueDefaultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ContinuePromptText.IsEmpty())
    {
        ContinuePromptText = LOCTEXT("ContinuePrompt", "Continue");
    }

    if (NoSpeakerText.IsEmpty())
    {
        NoSpeakerText = LOCTEXT("NoSpeaker", "Narrator");
    }

    if (bBuildDefaultLayout)
    {
        BuildDefaultLayout();
    }

    BindDefaultControls();
    RefreshDisplayedLine();
    RefreshChoiceList();
    RefreshBacklogList();
    RefreshModeStatus();
}

void UChronicleDialogueDefaultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    TickTextReveal(InDeltaTime);
}

void UChronicleDialogueDefaultWidget::NativeDestruct()
{
    UnbindDefaultControls();
    Super::NativeDestruct();
}

void UChronicleDialogueDefaultWidget::OnDialogueStarted_Implementation(UDialogueTree* Tree)
{
    Super::OnDialogueStarted_Implementation(Tree);
    ResetPresentationState();
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UChronicleDialogueDefaultWidget::OnDialogueEnded_Implementation(UDialogueTree* Tree)
{
    Super::OnDialogueEnded_Implementation(Tree);
    CurrentChoices.Reset();
    RefreshChoiceList();
    SetWaitingHintVisible(false);

    if (bHideWhenDialogueEnds)
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UChronicleDialogueDefaultWidget::OnLineStarted_Implementation(const FDialogueLine& Line, ETextRevealMode RevealMode)
{
    Super::OnLineStarted_Implementation(Line, RevealMode);

    CurrentLine = Line;
    CurrentChoices.Reset();
    CurrentRevealMode = RevealMode;
    RevealAccumulator = 0.0f;

    const FString FullText = CurrentLine.Text.ToString();
    bLineFullyRevealed = CurrentRevealMode == ETextRevealMode::Instant || TypewriterCharactersPerSecond <= 0.0f || FullText.IsEmpty();
    RevealedCharacterCount = bLineFullyRevealed ? FullText.Len() : 0;

    DisplayedSpeakerText = Line.SpeakerTag.IsValid()
        ? FText::FromString(Line.SpeakerTag.ToString())
        : NoSpeakerText;

    FDialogueHistoryEntry Entry;
    Entry.Timestamp = FDateTime::UtcNow();
    Entry.LineID = Line.LineID;
    Entry.SpeakerTag = Line.SpeakerTag;
    Entry.Text = Line.Text;
    LocalBacklog.Add(Entry);
    if (MaxBacklogEntries > 0 && LocalBacklog.Num() > MaxBacklogEntries)
    {
        LocalBacklog.RemoveAt(0, LocalBacklog.Num() - MaxBacklogEntries);
    }

    RefreshDisplayedLine();
    RefreshChoiceList();
    RefreshBacklogList();
    RefreshModeStatus();
    SetWaitingHintVisible(false);
}

void UChronicleDialogueDefaultWidget::OnLineCompleted_Implementation(const FDialogueLine& Line)
{
    Super::OnLineCompleted_Implementation(Line);
    RevealCurrentLineInstantly();
}

void UChronicleDialogueDefaultWidget::OnChoicesPresented_Implementation(const TArray<FDialogueChoice>& Choices)
{
    Super::OnChoicesPresented_Implementation(Choices);
    CurrentChoices = Choices;
    RevealCurrentLineInstantly();
    RefreshChoiceList();
    RefreshModeStatus();
    SetWaitingHintVisible(false);
}

void UChronicleDialogueDefaultWidget::OnWaitingForInput_Implementation()
{
    Super::OnWaitingForInput_Implementation();
    SetWaitingHintVisible(true);
}

void UChronicleDialogueDefaultWidget::OnRollback_Implementation(const TArray<FDialogueHistoryEntry>& HistorySnapshot)
{
    Super::OnRollback_Implementation(HistorySnapshot);
    LocalBacklog = HistorySnapshot;
    if (MaxBacklogEntries > 0 && LocalBacklog.Num() > MaxBacklogEntries)
    {
        LocalBacklog.RemoveAt(0, LocalBacklog.Num() - MaxBacklogEntries);
    }
    RefreshBacklogList();
}

void UChronicleDialogueDefaultWidget::RevealCurrentLineInstantly()
{
    const FString FullText = CurrentLine.Text.ToString();
    RevealedCharacterCount = FullText.Len();
    bLineFullyRevealed = true;
    RefreshDisplayedLine();
}

void UChronicleDialogueDefaultWidget::SetBacklogVisible(bool bVisible)
{
    bBacklogVisible = bVisible;
    if (BacklogScrollBox)
    {
        BacklogScrollBox->SetVisibility(bBacklogVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
    RefreshModeStatus();
}

void UChronicleDialogueDefaultWidget::ToggleBacklogVisible()
{
    SetBacklogVisible(!bBacklogVisible);
}

void UChronicleDialogueDefaultWidget::SetPortraitTexture(UTexture2D* Texture)
{
    if (PortraitImage)
    {
        PortraitImage->SetBrushFromTexture(Texture, true);
        PortraitImage->SetVisibility(Texture ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void UChronicleDialogueDefaultWidget::SetFullBodyTexture(UTexture2D* Texture)
{
    if (FullBodyImage)
    {
        FullBodyImage->SetBrushFromTexture(Texture, true);
        FullBodyImage->SetVisibility(Texture ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void UChronicleDialogueDefaultWidget::HandleAdvanceClicked()
{
    if (!bLineFullyRevealed)
    {
        RevealCurrentLineInstantly();
        return;
    }

    AdvanceDialogue();
}

void UChronicleDialogueDefaultWidget::HandleAutoClicked()
{
    if (UChronicleDialoguePresentationController* PresentationController = GetPresentationController())
    {
        SetAutoAdvanceEnabled(!PresentationController->IsAutoAdvanceEnabled(), AutoAdvanceDelaySeconds);
        RefreshModeStatus();
    }
}

void UChronicleDialogueDefaultWidget::HandleSkipClicked()
{
    if (UChronicleDialoguePresentationController* PresentationController = GetPresentationController())
    {
        SetSkipModeEnabled(!PresentationController->IsSkipModeEnabled());
        RefreshModeStatus();
    }
}

void UChronicleDialogueDefaultWidget::HandleBacklogClicked()
{
    ToggleBacklogVisible();
}

void UChronicleDialogueDefaultWidget::HandleRollbackClicked()
{
    RequestRollback(1);
}

void UChronicleDialogueDefaultWidget::HandleChoiceClicked(int32 ChoiceIndex)
{
    SelectDialogueChoice(ChoiceIndex);
}

void UChronicleDialogueDefaultWidget::BuildDefaultLayout()
{
    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ChronicleRoot"));
    WidgetTree->RootWidget = RootOverlay;

    UVerticalBox* RootStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootStack"));
    UOverlaySlot* RootStackSlot = RootOverlay->AddChildToOverlay(RootStack);
    RootStackSlot->SetHorizontalAlignment(HAlign_Fill);
    RootStackSlot->SetVerticalAlignment(VAlign_Bottom);

    UHorizontalBox* MainRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainRow"));
    UVerticalBoxSlot* MainRowSlot = RootStack->AddChildToVerticalBox(MainRow);
    MainRowSlot->SetHorizontalAlignment(HAlign_Fill);
    MainRowSlot->SetVerticalAlignment(VAlign_Bottom);

    PortraitImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("PortraitImage"));
    PortraitImage->SetVisibility(ESlateVisibility::Collapsed);
    UHorizontalBoxSlot* PortraitSlot = MainRow->AddChildToHorizontalBox(PortraitImage);
    PortraitSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    PortraitSlot->SetPadding(FMargin(8.0f));

    UBorder* DialoguePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DialoguePanel"));
    DialoguePanel->SetPadding(FMargin(16.0f));
    DialoguePanel->SetBrushColor(FLinearColor(0.015f, 0.018f, 0.022f, 0.88f));
    UHorizontalBoxSlot* DialoguePanelSlot = MainRow->AddChildToHorizontalBox(DialoguePanel);
    DialoguePanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    DialoguePanelSlot->SetPadding(FMargin(8.0f));

    UVerticalBox* DialogueStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DialogueStack"));
    DialoguePanel->SetContent(DialogueStack);

    SpeakerNameText = MakeTextBlock(TEXT("SpeakerNameText"), NoSpeakerText, 20.0f);
    DialogueStack->AddChildToVerticalBox(SpeakerNameText);

    DialogueLineText = MakeTextBlock(TEXT("DialogueLineText"), FText::GetEmpty(), 22.0f);
    DialogueLineText->SetAutoWrapText(true);
    UVerticalBoxSlot* LineSlot = DialogueStack->AddChildToVerticalBox(DialogueLineText);
    LineSlot->SetPadding(FMargin(0.0f, 8.0f));

    ChoiceList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ChoiceList"));
    UVerticalBoxSlot* ChoiceSlot = DialogueStack->AddChildToVerticalBox(ChoiceList);
    ChoiceSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));

    WaitingHintText = MakeTextBlock(TEXT("WaitingHintText"), ContinuePromptText, 14.0f);
    UVerticalBoxSlot* WaitingSlot = DialogueStack->AddChildToVerticalBox(WaitingHintText);
    WaitingSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));

    UHorizontalBox* Toolbar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Toolbar"));
    UVerticalBoxSlot* ToolbarSlot = DialogueStack->AddChildToVerticalBox(Toolbar);
    ToolbarSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));

    AdvanceButton = MakeToolbarButton(TEXT("AdvanceButton"), LOCTEXT("AdvanceButton", "Advance"));
    AutoButton = MakeToolbarButton(TEXT("AutoButton"), LOCTEXT("AutoButton", "Auto"));
    SkipButton = MakeToolbarButton(TEXT("SkipButton"), LOCTEXT("SkipButton", "Skip"));
    BacklogButton = MakeToolbarButton(TEXT("BacklogButton"), LOCTEXT("BacklogButton", "Backlog"));
    RollbackButton = MakeToolbarButton(TEXT("RollbackButton"), LOCTEXT("RollbackButton", "Back"));

    Toolbar->AddChildToHorizontalBox(AdvanceButton)->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
    Toolbar->AddChildToHorizontalBox(AutoButton)->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
    Toolbar->AddChildToHorizontalBox(SkipButton)->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
    Toolbar->AddChildToHorizontalBox(BacklogButton)->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
    Toolbar->AddChildToHorizontalBox(RollbackButton)->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));

    ModeStatusText = MakeTextBlock(TEXT("ModeStatusText"), FText::GetEmpty(), 14.0f);
    Toolbar->AddChildToHorizontalBox(ModeStatusText);

    BacklogScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("BacklogScrollBox"));
    BacklogScrollBox->SetVisibility(ESlateVisibility::Collapsed);
    UVerticalBoxSlot* BacklogSlot = RootStack->AddChildToVerticalBox(BacklogScrollBox);
    BacklogSlot->SetHorizontalAlignment(HAlign_Fill);
    BacklogSlot->SetPadding(FMargin(8.0f, 0.0f, 8.0f, 8.0f));

    FullBodyImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FullBodyImage"));
    FullBodyImage->SetVisibility(ESlateVisibility::Collapsed);
    UOverlaySlot* FullBodySlot = RootOverlay->AddChildToOverlay(FullBodyImage);
    FullBodySlot->SetHorizontalAlignment(HAlign_Right);
    FullBodySlot->SetVerticalAlignment(VAlign_Bottom);
}

void UChronicleDialogueDefaultWidget::BindDefaultControls()
{
    UnbindDefaultControls();

    if (AdvanceButton)
    {
        AdvanceButton->OnClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleAdvanceClicked);
    }
    if (AutoButton)
    {
        AutoButton->OnClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleAutoClicked);
    }
    if (SkipButton)
    {
        SkipButton->OnClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleSkipClicked);
    }
    if (BacklogButton)
    {
        BacklogButton->OnClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleBacklogClicked);
    }
    if (RollbackButton)
    {
        RollbackButton->OnClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleRollbackClicked);
    }
}

void UChronicleDialogueDefaultWidget::UnbindDefaultControls()
{
    if (AdvanceButton)
    {
        AdvanceButton->OnClicked.RemoveDynamic(this, &UChronicleDialogueDefaultWidget::HandleAdvanceClicked);
    }
    if (AutoButton)
    {
        AutoButton->OnClicked.RemoveDynamic(this, &UChronicleDialogueDefaultWidget::HandleAutoClicked);
    }
    if (SkipButton)
    {
        SkipButton->OnClicked.RemoveDynamic(this, &UChronicleDialogueDefaultWidget::HandleSkipClicked);
    }
    if (BacklogButton)
    {
        BacklogButton->OnClicked.RemoveDynamic(this, &UChronicleDialogueDefaultWidget::HandleBacklogClicked);
    }
    if (RollbackButton)
    {
        RollbackButton->OnClicked.RemoveDynamic(this, &UChronicleDialogueDefaultWidget::HandleRollbackClicked);
    }
}

void UChronicleDialogueDefaultWidget::ResetPresentationState()
{
    CurrentLine = FDialogueLine();
    CurrentChoices.Reset();
    LocalBacklog.Reset();
    DisplayedLineText = FText::GetEmpty();
    DisplayedSpeakerText = NoSpeakerText;
    CurrentRevealMode = ETextRevealMode::Instant;
    bLineFullyRevealed = true;
    RevealedCharacterCount = 0;
    RevealAccumulator = 0.0f;

    RefreshDisplayedLine();
    RefreshChoiceList();
    RefreshBacklogList();
    RefreshModeStatus();
}

void UChronicleDialogueDefaultWidget::TickTextReveal(float DeltaSeconds)
{
    if (bLineFullyRevealed || CurrentRevealMode == ETextRevealMode::Instant)
    {
        return;
    }

    const FString FullText = CurrentLine.Text.ToString();
    if (FullText.IsEmpty() || TypewriterCharactersPerSecond <= 0.0f)
    {
        RevealCurrentLineInstantly();
        return;
    }

    RevealAccumulator += FMath::Max(0.0f, DeltaSeconds) * TypewriterCharactersPerSecond;
    const int32 NewRevealedCount = FMath::Clamp(FMath::FloorToInt(RevealAccumulator), 0, FullText.Len());
    if (NewRevealedCount != RevealedCharacterCount)
    {
        RevealedCharacterCount = NewRevealedCount;
        bLineFullyRevealed = RevealedCharacterCount >= FullText.Len();
        RefreshDisplayedLine();
    }
}

void UChronicleDialogueDefaultWidget::RefreshDisplayedLine()
{
    const FString FullText = CurrentLine.Text.ToString();
    DisplayedLineText = FText::FromString(bLineFullyRevealed ? FullText : FullText.Left(RevealedCharacterCount));

    if (SpeakerNameText)
    {
        SpeakerNameText->SetText(DisplayedSpeakerText);
    }
    if (DialogueLineText)
    {
        DialogueLineText->SetText(DisplayedLineText);
    }
}

void UChronicleDialogueDefaultWidget::RefreshChoiceList()
{
    if (!ChoiceList)
    {
        return;
    }

    ChoiceList->ClearChildren();
    for (int32 ChoiceIndex = 0; ChoiceIndex < CurrentChoices.Num(); ++ChoiceIndex)
    {
        UChronicleDialogueChoiceButton* ChoiceButton = WidgetTree
            ? WidgetTree->ConstructWidget<UChronicleDialogueChoiceButton>(UChronicleDialogueChoiceButton::StaticClass())
            : NewObject<UChronicleDialogueChoiceButton>(this);
        ChoiceButton->InitializeChoice(ChoiceIndex, CurrentChoices[ChoiceIndex]);
        ChoiceButton->OnChoiceButtonClicked.AddDynamic(this, &UChronicleDialogueDefaultWidget::HandleChoiceClicked);

        UTextBlock* ChoiceText = WidgetTree
            ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
            : NewObject<UTextBlock>(this);
        ChoiceText->SetText(CurrentChoices[ChoiceIndex].Text);
        ChoiceText->SetAutoWrapText(true);
        ChoiceButton->AddChild(ChoiceText);

        UVerticalBoxSlot* ChoiceSlot = ChoiceList->AddChildToVerticalBox(ChoiceButton);
        ChoiceSlot->SetPadding(FMargin(0.0f, 3.0f));
    }
}

void UChronicleDialogueDefaultWidget::RefreshBacklogList()
{
    if (!BacklogScrollBox)
    {
        return;
    }

    BacklogScrollBox->ClearChildren();
    for (const FDialogueHistoryEntry& Entry : LocalBacklog)
    {
        const FString Speaker = Entry.SpeakerTag.IsValid() ? Entry.SpeakerTag.ToString() : NoSpeakerText.ToString();
        UTextBlock* EntryText = WidgetTree
            ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
            : NewObject<UTextBlock>(this);
        EntryText->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Speaker, *Entry.Text.ToString())));
        EntryText->SetAutoWrapText(true);
        BacklogScrollBox->AddChild(EntryText);
    }

    SetBacklogVisible(bBacklogVisible);
}

void UChronicleDialogueDefaultWidget::RefreshModeStatus()
{
    if (!ModeStatusText)
    {
        return;
    }

    const UChronicleDialoguePresentationController* PresentationController = GetPresentationController();
    const bool bAutoEnabled = PresentationController && PresentationController->IsAutoAdvanceEnabled();
    const bool bSkipEnabled = PresentationController && PresentationController->IsSkipModeEnabled();
    ModeStatusText->SetText(FText::Format(
        LOCTEXT("ModeStatus", "Auto: {0}  Skip: {1}  Backlog: {2}"),
        bAutoEnabled ? LOCTEXT("On", "On") : LOCTEXT("Off", "Off"),
        bSkipEnabled ? LOCTEXT("On", "On") : LOCTEXT("Off", "Off"),
        bBacklogVisible ? LOCTEXT("Shown", "Shown") : LOCTEXT("Hidden", "Hidden")));
}

void UChronicleDialogueDefaultWidget::SetWaitingHintVisible(bool bVisible)
{
    if (WaitingHintText)
    {
        WaitingHintText->SetText(ContinuePromptText);
        WaitingHintText->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}

UTextBlock* UChronicleDialogueDefaultWidget::MakeTextBlock(FName WidgetName, const FText& Text, float FontSize) const
{
    UTextBlock* TextBlock = WidgetTree
        ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName)
        : NewObject<UTextBlock>(const_cast<UChronicleDialogueDefaultWidget*>(this));
    TextBlock->SetText(Text);
    FSlateFontInfo FontInfo = TextBlock->GetFont();
    FontInfo.Size = FMath::RoundToInt(FontSize);
    TextBlock->SetFont(FontInfo);
    return TextBlock;
}

UButton* UChronicleDialogueDefaultWidget::MakeToolbarButton(FName WidgetName, const FText& Text) const
{
    UButton* Button = WidgetTree
        ? WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName)
        : NewObject<UButton>(const_cast<UChronicleDialogueDefaultWidget*>(this));
    UTextBlock* Label = MakeTextBlock(FName(*FString::Printf(TEXT("%sLabel"), *WidgetName.ToString())), Text, 14.0f);
    Button->AddChild(Label);
    return Button;
}

#undef LOCTEXT_NAMESPACE
