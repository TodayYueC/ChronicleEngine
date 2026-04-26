#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/ChronicleTypes.h"
#include "Presentation/DialoguePresenter.h"
#include "ChronicleDialogueWidget.generated.h"

class UChronicleDialoguePresentationController;
class UDialogueTree;

UCLASS(Abstract, Blueprintable)
class CHRONICLEENGINE_API UChronicleDialogueWidget : public UUserWidget, public IDialoguePresenter
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void BindPresentationController(UChronicleDialoguePresentationController* InController);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void UnbindPresentationController();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SetAutoAdvanceEnabled(bool bEnabled, float DelaySeconds = 1.5f);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void SetSkipModeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void RequestRollback(int32 Steps = 1);

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    UChronicleDialoguePresentationController* GetPresentationController() const { return Controller; }

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

    virtual void OnDialogueStarted_Implementation(UDialogueTree* Tree) override;
    virtual void OnDialogueEnded_Implementation(UDialogueTree* Tree) override;
    virtual void OnLineStarted_Implementation(const FDialogueLine& Line, ETextRevealMode RevealMode) override;
    virtual void OnLineCompleted_Implementation(const FDialogueLine& Line) override;
    virtual void OnChoicesPresented_Implementation(const TArray<FDialogueChoice>& Choices) override;
    virtual void OnDialogueEvent_Implementation(const FDialogueEventData& EventData) override;
    virtual void OnChoiceSelected_Implementation(int32 ChoiceIndex) override;
    virtual void OnWaitingForInput_Implementation() override;
    virtual void OnRollback_Implementation(const TArray<FDialogueHistoryEntry>& HistorySnapshot) override;
    virtual void HandleInlineTag_Implementation(const FGameplayTag& Tag, const FString& Params) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Widget")
    bool bDrivesPresentationTick = true;

protected:
    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Dialogue Started"))
    void BP_OnDialogueStarted(UDialogueTree* Tree);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Dialogue Ended"))
    void BP_OnDialogueEnded(UDialogueTree* Tree);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Line Started"))
    void BP_OnLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Line Completed"))
    void BP_OnLineCompleted(const FDialogueLine& Line);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Choices Presented"))
    void BP_OnChoicesPresented(const TArray<FDialogueChoice>& Choices);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Choice Selected"))
    void BP_OnChoiceSelected(int32 ChoiceIndex);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Waiting For Input"))
    void BP_OnWaitingForInput();

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Rollback"))
    void BP_OnRollback(const TArray<FDialogueHistoryEntry>& HistorySnapshot);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="On Dialogue Event"))
    void BP_OnDialogueEvent(const FDialogueEventData& EventData);

    UFUNCTION(BlueprintImplementableEvent, Category="Chronicle|Widget", meta=(DisplayName="Handle Inline Tag"))
    void BP_HandleInlineTag(const FGameplayTag& Tag, const FString& Params);

private:
    UPROPERTY(Transient, BlueprintReadOnly, Category="Chronicle|Widget", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UChronicleDialoguePresentationController> Controller;
};
