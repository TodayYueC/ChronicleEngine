#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Data/DialogueTrigger.h"
#include "Runtime/DialogueTriggerManager.h"
#include "UObject/Object.h"
#include "ChronicleTestListener.generated.h"

class UDialogueTree;

UCLASS()
class UChronicleTestListener : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int32 StartedCount = 0;

    UPROPERTY()
    int32 EndedCount = 0;

    UPROPERTY()
    int32 LineStartedCount = 0;

    UPROPERTY()
    int32 ChoicesPresentedCount = 0;

    UPROPERTY()
    int32 EventCount = 0;

    UPROPERTY()
    int32 TriggerActivatedCount = 0;

    UPROPERTY()
    int32 TriggerRejectedCount = 0;

    UPROPERTY()
    int32 QuestAdapterEventCount = 0;

    UPROPERTY()
    int32 GenericAdapterEventCount = 0;

    UPROPERTY()
    FGameplayTag LastAdapterQuestTag;

    UPROPERTY()
    FDialogueEventData LastAdapterEvent;

    UPROPERTY()
    FDialogueLine LastLine;

    UPROPERTY()
    TArray<FDialogueChoice> LastChoices;

    UPROPERTY()
    FDialogueEventData LastEvent;

    UPROPERTY()
    TArray<FDialogueEventData> EventHistory;

    UPROPERTY()
    FDialogueTriggerActivation LastTriggerActivation;

    UPROPERTY()
    FText LastTriggerRejectReason;

    UFUNCTION()
    void HandleDialogueStarted(UDialogueTree* Tree)
    {
        ++StartedCount;
    }

    UFUNCTION()
    void HandleDialogueEnded(UDialogueTree* Tree)
    {
        ++EndedCount;
    }

    UFUNCTION()
    void HandleLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode)
    {
        ++LineStartedCount;
        LastLine = Line;
    }

    UFUNCTION()
    void HandleChoicesPresented(const TArray<FDialogueChoice>& Choices)
    {
        ++ChoicesPresentedCount;
        LastChoices = Choices;
    }

    UFUNCTION()
    void HandleDialogueEvent(const FDialogueEventData& EventData)
    {
        ++EventCount;
        LastEvent = EventData;
        EventHistory.Add(EventData);
    }

    UFUNCTION()
    void HandleTriggerActivated(const FDialogueTriggerActivation& Activation)
    {
        ++TriggerActivatedCount;
        LastTriggerActivation = Activation;
    }

    UFUNCTION()
    void HandleTriggerRejected(UDialogueTrigger* Trigger, const FText& Reason)
    {
        ++TriggerRejectedCount;
        LastTriggerRejectReason = Reason;
    }

    UFUNCTION()
    void HandleQuestAdapterEvent(FGameplayTag QuestTag, const FDialogueEventData& EventData)
    {
        ++QuestAdapterEventCount;
        LastAdapterQuestTag = QuestTag;
        LastAdapterEvent = EventData;
    }

    UFUNCTION()
    void HandleGenericAdapterEvent(const FDialogueEventData& EventData)
    {
        ++GenericAdapterEventCount;
        LastAdapterEvent = EventData;
    }
};
