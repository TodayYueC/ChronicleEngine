#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Object.h"
#include "DialogueTriggerManager.generated.h"

class UChronicleDialoguePresentationController;
class UDialogueRunner;
class UDialogueTree;
class UDialogueTrigger;

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueTriggerActivation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    TObjectPtr<UDialogueTrigger> Trigger;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    FGameplayTag TriggerTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    TObjectPtr<UDialogueTree> TargetTree;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    FName EntryNode;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    EDialogueTriggerType TriggerType = EDialogueTriggerType::Interact;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    int32 Priority = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Trigger")
    TObjectPtr<UObject> Instigator;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleDialogueTriggerActivated, const FDialogueTriggerActivation&, Activation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChronicleDialogueTriggerRejected, UDialogueTrigger*, Trigger, const FText&, Reason);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UDialogueTriggerManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void Initialize(UDialogueRunner* InDialogueRunner, UChronicleDialoguePresentationController* InPresentationController);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void Deinitialize();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void RegisterTrigger(UDialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool UnregisterTrigger(UDialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void ClearTriggers();

    UFUNCTION(BlueprintPure, Category="Chronicle|Trigger")
    int32 GetRegisteredTriggerCount() const { return RegisteredTriggers.Num(); }

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void GetRegisteredTriggers(TArray<UDialogueTrigger*>& OutTriggers) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool CanActivateTrigger(UDialogueTrigger* Trigger, UObject* Instigator, FText& OutFailureReason) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool TryActivateTrigger(UDialogueTrigger* Trigger, UObject* Instigator = nullptr, bool bStartDialogue = true);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool TryActivateTriggerByTag(FGameplayTag TriggerTag, UObject* Instigator = nullptr, bool bStartDialogue = true);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool TryActivateBestTrigger(const TArray<UDialogueTrigger*>& CandidateTriggers, EDialogueTriggerType RequiredType, UObject* Instigator = nullptr, bool bStartDialogue = true);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    bool EvaluateAutoTriggers(bool bStartDialogue = true);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void SetTriggerConsumed(FGameplayTag TriggerTag, bool bConsumed);

    UFUNCTION(BlueprintPure, Category="Chronicle|Trigger")
    bool IsTriggerConsumed(FGameplayTag TriggerTag) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Trigger")
    void ResetRuntimeState();

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Trigger")
    FOnChronicleDialogueTriggerActivated OnTriggerActivated;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Trigger")
    FOnChronicleDialogueTriggerRejected OnTriggerRejected;

private:
    UFUNCTION()
    void HandleDialogueEnded(UDialogueTree* FinishedTree);

    UDialogueTree* ResolveTargetTree(UDialogueTrigger* Trigger) const;
    bool EvaluateActivationConditions(UDialogueTrigger* Trigger, FText& OutFailureReason) const;
    void RejectTrigger(UDialogueTrigger* Trigger, const FText& Reason);

    UPROPERTY()
    TObjectPtr<UDialogueRunner> DialogueRunner;

    UPROPERTY()
    TObjectPtr<UChronicleDialoguePresentationController> PresentationController;

    UPROPERTY()
    TArray<TObjectPtr<UDialogueTrigger>> RegisteredTriggers;

    UPROPERTY()
    TObjectPtr<UDialogueTrigger> ActiveTrigger;

    UPROPERTY()
    TSet<FGameplayTag> ConsumedOneShotTriggers;

    TMap<FGameplayTag, double> LastActivationSeconds;
};
