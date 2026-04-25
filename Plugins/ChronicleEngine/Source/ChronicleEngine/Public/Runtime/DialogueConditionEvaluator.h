#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogueConditionEvaluator.generated.h"

class UVariableBank;

UCLASS()
class CHRONICLEENGINE_API UDialogueConditionEvaluator : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Conditions")
    static bool EvaluateCondition(const FString& Expression, const UVariableBank* VariableBank, bool& bSuccess);
};

