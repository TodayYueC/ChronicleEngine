#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "GameplayTagContainer.h"
#include "VariableBank.generated.h"

DECLARE_DELEGATE_RetVal_TwoParams(bool, FChronicleGetExternalVariableNative, FGameplayTag, FVariableValue&);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FChronicleSetExternalVariableNative, FGameplayTag, const FVariableValue&);

struct CHRONICLEENGINE_API FChronicleExternalVariableBinding
{
    FChronicleGetExternalVariableNative Getter;
    FChronicleSetExternalVariableNative Setter;
};

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UVariableBank : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    void InitializeFromDefinitions(const TArray<FVariableDefinition>& GlobalDefinitions, const TArray<FVariableDefinition>& LocalDefinitions);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    void SetVariable(FGameplayTag Tag, const FVariableValue& Value, EChronicleVariableScope Scope = EChronicleVariableScope::Global);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    FVariableValue GetVariable(FGameplayTag Tag, bool& bFound) const;

    bool GetVariableByName(FName VariableName, FVariableValue& OutValue) const;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    void ResetLocalVariables();

    void BindExternalVariable(FGameplayTag Tag, const FChronicleExternalVariableBinding& Binding);

    const TMap<FGameplayTag, FVariableValue>& GetGlobalVariables() const { return GlobalVariables; }
    const TMap<FGameplayTag, FVariableValue>& GetLocalVariables() const { return LocalVariables; }

    void RestoreSnapshot(const TMap<FGameplayTag, FVariableValue>& InGlobalVariables, const TMap<FGameplayTag, FVariableValue>& InLocalVariables);

private:
    UPROPERTY()
    TMap<FGameplayTag, FVariableValue> GlobalVariables;

    UPROPERTY()
    TMap<FGameplayTag, FVariableValue> LocalVariables;

    TMap<FGameplayTag, FChronicleExternalVariableBinding> ExternalBindings;
};

