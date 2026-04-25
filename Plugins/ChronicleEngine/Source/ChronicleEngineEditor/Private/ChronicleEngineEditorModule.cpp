#include "Asset/ChronicleAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FChronicleEngineEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        RegisterAssetTypeAction(AssetTools, MakeShared<FAssetTypeActions_DialogueTree>());
        RegisterAssetTypeAction(AssetTools, MakeShared<FAssetTypeActions_DialogueDatabase>());
        RegisterAssetTypeAction(AssetTools, MakeShared<FAssetTypeActions_SpeakerProfile>());
    }

    virtual void ShutdownModule() override
    {
        if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
        {
            IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
            for (const TSharedPtr<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
            {
                if (Action.IsValid())
                {
                    AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
                }
            }
        }

        RegisteredAssetTypeActions.Reset();
    }

private:
    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
    {
        AssetTools.RegisterAssetTypeActions(Action);
        RegisteredAssetTypeActions.Add(Action);
    }

    TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
};

IMPLEMENT_MODULE(FChronicleEngineEditorModule, ChronicleEngineEditor)
