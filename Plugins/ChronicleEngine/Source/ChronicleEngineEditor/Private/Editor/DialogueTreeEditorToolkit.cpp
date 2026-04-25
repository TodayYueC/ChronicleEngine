#include "Editor/DialogueTreeEditorToolkit.h"

#include "Data/DialogueTree.h"
#include "Editor/SDialogueTreeEditor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "DialogueTreeEditorToolkit"

namespace ChronicleDialogueTreeEditor
{
static const FName AppIdentifier(TEXT("ChronicleDialogueTreeEditorApp"));
static const FName EditorTabId(TEXT("ChronicleDialogueTreeEditor_Graph"));
}

void FDialogueTreeEditorToolkit::Initialize(UDialogueTree* InDialogueTree, const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
    DialogueTree = InDialogueTree;

    const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("ChronicleDialogueTreeEditorLayout_v1")
        ->AddArea(
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Vertical)
            ->Split(
                FTabManager::NewStack()
                ->AddTab(ChronicleDialogueTreeEditor::EditorTabId, ETabState::OpenedTab)
                ->SetHideTabWell(true)
            )
        );

    InitAssetEditor(EToolkitMode::Standalone, InitToolkitHost, ChronicleDialogueTreeEditor::AppIdentifier, StandaloneDefaultLayout, true, true, InDialogueTree);
}

FName FDialogueTreeEditorToolkit::GetToolkitFName() const
{
    return FName(TEXT("ChronicleDialogueTreeEditor"));
}

FText FDialogueTreeEditorToolkit::GetBaseToolkitName() const
{
    return LOCTEXT("ToolkitName", "Chronicle Dialogue Tree");
}

FString FDialogueTreeEditorToolkit::GetWorldCentricTabPrefix() const
{
    return TEXT("Chronicle Dialogue Tree");
}

FLinearColor FDialogueTreeEditorToolkit::GetWorldCentricTabColorScale() const
{
    return FLinearColor(0.15f, 0.35f, 0.85f, 1.0f);
}

void FDialogueTreeEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu", "Chronicle"));
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(ChronicleDialogueTreeEditor::EditorTabId, FOnSpawnTab::CreateSP(this, &FDialogueTreeEditorToolkit::SpawnEditorTab))
        .SetDisplayName(LOCTEXT("EditorTab", "Dialogue Tree"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FDialogueTreeEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner(ChronicleDialogueTreeEditor::EditorTabId);
}

void FDialogueTreeEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(DialogueTree);
}

FString FDialogueTreeEditorToolkit::GetReferencerName() const
{
    return TEXT("FDialogueTreeEditorToolkit");
}

TSharedRef<SDockTab> FDialogueTreeEditorToolkit::SpawnEditorTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .Label(LOCTEXT("EditorTabLabel", "Dialogue Tree"))
        [
            SNew(SDialogueTreeEditor)
            .DialogueTree(DialogueTree.Get())
        ];
}

#undef LOCTEXT_NAMESPACE
