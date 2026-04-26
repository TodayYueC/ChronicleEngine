#include "Misc/AutomationTest.h"

#include "Data/DialogueTree.h"
#include "Data/DialogueDatabase.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Editor/ChronicleDialogueGraph.h"
#include "Editor/ChronicleDialogueGraphNode.h"
#include "Editor/ChronicleDialogueGraphSchema.h"
#include "Editor/ChronicleDialogueNodeDetails.h"
#include "Runtime/DialogueRunner.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorGraphSchemaTest, "Chronicle.Editor.Graph.SchemaSync", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorGraphSchemaTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FString Error;
    FGuid RootGuid;
    FGuid SpeechGuid;
    TestTrue(TEXT("Root node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Root, FVector2D::ZeroVector, RootGuid, Error));
    TestTrue(TEXT("Speech node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Speech, FVector2D(200.0f, 100.0f), SpeechGuid, Error));

    UChronicleDialogueGraph* Graph = NewObject<UChronicleDialogueGraph>();
    Graph->Initialize(Tree);

    TestEqual(TEXT("Graph mirrors dialogue nodes"), Graph->Nodes.Num(), 2);
    UChronicleDialogueGraphNode* RootGraphNode = Graph->FindDialogueGraphNode(RootGuid);
    UChronicleDialogueGraphNode* SpeechGraphNode = Graph->FindDialogueGraphNode(SpeechGuid);
    TestNotNull(TEXT("Root graph node exists"), RootGraphNode);
    TestNotNull(TEXT("Speech graph node exists"), SpeechGraphNode);
    if (!RootGraphNode || !SpeechGraphNode)
    {
        return false;
    }

    UEdGraphPin* RootOutputPin = RootGraphNode->GetOutputPinBySlot(0);
    UEdGraphPin* SpeechInputPin = SpeechGraphNode->GetInputPin();
    TestNotNull(TEXT("Root output pin exists"), RootOutputPin);
    TestNotNull(TEXT("Speech input pin exists"), SpeechInputPin);
    if (!RootOutputPin || !SpeechInputPin)
    {
        return false;
    }

    const UChronicleDialogueGraphSchema* Schema = Cast<UChronicleDialogueGraphSchema>(Graph->GetSchema());
    TestNotNull(TEXT("Chronicle graph schema exists"), Schema);
    if (!Schema)
    {
        return false;
    }

    const FPinConnectionResponse Response = Schema->CanCreateConnection(RootOutputPin, SpeechInputPin);
    TestEqual(TEXT("Schema allows output-to-input dialogue connection"), Response.Response.GetValue(), CONNECT_RESPONSE_MAKE);

    TestTrue(TEXT("Schema creates a dialogue edge"), Schema->TryCreateConnection(RootOutputPin, SpeechInputPin));
    TestEqual(TEXT("Dialogue tree gained one edge"), Tree->Edges.Num(), 1);
    TestEqual(TEXT("Created edge source"), Tree->Edges[0].FromNodeGuid, RootGuid);
    TestEqual(TEXT("Created edge target"), Tree->Edges[0].ToNodeGuid, SpeechGuid);
    TestTrue(TEXT("Graph pins are linked"), RootOutputPin->LinkedTo.Contains(SpeechInputPin));

    SpeechGraphNode->NodePosX = 640;
    SpeechGraphNode->NodePosY = 320;
    Graph->SynchronizeNodePositionsToDialogueTree();
    const FDialogueNode* SpeechNode = Tree->FindNode(SpeechGuid);
    TestNotNull(TEXT("Speech dialogue node exists after position sync"), SpeechNode);
    if (SpeechNode)
    {
        TestEqual(TEXT("Graph node position synced to dialogue tree"), SpeechNode->Position, FVector2D(640.0f, 320.0f));
    }

    Schema->BreakSinglePinLink(RootOutputPin, SpeechInputPin);
    TestEqual(TEXT("Dialogue edge removed when pin link breaks"), Tree->Edges.Num(), 0);
    TestFalse(TEXT("Graph pins are unlinked"), RootOutputPin->LinkedTo.Contains(SpeechInputPin));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorGraphContextActionsTest, "Chronicle.Editor.Graph.ContextActions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorGraphContextActionsTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    UChronicleDialogueGraph* Graph = NewObject<UChronicleDialogueGraph>();
    Graph->Initialize(Tree);

    const UChronicleDialogueGraphSchema* Schema = Cast<UChronicleDialogueGraphSchema>(Graph->GetSchema());
    TestNotNull(TEXT("Chronicle graph schema exists"), Schema);
    if (!Schema)
    {
        return false;
    }

    TArray<EDialogueNodeType> SupportedNodeTypes;
    UChronicleDialogueGraphSchema::GetSupportedContextNodeTypes(SupportedNodeTypes);
    TestTrue(TEXT("Context menu supports M3 node actions"), SupportedNodeTypes.Num() >= 5);
    TestTrue(TEXT("Context menu supports Speech nodes"), SupportedNodeTypes.Contains(EDialogueNodeType::Speech));
    TestTrue(TEXT("Context menu supports Choice nodes"), SupportedNodeTypes.Contains(EDialogueNodeType::Choice));
    TestTrue(TEXT("Context menu supports Condition nodes"), SupportedNodeTypes.Contains(EDialogueNodeType::Condition));

    FString Error;
    UEdGraphNode* CreatedNode = Graph->AddDialogueNodeFromSchemaAction(SupportedNodeTypes[0], FVector2D(88.0f, 144.0f), Error);
    TestNotNull(TEXT("Context action creates a graph node"), CreatedNode);
    TestEqual(TEXT("Context action adds one dialogue node"), Tree->Nodes.Num(), 1);
    TestEqual(TEXT("Created node position comes from context location"), Tree->Nodes[0].Position, FVector2D(88.0f, 144.0f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorNodeDetailsTest, "Chronicle.Editor.TreeEditor.NodeDetails", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorNodeDetailsTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FString Error;
    FGuid SpeechGuid;
    TestTrue(TEXT("Speech node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Speech, FVector2D::ZeroVector, SpeechGuid, Error));

    UChronicleDialogueNodeDetails* Details = NewObject<UChronicleDialogueNodeDetails>();
    Details->LoadFromNode(Tree, SpeechGuid);
    Details->NodeType = EDialogueNodeType::Condition;
    Details->ConditionExpression = TEXT("score >= 10");
    Details->bBreakpointEnabled = true;
    Details->BreakpointNote = TEXT("Inspect high score branch");

    TestTrue(TEXT("Details object applies selected node edits"), Details->ApplyToNode(Error));
    const FDialogueNode* EditedNode = Tree->FindNode(SpeechGuid);
    TestNotNull(TEXT("Edited node still exists"), EditedNode);
    if (EditedNode)
    {
        TestEqual(TEXT("Details changed node type"), EditedNode->NodeType, EDialogueNodeType::Condition);
        TestEqual(TEXT("Details changed condition"), EditedNode->ConditionExpression, FString(TEXT("score >= 10")));
    }

    TestTrue(TEXT("Details applied breakpoint state"), UChronicleDialogueEditorLibrary::IsDialogueNodeBreakpointSet(Tree, SpeechGuid));
    const FDialogueNodeEditorState* EditorState = Tree->FindEditorState(SpeechGuid);
    TestNotNull(TEXT("Editor state exists"), EditorState);
    if (EditorState)
    {
        TestEqual(TEXT("Breakpoint note persisted"), EditorState->BreakpointNote, FString(TEXT("Inspect high score branch")));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChronicleEditorDebuggerAndLockTest, "Chronicle.Editor.DebuggerAndLock", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FChronicleEditorDebuggerAndLockTest::RunTest(const FString& Parameters)
{
    UDialogueTree* Tree = NewObject<UDialogueTree>();
    Tree->TreeGuid = FGuid::NewGuid();

    FString Error;
    FGuid RootGuid;
    FGuid SpeechGuid;
    TestTrue(TEXT("Root node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Root, FVector2D::ZeroVector, RootGuid, Error));
    TestTrue(TEXT("Speech node can be added"), UChronicleDialogueEditorLibrary::AddDialogueNode(Tree, EDialogueNodeType::Speech, FVector2D(200.0f, 0.0f), SpeechGuid, Error));
    FDialogueEdge Edge;
    TestTrue(TEXT("Root can link to speech"), UChronicleDialogueEditorLibrary::AddDialogueEdge(Tree, RootGuid, SpeechGuid, 0, FString(), Edge, Error));
    TestTrue(TEXT("Breakpoint can be enabled"), UChronicleDialogueEditorLibrary::SetDialogueNodeBreakpoint(Tree, SpeechGuid, true, TEXT("Stop here"), Error));

    UDialogueRunner* Runner = NewObject<UDialogueRunner>();
    Runner->Initialize(nullptr);
    Runner->StartDialogue(Tree);

    FChronicleDialogueDebuggerSnapshot Snapshot;
    TestTrue(TEXT("Debugger snapshot can be captured"), UChronicleDialogueEditorLibrary::CaptureDebuggerSnapshot(Runner, Snapshot, Error));
    TestEqual(TEXT("Snapshot points at the speech node"), Snapshot.CurrentNodeGuid, SpeechGuid);
    TestEqual(TEXT("Snapshot records runner state"), Snapshot.RunnerState, EDialogueRunnerState::WaitingForInput);
    TestTrue(TEXT("Snapshot sees node breakpoint"), Snapshot.bNodeHasBreakpoint);

    FChronicleSoftLockMetadata TreeLock;
    TestTrue(TEXT("Tree lock can be acquired"), UChronicleDialogueEditorLibrary::AcquireDialogueTreeLock(Tree, TEXT("Automation"), TreeLock, Error));
    TestTrue(TEXT("Tree lock is marked locked"), Tree->EditorLock.bLocked);
    TestFalse(TEXT("Current user lock is not treated as foreign"), UChronicleDialogueEditorLibrary::IsDialogueTreeLockedByOtherUser(Tree));
    TestTrue(TEXT("Tree lock can be released"), UChronicleDialogueEditorLibrary::ReleaseDialogueTreeLock(Tree, Error));
    TestFalse(TEXT("Tree lock released"), Tree->EditorLock.bLocked);

    UDialogueDatabase* Database = NewObject<UDialogueDatabase>();
    FChronicleSoftLockMetadata DatabaseLock;
    TestTrue(TEXT("Database lock can be acquired"), UChronicleDialogueEditorLibrary::AcquireDialogueDatabaseLock(Database, TEXT("Automation"), DatabaseLock, Error));
    TestTrue(TEXT("Database lock is marked locked"), Database->EditorLock.bLocked);
    TestTrue(TEXT("Database lock can be released"), UChronicleDialogueEditorLibrary::ReleaseDialogueDatabaseLock(Database, Error));
    TestFalse(TEXT("Database lock released"), Database->EditorLock.bLocked);

    return true;
}
