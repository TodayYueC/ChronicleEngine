#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
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
    FDialogueLine LastLine;

    UPROPERTY()
    TArray<FDialogueChoice> LastChoices;

    UPROPERTY()
    FDialogueEventData LastEvent;

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
    }
};
