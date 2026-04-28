#include "Asset/ChronicleAssetTypeActions.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Data/DialogueTrigger.h"
#include "Data/SpeakerProfile.h"
#include "Editor/DialogueTreeEditorToolkit.h"

#define LOCTEXT_NAMESPACE "ChronicleAssetTypeActions"

FText FAssetTypeActions_DialogueTree::GetName() const
{
    return LOCTEXT("DialogueTreeName", "Dialogue Tree");
}

UClass* FAssetTypeActions_DialogueTree::GetSupportedClass() const
{
    return UDialogueTree::StaticClass();
}

void FAssetTypeActions_DialogueTree::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    for (UObject* Object : InObjects)
    {
        if (UDialogueTree* DialogueTree = Cast<UDialogueTree>(Object))
        {
            const TSharedRef<FDialogueTreeEditorToolkit> EditorToolkit = MakeShared<FDialogueTreeEditorToolkit>();
            EditorToolkit->Initialize(DialogueTree, EditWithinLevelEditor);
        }
    }
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

FText FAssetTypeActions_DialogueTrigger::GetName() const
{
    return LOCTEXT("DialogueTriggerName", "Dialogue Trigger");
}

UClass* FAssetTypeActions_DialogueTrigger::GetSupportedClass() const
{
    return UDialogueTrigger::StaticClass();
}

#undef LOCTEXT_NAMESPACE
