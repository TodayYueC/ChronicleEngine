#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Object.h"
#include "DialogueRunner.generated.h"

class UDialogueDatabase;
class UDialogueTree;
class UVariableBank;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleDialogueStarted, UDialogueTree*, Tree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleDialogueEnded, UDialogueTree*, Tree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChronicleLineStarted, const FDialogueLine&, Line, ETextRevealMode, RevealMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleChoicesPresented, const TArray<FDialogueChoice>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleDialogueEvent, const FDialogueEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChronicleRunnerStateChanged, EDialogueRunnerState, OldState, EDialogueRunnerState, NewState);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UDialogueRunner : public UObject
{
    GENERATED_BODY()

public:
    UDialogueRunner();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void Initialize(UDialogueDatabase* InDatabase);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void StartDialogue(UDialogueTree* Tree, FName EntryNode = NAME_None);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void Advance();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void NotifyEventComplete(FGameplayTag EventTag);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    void SetVariable(FGameplayTag Tag, const FVariableValue& Value, EChronicleVariableScope Scope = EChronicleVariableScope::Global);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    FVariableValue GetVariable(FGameplayTag Tag, bool& bFound) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Save")
    void SaveState(FDialogueSaveData& OutData) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Save")
    void LoadState(const FDialogueSaveData& InData);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Rollback")
    void PerformRollback(int32 Steps = 1);

    UFUNCTION(BlueprintPure, Category="Chronicle|Dialogue")
    EDialogueRunnerState GetRunnerState() const { return RunnerState; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Dialogue")
    bool IsDialoguePlaying() const { return RunnerState != EDialogueRunnerState::Idle && CurrentTree != nullptr; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Dialogue")
    UVariableBank* GetVariableBank() const { return VariableBank; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Dialogue")
    UDialogueTree* GetCurrentTree() const { return CurrentTree; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Dialogue")
    FGuid GetCurrentNodeGuid() const { return CurrentNodeGuid; }

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleLineStarted OnLineStarted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleChoicesPresented OnChoicesPresented;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleDialogueEvent OnDialogueEvent;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChronicleRunnerStateChanged OnRunnerStateChanged;

private:
    void SetRunnerState(EDialogueRunnerState NewState);
    void ProcessCurrentNode();
    void PresentCurrentLine();
    bool MoveToNode(const FGuid& NodeGuid);
    bool FollowFirstEdge();
    bool FollowEdgeBySlot(int32 SlotIndex);
    bool FollowEdge(const FDialogueEdge& Edge);
    bool EvaluateEdgeCondition(const FDialogueEdge& Edge) const;
    bool EvaluateChoiceCondition(const FDialogueChoice& Choice) const;
    bool SelectConditionBranch(const FDialogueNode& Node, FDialogueEdge& OutEdge) const;
    void PushMemento();
    FString MakeSeenHash(const FDialogueLine& Line) const;

    UPROPERTY()
    TObjectPtr<UDialogueDatabase> Database;

    UPROPERTY()
    TObjectPtr<UDialogueTree> CurrentTree;

    UPROPERTY()
    TObjectPtr<UVariableBank> VariableBank;

    UPROPERTY()
    EDialogueRunnerState RunnerState = EDialogueRunnerState::Idle;

    UPROPERTY()
    FGuid CurrentNodeGuid;

    UPROPERTY()
    int32 CurrentLineIndex = INDEX_NONE;

    UPROPERTY()
    FGameplayTag WaitingEventTag;

    UPROPERTY()
    TArray<FDialogueHistoryEntry> History;

    UPROPERTY()
    TArray<FDialogueSaveData> MementoStack;

    UPROPERTY()
    TArray<FString> SeenDialogueHashes;

    UPROPERTY()
    TArray<int32> PresentedChoiceSlots;
};

