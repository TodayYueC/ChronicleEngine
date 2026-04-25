#include "Data/DialogueDatabase.h"

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

