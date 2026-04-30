#include "Asset/ChronicleAssetTypeActions.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Data/DialogueTrigger.h"
#include "Data/SpeakerProfile.h"
#include "DesktopPlatformModule.h"
#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Editor/DialogueTreeEditorToolkit.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDesktopPlatform.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "ChronicleAssetTypeActions"

FText FAssetTypeActions_DialogueTree::GetName() const
{
    return LOCTEXT("DialogueTreeName", "Dialogue Tree");
}

UClass* FAssetTypeActions_DialogueTree::GetSupportedClass() const
{
    return UDialogueTree::StaticClass();
}

void FAssetTypeActions_DialogueTree::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
    const TArray<TWeakObjectPtr<UDialogueTree>> Trees = GetTypedWeakObjectPtrs<UDialogueTree>(InObjects);

    MenuBuilder.AddMenuSeparator(TEXT("ChronicleDialoguePipeline"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("ExportPipelineArtifacts", "Export Chronicle Pipeline Artifacts..."),
        LOCTEXT("ExportPipelineArtifactsTooltip", "Export JSON, dialogue line CSV, localization CSV, and audit JSON for the selected Dialogue Tree assets."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_DialogueTree::ExecuteExportPipelineArtifacts, Trees)));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("ImportScriptCsv", "Import Chronicle Script CSV..."),
        LOCTEXT("ImportScriptCsvTooltip", "Replace the selected Dialogue Tree with nodes and edges generated from an Excel-authored CSV script."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_DialogueTree::ExecuteImportScriptCsv, Trees)));
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

void FAssetTypeActions_DialogueTree::ExecuteExportPipelineArtifacts(TArray<TWeakObjectPtr<UDialogueTree>> Trees) const
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    FString ExportDirectory = FPaths::ProjectSavedDir() / TEXT("ChronicleExports");

    if (DesktopPlatform)
    {
        FString SelectedDirectory;
        const void* ParentWindowHandle = FSlateApplication::IsInitialized() ? FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr) : nullptr;
        if (!DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, TEXT("Choose Chronicle export directory"), ExportDirectory, SelectedDirectory))
        {
            return;
        }
        if (!SelectedDirectory.IsEmpty())
        {
            ExportDirectory = SelectedDirectory;
        }
    }

    for (const TWeakObjectPtr<UDialogueTree>& TreePtr : Trees)
    {
        UDialogueTree* Tree = TreePtr.Get();
        if (!Tree)
        {
            continue;
        }

        FChronicleDialoguePipelineExportPaths ExportPaths;
        FString Error;
        if (UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts(Tree, ExportDirectory, TEXT("en"), ExportPaths, Error))
        {
            UE_LOG(LogTemp, Display, TEXT("Chronicle pipeline artifacts exported for %s to %s."), *Tree->GetName(), *ExportDirectory);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Chronicle pipeline export failed for %s: %s"), *Tree->GetName(), *Error);
        }
    }
}

void FAssetTypeActions_DialogueTree::ExecuteImportScriptCsv(TArray<TWeakObjectPtr<UDialogueTree>> Trees) const
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        return;
    }

    TArray<FString> SelectedFiles;
    const void* ParentWindowHandle = FSlateApplication::IsInitialized() ? FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr) : nullptr;
    if (!DesktopPlatform->OpenFileDialog(
        ParentWindowHandle,
        TEXT("Import Chronicle Script CSV"),
        FPaths::ProjectContentDir(),
        FString(),
        TEXT("CSV files (*.csv)|*.csv|All files (*.*)|*.*"),
        EFileDialogFlags::None,
        SelectedFiles) || SelectedFiles.Num() == 0)
    {
        return;
    }

    const FString CsvFilePath = SelectedFiles[0];
    for (const TWeakObjectPtr<UDialogueTree>& TreePtr : Trees)
    {
        UDialogueTree* Tree = TreePtr.Get();
        if (!Tree)
        {
            continue;
        }

        FString Error;
        if (UChronicleDialogueJsonLibrary::ImportDialogueScriptCsvFile(Tree, CsvFilePath, true, Error))
        {
            UE_LOG(LogTemp, Display, TEXT("Chronicle script CSV imported into %s from %s."), *Tree->GetName(), *CsvFilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Chronicle script CSV import failed for %s: %s"), *Tree->GetName(), *Error);
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
