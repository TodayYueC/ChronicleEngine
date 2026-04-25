#include "Asset/ChronicleAssetTypeActions.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Data/SpeakerProfile.h"

#define LOCTEXT_NAMESPACE "ChronicleAssetTypeActions"

FText FAssetTypeActions_DialogueTree::GetName() const
{
    return LOCTEXT("DialogueTreeName", "Dialogue Tree");
}

UClass* FAssetTypeActions_DialogueTree::GetSupportedClass() const
{
    return UDialogueTree::StaticClass();
}

FText FAssetTypeActions_DialogueDatabase::GetName() const
{
    return LOCTEXT("DialogueDatabaseName", "Dialogue Database");
}

UClass* FAssetTypeActions_DialogueDatabase::GetSupportedClass() const
{
    return UDialogueDatabase::StaticClass();
}

FText FAssetTypeActions_SpeakerProfile::GetName() const
{
    return LOCTEXT("SpeakerProfileName", "Speaker Profile");
}

UClass* FAssetTypeActions_SpeakerProfile::GetSupportedClass() const
{
    return USpeakerProfile::StaticClass();
}

#undef LOCTEXT_NAMESPACE

