#include "Misc/AutomationTest.h"

#include "Data/DialogueTree.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorAddNodeTest, "Chronicle.Editor.TreeEditor.AddNodeAndSearch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorAddNodeTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FGuid RootGuid;
    FString Error;
    TestTrue(TEXT("Root node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Root, FVector2D::ZeroVector, RootGuid, Error));
    TestEqual(TEXT("Root guid assigned"), Tree->RootNodeGuid, RootGuid);

    FGuid SpeechGuid;
    TestTrue(TEXT("Speech node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Speech, FVector2D(100.0f, 200.0f), SpeechGuid, Error));
    const FDialogueNode* SpeechNode = Tree->FindNode(SpeechGuid);
    TestNotNull(TEXT("Speech node exists"), SpeechNode);
    if (SpeechNode)
    {
        TestEqual(TEXT("Speech node has starter line"), SpeechNode->Lines.Num(), 1);
        TestEqual(TEXT("Speech node position round-trips"), SpeechNode->Position, FVector2D(100.0f, 200.0f));
    }

    TArray<FGuid> SearchResults;
    TestEqual(TEXT("Search finds starter line"), UChronicleDialogueEditorLibrary::SearchDialogueNodes(Tree, TEXT("New dialogue"), SearchResults), 1);
    TestEqual(TEXT("Search result is speech node"), SearchResults[0], SpeechGuid);

    FGuid DuplicateRootGuid;
    TestFalse(TEXT("Second root node is rejected"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Root, FVector2D::ZeroVector, DuplicateRootGuid, Error));
    TestTrue(TEXT("Duplicate root reports an error"), !Error.IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorNodeDisplayNameTest, "Chronicle.Editor.TreeEditor.NodeDisplayNames", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorNodeDisplayNameTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Speech display name"), UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(EDialogueNodeType::Speech).ToString(), FString(TEXT("Speech")));
    TestEqual(TEXT("Choice display name"), UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(EDialogueNodeType::Choice).ToString(), FString(TEXT("Choice")));
    TestEqual(TEXT("Condition display name"), UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(EDialogueNodeType::Condition).ToString(), FString(TEXT("Condition")));
    return true;
}

