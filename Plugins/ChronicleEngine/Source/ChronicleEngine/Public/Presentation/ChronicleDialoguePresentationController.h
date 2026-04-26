#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Object.h"
#include "ChronicleDialoguePresentationController.generated.h"

class UDialogueRunner;
class UDialogueTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChroniclePresentationLineStarted, const FDialogueLine&, Line, ETextRevealMode, RevealMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationChoicesPresented, const TArray<FDialogueChoice>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationBacklogChanged, const TArray<FDialogueHistoryEntry>&, Backlog);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationDialogueStarted, UDialogueTree*, Tree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationDialogueEnded, UDialogueTree*, Tree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationEvent, const FDialogueEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChroniclePresentationRunnerStateChanged, EDialogueRunnerState, OldState, EDialogueRunnerState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChroniclePresentationAutoChanged, bool, bAutoAdvanceEnabled, float, AutoAdvanceDelay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChroniclePresentationSkipChanged, bool, bSkipModeEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChroniclePresentationRollbackPerformed, int32, Steps, FGuid, CurrentNodeGuid);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UChronicleDialoguePresentationController : public UObject
{
    GENERATED_BODY()

public:
    UChronicleDialoguePresentationController();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void BindRunner(UDialogueRunner* InRunner);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void UnbindRunner();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    bool AddPresenter(UObject* Presenter);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void RemovePresenter(UObject* Presenter);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void StartDialogue(UDialogueTree* Tree, FName EntryNode = NAME_None);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void Advance();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void NotifyEventComplete(FGameplayTag EventTag);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void SetAutoAdvanceEnabled(bool bEnabled, float DelaySeconds = 1.5f);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void SetSkipModeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void TickPresentation(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void RequestRollback(int32 Steps = 1);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void ClearBacklog();

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    UDialogueRunner* GetRunner() const { return Runner; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    bool IsAutoAdvanceEnabled() const { return bAutoAdvanceEnabled; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    bool IsSkipModeEnabled() const { return bSkipModeEnabled; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    float GetAutoAdvanceDelay() const { return AutoAdvanceDelay; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    TArray<FDialogueHistoryEntry> GetBacklog() const { return Backlog; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    TArray<FDialogueChoice> GetPresentedChoices() const { return PresentedChoices; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    FDialogueLine GetLastLine() const { return LastLine; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    FDialogueEventData GetLastEventData() const { return LastEventData; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    ETextRevealMode GetLastRevealMode() const { return LastRevealMode; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    int32 MaxSkipStepsPerTick = 32;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationDialogueStarted OnPresentationDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationDialogueEnded OnPresentationDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationLineStarted OnPresentationLineStarted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationChoicesPresented OnPresentationChoicesPresented;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationBacklogChanged OnBacklogChanged;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationEvent OnPresentationEvent;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationRunnerStateChanged OnPresentationRunnerStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationAutoChanged OnAutoAdvanceChanged;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationSkipChanged OnSkipModeChanged;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChroniclePresentationRollbackPerformed OnRollbackPerformed;

private:
    UFUNCTION()
    void HandleDialogueStarted(UDialogueTree* Tree);

    UFUNCTION()
    void HandleDialogueEnded(UDialogueTree* Tree);

    UFUNCTION()
    void HandleLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode);

    UFUNCTION()
    void HandleChoicesPresented(const TArray<FDialogueChoice>& Choices);

    UFUNCTION()
    void HandleDialogueEvent(const FDialogueEventData& EventData);

    UFUNCTION()
    void HandleRunnerStateChanged(EDialogueRunnerState OldState, EDialogueRunnerState NewState);

    void BroadcastLineCompleted();
    void DrainSkip();
    void SyncBacklogFromRunner();
    void ForEachPresenter(TFunctionRef<void(UObject*)> Callback);

    UPROPERTY()
    TObjectPtr<UDialogueRunner> Runner;

    UPROPERTY()
    TArray<FDialogueHistoryEntry> Backlog;

    UPROPERTY()
    TArray<FDialogueChoice> PresentedChoices;

    UPROPERTY()
    FDialogueLine LastLine;

    UPROPERTY()
    FDialogueEventData LastEventData;

    UPROPERTY()
    bool bHasLastLine = false;

    UPROPERTY()
    bool bAutoAdvanceEnabled = false;

    UPROPERTY()
    bool bSkipModeEnabled = false;

    UPROPERTY()
    float AutoAdvanceDelay = 1.5f;

    UPROPERTY()
    float AutoAdvanceAccumulator = 0.0f;

    UPROPERTY()
    ETextRevealMode LastRevealMode = ETextRevealMode::Typewriter;

    TArray<TWeakObjectPtr<UObject>> PresenterObjects;
};
