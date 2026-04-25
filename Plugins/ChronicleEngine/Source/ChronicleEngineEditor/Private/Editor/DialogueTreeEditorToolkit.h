#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class UDialogueTree;
class SDockTab;
class FSpawnTabArgs;

class FDialogueTreeEditorToolkit : public FAssetEditorToolkit, public FGCObject
{
public:
    void Initialize(UDialogueTree* InDialogueTree, const TSharedPtr<IToolkitHost>& InitToolkitHost);

    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;
    virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override;

private:
    TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& Args);

    TObjectPtr<UDialogueTree> DialogueTree;
    TSharedPtr<FWorkspaceItem> WorkspaceMenuCategory;
};
