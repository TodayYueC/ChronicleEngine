#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Core/ChronicleTypes.h"
#include "ChronicleDialogueChoiceButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleDialogueChoiceButtonClicked, int32, ChoiceIndex);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UChronicleDialogueChoiceButton : public UButton
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Widget")
    void InitializeChoice(int32 InChoiceIndex, const FDialogueChoice& InChoice);

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    int32 GetChoiceIndex() const { return ChoiceIndex; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Widget")
    FDialogueChoice GetChoiceData() const { return ChoiceData; }

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Widget")
    FOnChronicleDialogueChoiceButtonClicked OnChoiceButtonClicked;

private:
    UFUNCTION()
    void HandleClicked();

    UPROPERTY(Transient)
    int32 ChoiceIndex = INDEX_NONE;

    UPROPERTY(Transient)
    FDialogueChoice ChoiceData;
};
