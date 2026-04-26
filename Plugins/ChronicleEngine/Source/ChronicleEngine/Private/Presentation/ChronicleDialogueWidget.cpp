#include "Presentation/ChronicleDialogueWidget.h"

#include "Presentation/ChronicleDialoguePresentationController.h"

void UChronicleDialogueWidget::BindPresentationController(UChronicleDialoguePresentationController* InController)
{
    if (Controller == InController)
    {
        return;
    }

    UnbindPresentationController();
    Controller = InController;

    if (Controller)
    {
        Controller->AddPresenter(this);
    }
}

void UChronicleDialogueWidget::UnbindPresentationController()
{
    if (Controller)
    {
        Controller->RemovePresenter(this);
        Controller = nullptr;
    }
}

void UChronicleDialogueWidget::AdvanceDialogue()
{
    if (Controller)
    {
        Controller->Advance();
    }
}

void UChronicleDialogueWidget::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (Controller)
    {
        Controller->SelectChoice(ChoiceIndex);
    }
}

void UChronicleDialogueWidget::SetAutoAdvanceEnabled(bool bEnabled, float DelaySeconds)
{
    if (Controller)
    {
        Controller->SetAutoAdvanceEnabled(bEnabled, DelaySeconds);
    }
}

void UChronicleDialogueWidget::SetSkipModeEnabled(bool bEnabled)
{
    if (Controller)
    {
        Controller->SetSkipModeEnabled(bEnabled);
    }
}

void UChronicleDialogueWidget::RequestRollback(int32 Steps)
{
    if (Controller)
    {
        Controller->RequestRollback(Steps);
    }
}

void UChronicleDialogueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bDrivesPresentationTick && Controller)
    {
        Controller->TickPresentation(InDeltaTime);
    }
}

void UChronicleDialogueWidget::NativeDestruct()
{
    UnbindPresentationController();
    Super::NativeDestruct();
}

void UChronicleDialogueWidget::OnDialogueStarted_Implementation(UDialogueTree* Tree)
{
    BP_OnDialogueStarted(Tree);
}

void UChronicleDialogueWidget::OnDialogueEnded_Implementation(UDialogueTree* Tree)
{
    BP_OnDialogueEnded(Tree);
}

void UChronicleDialogueWidget::OnLineStarted_Implementation(const FDialogueLine& Line, ETextRevealMode RevealMode)
{
    BP_OnLineStarted(Line, RevealMode);
}

void UChronicleDialogueWidget::OnLineCompleted_Implementation(const FDialogueLine& Line)
{
    BP_OnLineCompleted(Line);
}

void UChronicleDialogueWidget::OnChoicesPresented_Implementation(const TArray<FDialogueChoice>& Choices)
{
    BP_OnChoicesPresented(Choices);
}

void UChronicleDialogueWidget::OnDialogueEvent_Implementation(const FDialogueEventData& EventData)
{
    BP_OnDialogueEvent(EventData);
}

void UChronicleDialogueWidget::OnChoiceSelected_Implementation(int32 ChoiceIndex)
{
    BP_OnChoiceSelected(ChoiceIndex);
}

void UChronicleDialogueWidget::OnWaitingForInput_Implementation()
{
    BP_OnWaitingForInput();
}

void UChronicleDialogueWidget::OnRollback_Implementation(const TArray<FDialogueHistoryEntry>& HistorySnapshot)
{
    BP_OnRollback(HistorySnapshot);
}

void UChronicleDialogueWidget::HandleInlineTag_Implementation(const FGameplayTag& Tag, const FString& Params)
{
    BP_HandleInlineTag(Tag, Params);
}
