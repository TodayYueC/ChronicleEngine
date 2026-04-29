#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Object.h"
#include "ChronicleExampleQuestAdapter.generated.h"

class UDialogueRunner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChronicleQuestAdapterEvent, FGameplayTag, QuestTag, const FDialogueEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChronicleStateAdapterEvent, FName, StateName, const FDialogueEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleGenericAdapterEvent, const FDialogueEventData&, EventData);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UChronicleExampleQuestAdapter : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Integration")
    void BindToRunner(UDialogueRunner* Runner);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Integration")
    void UnbindFromRunner();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Integration")
    void PushExternalVariable(FGameplayTag VariableTag, const FVariableValue& Value, EChronicleVariableScope Scope = EChronicleVariableScope::Global);

    UFUNCTION(BlueprintPure, Category="Chronicle|Integration")
    UDialogueRunner* GetBoundRunner() const { return BoundRunner; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Integration")
    int32 GetHandledEventCount() const { return HandledEventCount; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Integration")
    FDialogueEventData GetLastEvent() const { return LastEvent; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Integration")
    FGameplayTag GetLastQuestTag() const { return LastQuestTag; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Integration")
    FName GetLastStateName() const { return LastStateName; }

    UFUNCTION()
    void HandleDialogueEvent(const FDialogueEventData& EventData);

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleQuestAdapterEvent OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleQuestAdapterEvent OnQuestUpdated;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleQuestAdapterEvent OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleStateAdapterEvent OnGameStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleGenericAdapterEvent OnActorAnimationRequested;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleGenericAdapterEvent OnBattleEncounterRequested;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleGenericAdapterEvent OnSceneLoadRequested;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Integration")
    FOnChronicleGenericAdapterEvent OnUnhandledDialogueEvent;

private:
    FGameplayTag ResolvePayloadTag(const FDialogueEventData& EventData, FName Key) const;
    FName ResolvePayloadName(const FDialogueEventData& EventData, FName Key) const;
    bool MatchesEventTag(const FDialogueEventData& EventData, const TCHAR* TagName) const;

    UPROPERTY()
    TObjectPtr<UDialogueRunner> BoundRunner;

    UPROPERTY()
    int32 HandledEventCount = 0;

    UPROPERTY()
    FDialogueEventData LastEvent;

    UPROPERTY()
    FGameplayTag LastQuestTag;

    UPROPERTY()
    FName LastStateName;
};
