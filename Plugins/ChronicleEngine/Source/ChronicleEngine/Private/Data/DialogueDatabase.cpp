#include "Data/DialogueDatabase.h"

#include "Engine/DataTable.h"

bool UDialogueDatabase::FindVariableDefinition(FGameplayTag VariableTag, FVariableDefinition& OutDefinition) const
{
    for (const FVariableDefinition& Definition : GlobalVariables)
    {
        if (Definition.VariableTag == VariableTag)
        {
            OutDefinition = Definition;
            return true;
        }
    }

    return false;
}

UDataTable* UDialogueDatabase::ResolveVoiceTableForCulture(const FString& Culture) const
{
    const FString NormalizedCulture = Culture.TrimStartAndEnd();
    if (!NormalizedCulture.IsEmpty())
    {
        if (const TSoftObjectPtr<UDataTable>* CultureTable = CultureVoiceTables.Find(NormalizedCulture))
        {
            return CultureTable->LoadSynchronous();
        }
    }

    return VoiceTable.LoadSynchronous();
}
