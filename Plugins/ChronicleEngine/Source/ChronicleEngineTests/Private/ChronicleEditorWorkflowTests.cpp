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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorEdgeEditingTest, "Chronicle.Editor.TreeEditor.EdgeEditing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorEdgeEditingTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FString Error;
    FGuid RootGuid;
    FGuid SpeechGuid;
    FGuid ChoiceGuid;
    TestTrue(TEXT("Root node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Root, FVector2D::ZeroVector, RootGuid, Error));
    TestTrue(TEXT("Speech node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Speech, FVector2D(100.0f, 200.0f), SpeechGuid, Error));
    TestTrue(TEXT("Choice node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Choice, FVector2D(300.0f, 200.0f), ChoiceGuid, Error));

    TestTrue(TEXT("Node position can be edited"), UChronicleDialogueEditorLibrary::SetDialogueNodePosition(Tree, SpeechGuid, FVector2D(420.0f, 240.0f), Error));
    const FDialogueNode* MovedNode = Tree->FindNode(SpeechGuid);
    TestNotNull(TEXT("Moved node exists"), MovedNode);
    if (MovedNode)
    {
        TestEqual(TEXT("Moved position persisted"), MovedNode->Position, FVector2D(420.0f, 240.0f));
    }

    FDialogueEdge RootToSpeech;
    TestTrue(TEXT("Edge can be added"), UChronicleDialogueEditorLibrary::AddDialogueEdge(Tree, RootGuid, SpeechGuid, 0, TEXT(""), RootToSpeech, Error));
    TestEqual(TEXT("One edge exists"), Tree->Edges.Num(), 1);
    TestEqual(TEXT("Edge source persisted"), Tree->Edges[0].FromNodeGuid, RootGuid);
    TestEqual(TEXT("Edge target persisted"), Tree->Edges[0].ToNodeGuid, SpeechGuid);
    TestEqual(TEXT("Edge slot persisted"), Tree->Edges[0].FromSlotIndex, 0);

    FDialogueEdge DuplicateEdge;
    TestFalse(TEXT("Duplicate edge is rejected"), UChronicleDialogueEditorLibrary::AddDialogueEdge(Tree, RootGuid, SpeechGuid, 0, TEXT(""), DuplicateEdge, Error));
    TestTrue(TEXT("Duplicate edge reports an error"), !Error.IsEmpty());

    FDialogueEdge ConditionalEdge;
    TestTrue(TEXT("Conditional edge can share source slot"), UChronicleDialogueEditorLibrary::AddDialogueEdge(Tree, RootGuid, ChoiceGuid, 0, TEXT("flag == true"), ConditionalEdge, Error));
    TestEqual(TEXT("Two edges exist"), Tree->Edges.Num(), 2);
    TestEqual(TEXT("Condition is trimmed and persisted"), ConditionalEdge.ConditionExpression, FString(TEXT("flag == true")));

    int32 RemovedCount = 0;
    TestTrue(TEXT("Edge can be removed"), UChronicleDialogueEditorLibrary::RemoveDialogueEdge(Tree, RootGuid, SpeechGuid, 0, TEXT(""), RemovedCount, Error));
    TestEqual(TEXT("One matching edge removed"), RemovedCount, 1);
    TestEqual(TEXT("One edge remains"), Tree->Edges.Num(), 1);
    TestEqual(TEXT("Conditional edge remains"), Tree->Edges[0].ToNodeGuid, ChoiceGuid);

    return true;
}
