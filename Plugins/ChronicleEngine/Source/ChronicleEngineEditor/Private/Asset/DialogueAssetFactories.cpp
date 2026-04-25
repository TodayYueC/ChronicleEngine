#include "Asset/DialogueAssetFactories.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "Data/SpeakerProfile.h"

UDialogueTreeFactory::UDialogueTreeFactory()
{
    SupportedClass = UDialogueTree::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* UDialogueTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>(InParent, Class, Name, Flags);

    FDialogueNode RootNode;
    RootNode.NodeGuid = FGuid::NewGuid();
    RootNode.NodeType = EDialogueNodeType::Root;
    Tree->Nodes.Add(RootNode);
    Tree->RootNodeGuid = RootNode.NodeGuid;
    Tree->EnsureStableGuids();

    return Tree;
}

UDialogueDatabaseFactory::UDialogueDatabaseFactory()
{
    SupportedClass = UDialogueDatabase::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* UDialogueDatabaseFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UDialogueDatabase>(InParent, Class, Name, Flags);
}

USpeakerProfileFactory::USpeakerProfileFactory()
{
    SupportedClass = USpeakerProfile::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USpeakerProfileFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USpeakerProfile>(InParent, Class, Name, Flags);
}

