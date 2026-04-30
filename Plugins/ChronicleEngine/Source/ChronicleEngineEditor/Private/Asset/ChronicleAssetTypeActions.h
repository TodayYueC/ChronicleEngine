#pragma once

#include "AssetTypeActions_Base.h"

class IToolkitHost;
class UDialogueTree;

class FChronicleAssetTypeActionsBase : public FAssetTypeActions_Base
{
public:
    virtual FColor GetTypeColor() const override { return FColor(64, 156, 255); }
    virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

class FAssetTypeActions_DialogueTree final : public FChronicleAssetTypeActionsBase
{
public:
    virtual FText GetName() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:
    void ExecuteExportPipelineArtifacts(TArray<TWeakObjectPtr<UDialogueTree>> Trees) const;
    void ExecuteImportScriptCsv(TArray<TWeakObjectPtr<UDialogueTree>> Trees) const;
};

class FAssetTypeActions_DialogueDatabase final : public FChronicleAssetTypeActionsBase
{
public:
    virtual FText GetName() const override;
    virtual UClass* GetSupportedClass() const override;
};

class FAssetTypeActions_SpeakerProfile final : public FChronicleAssetTypeActionsBase
{
public:
    virtual FText GetName() const override;
    virtual UClass* GetSupportedClass() const override;
};

class FAssetTypeActions_DialogueTrigger final : public FChronicleAssetTypeActionsBase
{
public:
    virtual FText GetName() const override;
    virtual UClass* GetSupportedClass() const override;
};
