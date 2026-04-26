#include "Runtime/VariableBank.h"

#include "GameplayTagsManager.h"

void UVariableBank::InitializeFromDefinitions(const TArray<FVariableDefinition>& GlobalDefinitions, const TArray<FVariableDefinition>& LocalDefinitions)
{
    LocalVariables.Reset();

    for (const FVariableDefinition& Definition : GlobalDefinitions)
    {
        if (!Definition.VariableTag.IsValid())
        {
            continue;
        }

        if (Definition.Scope == EChronicleVariableScope::Local)
        {
            LocalVariables.Add(Definition.VariableTag, Definition.DefaultValue);
        }
        else
        {
            if (!GlobalVariables.Contains(Definition.VariableTag))
            {
                GlobalVariables.Add(Definition.VariableTag, Definition.DefaultValue);
            }
        }
    }

    for (const FVariableDefinition& Definition : LocalDefinitions)
    {
        if (Definition.VariableTag.IsValid())
        {
            LocalVariables.Add(Definition.VariableTag, Definition.DefaultValue);
        }
    }
}

void UVariableBank::SetVariable(FGameplayTag Tag, const FVariableValue& Value, EChronicleVariableScope Scope)
{
    if (!Tag.IsValid())
    {
        return;
    }

    if (Scope == EChronicleVariableScope::External)
    {
        if (FChronicleExternalVariableBinding* Binding = ExternalBindings.Find(Tag))
        {
            if (Binding->Setter.IsBound())
            {
                Binding->Setter.Execute(Tag, Value);
            }
        }
        return;
    }

    if (Scope == EChronicleVariableScope::Local)
    {
        LocalVariables.Add(Tag, Value);
        return;
    }

    GlobalVariables.Add(Tag, Value);
}

FVariableValue UVariableBank::GetVariable(FGameplayTag Tag, bool& bFound) const
{
    bFound = false;

    if (!Tag.IsValid())
    {
        return FVariableValue();
    }

    if (const FVariableValue* LocalValue = LocalVariables.Find(Tag))
    {
        bFound = true;
        return *LocalValue;
    }

    if (const FVariableValue* GlobalValue = GlobalVariables.Find(Tag))
    {
        bFound = true;
        return *GlobalValue;
    }

    if (const FChronicleExternalVariableBinding* Binding = ExternalBindings.Find(Tag))
    {
        if (Binding->Getter.IsBound())
        {
            FVariableValue ExternalValue;
            bFound = Binding->Getter.Execute(Tag, ExternalValue);
            return ExternalValue;
        }
    }

    return FVariableValue();
}

bool UVariableBank::GetVariableByName(FName VariableName, FVariableValue& OutValue) const
{
    FGameplayTag Tag;
    if (const FGameplayTag* CachedTag = VariableNameCache.Find(VariableName))
    {
        Tag = *CachedTag;
    }
    else
    {
        Tag = UGameplayTagsManager::Get().RequestGameplayTag(VariableName, false);
        VariableNameCache.Add(VariableName, Tag);
    }

    if (!Tag.IsValid())
    {
        return false;
    }

    bool bFound = false;
    OutValue = GetVariable(Tag, bFound);
    return bFound;
}

void UVariableBank::ResetLocalVariables()
{
    LocalVariables.Reset();
}

void UVariableBank::BindExternalVariable(FGameplayTag Tag, const FChronicleExternalVariableBinding& Binding)
{
    if (Tag.IsValid())
    {
        ExternalBindings.Add(Tag, Binding);
    }
}

void UVariableBank::RestoreSnapshot(const TMap<FGameplayTag, FVariableValue>& InGlobalVariables, const TMap<FGameplayTag, FVariableValue>& InLocalVariables)
{
    GlobalVariables = InGlobalVariables;
    LocalVariables = InLocalVariables;
}
