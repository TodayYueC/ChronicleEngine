#pragma once

#include "CoreMinimal.h"
#include "Presentation/ChronicleDialogueWidget.h"
#include "ChronicleDialogueDefaultWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UTexture2D;
class UVerticalBox;

UCLASS(Blueprintable)
class CHRONICLEENGINE_API UChronicleDialogueDefaultWidget : public UChronicleDialogueWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

    virtual void OnDialogueStarted_Implementation(UDialogueTree* Tree) override;
    virtual void OnDialogueEnded_Implementation(UDialogueTree* Tree) override;
    virtual void OnLineStarted_Implementation(const FDialogueLine& Line, ETextRevealMode RevealMode) override;
    virtual void OnLineCompleted_Implementation(const FDialogueLine& Line) override;
    virtual void OnChoicesPresented_Implementation(const TArray<FDialogueChoice>& Choices) override;
    virtual void OnWaitingForInput_Implementation() override;
    virtual void OnRollback_Implementation(const TArray<FDialogueHistoryEntry>& HistorySnapshot) override;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void RevealCurrentLineInstantly();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SetBacklogVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void ToggleBacklogVisible();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SetPortraitTexture(UTexture2D* Texture);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SetFullBodyTexture(UTexture2D* Texture);

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    FText GetDisplayedLineText() const { return DisplayedLineText; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    FText GetDisplayedSpeakerText() const { return DisplayedSpeakerText; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    bool IsCurrentLineFullyRevealed() const { return bLineFullyRevealed; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    bool IsBacklogVisible() const { return bBacklogVisible; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    int32 GetVisibleChoiceCount() const { return CurrentChoices.Num(); }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    TArray<FDialogueHistoryEntry> GetLocalBacklog() const { return LocalBacklog; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    bool bBuildDefaultLayout = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    bool bHideWhenDialogueEnds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget", meta=(ClampMin="0.0"))
    float TypewriterCharactersPerSecond = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget", meta=(ClampMin="1"))
    int32 MaxBacklogEntries = 80;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    float AutoAdvanceDelaySeconds = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    FText ContinuePromptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    FText NoSpeakerText;

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UTextBlock> SpeakerNameText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UTextBlock> DialogueLineText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UTextBlock> WaitingHintText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UTextBlock> ModeStatusText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UImage> PortraitImage;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UImage> FullBodyImage;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UVerticalBox> ChoiceList;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UScrollBox> BacklogScrollBox;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UButton> AdvanceButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UButton> AutoButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UButton> SkipButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UButton> BacklogButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Chronicle|Widget")
    TObjectPtr<UButton> RollbackButton;

private:
    UFUNCTION()
    void HandleAdvanceClicked();

    UFUNCTION()
    void HandleAutoClicked();

    UFUNCTION()
    void HandleSkipClicked();

    UFUNCTION()
    void HandleBacklogClicked();

    UFUNCTION()
    void HandleRollbackClicked();

    UFUNCTION()
    void HandleChoiceClicked(int32 ChoiceIndex);

    void BuildDefaultLayout();
    void BindDefaultControls();
    void UnbindDefaultControls();
    void ResetPresentationState();
    void TickTextReveal(float DeltaSeconds);
    void RefreshDisplayedLine();
    void RefreshChoiceList();
    void RefreshBacklogList();
    void RefreshModeStatus();
    void SetWaitingHintVisible(bool bVisible);
    UTextBlock* MakeTextBlock(FName WidgetName, const FText& Text, float FontSize = 18.0f) const;
    UButton* MakeToolbarButton(FName WidgetName, const FText& Text) const;

    UPROPERTY(Transient)
    FDialogueLine CurrentLine;

    UPROPERTY(Transient)
    TArray<FDialogueChoice> CurrentChoices;

    UPROPERTY(Transient)
    TArray<FDialogueHistoryEntry> LocalBacklog;

    UPROPERTY(Transient)
    FText DisplayedLineText;

    UPROPERTY(Transient)
    FText DisplayedSpeakerText;

    UPROPERTY(Transient)
    ETextRevealMode CurrentRevealMode = ETextRevealMode::Instant;

    UPROPERTY(Transient)
    bool bLineFullyRevealed = true;

    UPROPERTY(Transient)
    bool bBacklogVisible = false;

    UPROPERTY(Transient)
    int32 RevealedCharacterCount = 0;

    UPROPERTY(Transient)
    float RevealAccumulator = 0.0f;
};
