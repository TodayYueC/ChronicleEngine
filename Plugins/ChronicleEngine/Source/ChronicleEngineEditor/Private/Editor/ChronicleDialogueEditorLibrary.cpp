#include "Editor/ChronicleDialogueEditorLibrary.h"

#include "Data/DialogueDatabase.h"
#include "Data/DialogueTree.h"
#include "HAL/PlatformProcess.h"
#include "Runtime/DialogueRunner.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueEditorLibrary"

namespace
{
void FillCurrentUserLock(FChronicleSoftLockMetadata& Lock, const FString& Note)
{
    Lock.bLocked = true;
    Lock.OwnerUserName = FPlatformProcess::UserName(false);
    Lock.OwnerMachineName = FPlatformProcess::ComputerName();
    Lock.SessionGuid = FGuid::NewGuid();
    Lock.LockedAtUtc = FDateTime::UtcNow();
    Lock.Note = Note;
}

bool IsLockedByOtherUser(const FChronicleSoftLockMetadata& Lock)
{
    if (!Lock.bLocked)
    {
        return false;
    }

    return !Lock.OwnerUserName.Equals(FPlatformProcess::UserName(false), ESearchCase::IgnoreCase)
        || !Lock.OwnerMachineName.Equals(FPlatformProcess::ComputerName(), ESearchCase::IgnoreCase);
}

FText BuildNodeTitle(const FDialogueNode& Node)
{
    if (Node.NodeType == EDialogueNodeType::Speech && Node.Lines.Num() > 0)
    {
        return FText::Format(
            LOCTEXT("DebuggerSpeechTitle", "{0}: {1}"),
            UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(Node.NodeType),
            Node.Lines[0].Text);
    }

    return UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(Node.NodeType);
}
}

bool UChronicleDialogueEditorLibrary::AddDialogueNode(UDialogueTree* Tree, EDialogueNodeType NodeType, FVector2D Position, FGuid& OutNodeGuid, FString& OutError)
{
    OutNodeGuid.Invalidate();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (NodeType == EDialogueNodeType::Root && Tree->RootNodeGuid.IsValid() && Tree->FindNode(Tree->RootNodeGuid))
    {
        OutError = TEXT("Dialogue tree already has a root node.");
        return false;
    }

    Tree->Modify();

    FDialogueNode Node;
    Node.NodeGuid = FGuid::NewGuid();
    Node.NodeType = NodeType;
    Node.Position = Position;

    switch (NodeType)
    {
    case EDialogueNodeType::Root:
        Tree->RootNodeGuid = Node.NodeGuid;
        break;
    case EDialogueNodeType::Speech:
    {
        FDialogueLine Line;
        Line.LineID = FName(*FString::Printf(TEXT("Line_%s_0"), *Node.NodeGuid.ToString(EGuidFormats::Digits)));
        Line.Text = LOCTEXT("NewSpeechLine", "New dialogue line");
        Node.Lines.Add(Line);
        break;
    }
    case EDialogueNodeType::Choice:
    {
        FDialogueChoice Choice;
        Choice.Text = LOCTEXT("NewChoice", "New choice");
        Choice.TargetOutputIndex = 0;
        Node.Choices.Add(Choice);
        break;
    }
    case EDialogueNodeType::Condition:
        Node.ConditionExpression = TEXT("true");
        break;
    default:
        break;
    }

    Tree->Nodes.Add(Node);
    Tree->EnsureStableGuids();
    Tree->MarkPackageDirty();

    OutNodeGuid = Node.NodeGuid;
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::SetDialogueNodePosition(UDialogueTree* Tree, const FGuid& NodeGuid, FVector2D Position, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (!NodeGuid.IsValid())
    {
        OutError = TEXT("Node GUID is invalid.");
        return false;
    }

    FDialogueNode* Node = Tree->FindNodeMutable(NodeGuid);
    if (!Node)
    {
        OutError = TEXT("Node was not found in the dialogue tree.");
        return false;
    }

    Tree->Modify();
    Node->Position = Position;
    Tree->MarkPackageDirty();

    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::RemoveDialogueNodes(UDialogueTree* Tree, const TArray<FGuid>& NodeGuids, int32& OutRemovedNodeCount, int32& OutRemovedEdgeCount, FString& OutError)
{
    OutRemovedNodeCount = 0;
    OutRemovedEdgeCount = 0;
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TSet<FGuid> NodeGuidSet;
    for (const FGuid& NodeGuid : NodeGuids)
    {
        if (NodeGuid.IsValid())
        {
            NodeGuidSet.Add(NodeGuid);
        }
    }

    if (NodeGuidSet.Num() == 0)
    {
        OutError = TEXT("No valid dialogue node GUIDs were supplied.");
        return false;
    }

    Tree->Modify();

    for (int32 Index = Tree->Nodes.Num() - 1; Index >= 0; --Index)
    {
        if (NodeGuidSet.Contains(Tree->Nodes[Index].NodeGuid))
        {
            if (Tree->RootNodeGuid == Tree->Nodes[Index].NodeGuid)
            {
                Tree->RootNodeGuid.Invalidate();
            }

            Tree->Nodes.RemoveAt(Index);
            ++OutRemovedNodeCount;
        }
    }

    for (int32 Index = Tree->Edges.Num() - 1; Index >= 0; --Index)
    {
        if (NodeGuidSet.Contains(Tree->Edges[Index].FromNodeGuid) || NodeGuidSet.Contains(Tree->Edges[Index].ToNodeGuid))
        {
            Tree->Edges.RemoveAt(Index);
            ++OutRemovedEdgeCount;
        }
    }

    Tree->EditorStates.RemoveAll([&NodeGuidSet](const FDialogueNodeEditorState& State)
    {
        return NodeGuidSet.Contains(State.NodeGuid);
    });

    if (OutRemovedNodeCount == 0)
    {
        OutError = TEXT("No matching dialogue nodes were found.");
        return false;
    }

    Tree->EnsureStableGuids();
    Tree->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::DuplicateDialogueNodes(UDialogueTree* Tree, const TArray<FGuid>& NodeGuids, FVector2D PositionOffset, TArray<FGuid>& OutDuplicatedNodeGuids, FString& OutError)
{
    OutDuplicatedNodeGuids.Reset();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    TSet<FGuid> SourceNodeGuidSet;
    for (const FGuid& NodeGuid : NodeGuids)
    {
        if (NodeGuid.IsValid())
        {
            SourceNodeGuidSet.Add(NodeGuid);
        }
    }

    if (SourceNodeGuidSet.Num() == 0)
    {
        OutError = TEXT("No valid dialogue node GUIDs were supplied.");
        return false;
    }

    TArray<FDialogueNode> DuplicatedNodes;
    TMap<FGuid, FGuid> OldToNewNodeGuids;

    for (const FDialogueNode& SourceNode : Tree->Nodes)
    {
        if (!SourceNodeGuidSet.Contains(SourceNode.NodeGuid))
        {
            continue;
        }

        if (SourceNode.NodeType == EDialogueNodeType::Root)
        {
            continue;
        }

        FDialogueNode DuplicatedNode = SourceNode;
        const FGuid SourceNodeGuid = SourceNode.NodeGuid;
        DuplicatedNode.NodeGuid = FGuid::NewGuid();
        DuplicatedNode.Position += PositionOffset;

        for (int32 LineIndex = 0; LineIndex < DuplicatedNode.Lines.Num(); ++LineIndex)
        {
            DuplicatedNode.Lines[LineIndex].LineID = FName(*FString::Printf(TEXT("Line_%s_%d"), *DuplicatedNode.NodeGuid.ToString(EGuidFormats::Digits), LineIndex));
        }

        OldToNewNodeGuids.Add(SourceNodeGuid, DuplicatedNode.NodeGuid);
        OutDuplicatedNodeGuids.Add(DuplicatedNode.NodeGuid);
        DuplicatedNodes.Add(DuplicatedNode);
    }

    if (DuplicatedNodes.Num() == 0)
    {
        OutError = TEXT("No duplicatable dialogue nodes were found. Root nodes cannot be duplicated.");
        return false;
    }

    TArray<FDialogueEdge> DuplicatedEdges;
    for (const FDialogueEdge& SourceEdge : Tree->Edges)
    {
        const FGuid* NewFromGuid = OldToNewNodeGuids.Find(SourceEdge.FromNodeGuid);
        const FGuid* NewToGuid = OldToNewNodeGuids.Find(SourceEdge.ToNodeGuid);
        if (!NewFromGuid || !NewToGuid)
        {
            continue;
        }

        FDialogueEdge DuplicatedEdge = SourceEdge;
        DuplicatedEdge.FromNodeGuid = *NewFromGuid;
        DuplicatedEdge.ToNodeGuid = *NewToGuid;
        DuplicatedEdges.Add(DuplicatedEdge);
    }

    Tree->Modify();
    Tree->Nodes.Append(DuplicatedNodes);
    Tree->Edges.Append(DuplicatedEdges);
    Tree->EnsureStableGuids();
    Tree->MarkPackageDirty();

    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::AddDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, FDialogueEdge& OutEdge, FString& OutError)
{
    OutEdge = FDialogueEdge();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (!FromNodeGuid.IsValid() || !ToNodeGuid.IsValid())
    {
        OutError = TEXT("Both edge endpoints must have valid GUIDs.");
        return false;
    }

    if (FromSlotIndex < 0)
    {
        OutError = TEXT("Edge output slot must be zero or greater.");
        return false;
    }

    if (!Tree->FindNode(FromNodeGuid))
    {
        OutError = TEXT("Edge source node was not found in the dialogue tree.");
        return false;
    }

    if (!Tree->FindNode(ToNodeGuid))
    {
        OutError = TEXT("Edge target node was not found in the dialogue tree.");
        return false;
    }

    const FString TrimmedCondition = ConditionExpression.TrimStartAndEnd();
    const bool bDuplicate = Tree->Edges.ContainsByPredicate([&](const FDialogueEdge& Edge)
    {
        return Edge.FromNodeGuid == FromNodeGuid
            && Edge.ToNodeGuid == ToNodeGuid
            && Edge.FromSlotIndex == FromSlotIndex
            && Edge.ConditionExpression.TrimStartAndEnd().Equals(TrimmedCondition, ESearchCase::CaseSensitive);
    });

    if (bDuplicate)
    {
        OutError = TEXT("An identical dialogue edge already exists.");
        return false;
    }

    Tree->Modify();

    FDialogueEdge NewEdge;
    NewEdge.FromNodeGuid = FromNodeGuid;
    NewEdge.ToNodeGuid = ToNodeGuid;
    NewEdge.FromSlotIndex = FromSlotIndex;
    NewEdge.ConditionExpression = TrimmedCondition;
    Tree->Edges.Add(NewEdge);
    Tree->MarkPackageDirty();

    OutEdge = NewEdge;
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::RemoveDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, int32& OutRemovedCount, FString& OutError)
{
    OutRemovedCount = 0;
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (!FromNodeGuid.IsValid() || !ToNodeGuid.IsValid())
    {
        OutError = TEXT("Both edge endpoints must have valid GUIDs.");
        return false;
    }

    const FString TrimmedCondition = ConditionExpression.TrimStartAndEnd();
    Tree->Modify();

    for (int32 Index = Tree->Edges.Num() - 1; Index >= 0; --Index)
    {
        const FDialogueEdge& Edge = Tree->Edges[Index];
        if (Edge.FromNodeGuid == FromNodeGuid
            && Edge.ToNodeGuid == ToNodeGuid
            && Edge.FromSlotIndex == FromSlotIndex
            && Edge.ConditionExpression.TrimStartAndEnd().Equals(TrimmedCondition, ESearchCase::CaseSensitive))
        {
            Tree->Edges.RemoveAt(Index);
            ++OutRemovedCount;
        }
    }

    if (OutRemovedCount == 0)
    {
        OutError = TEXT("No matching dialogue edge was found.");
        return false;
    }

    Tree->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::SetDialogueNodeBreakpoint(UDialogueTree* Tree, const FGuid& NodeGuid, bool bEnabled, const FString& Note, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (!Tree->FindNode(NodeGuid))
    {
        OutError = TEXT("Node was not found in the dialogue tree.");
        return false;
    }

    Tree->Modify();
    FDialogueNodeEditorState& State = Tree->FindOrAddEditorState(NodeGuid);
    State.bBreakpointEnabled = bEnabled;
    State.BreakpointNote = Note;
    Tree->MarkPackageDirty();

    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::IsDialogueNodeBreakpointSet(UDialogueTree* Tree, const FGuid& NodeGuid)
{
    return Tree && Tree->HasBreakpoint(NodeGuid);
}

int32 UChronicleDialogueEditorLibrary::GetDialogueNodeBreakpoints(UDialogueTree* Tree, TArray<FGuid>& OutNodeGuids)
{
    OutNodeGuids.Reset();
    if (!Tree)
    {
        return 0;
    }

    for (const FDialogueNodeEditorState& State : Tree->EditorStates)
    {
        if (State.bBreakpointEnabled && Tree->FindNode(State.NodeGuid))
        {
            OutNodeGuids.Add(State.NodeGuid);
        }
    }

    return OutNodeGuids.Num();
}

int32 UChronicleDialogueEditorLibrary::SearchDialogueNodes(UDialogueTree* Tree, const FString& Query, TArray<FGuid>& OutNodeGuids)
{
    OutNodeGuids.Reset();
    if (!Tree)
    {
        return 0;
    }

    const FString NormalizedQuery = Query.TrimStartAndEnd();
    if (NormalizedQuery.IsEmpty())
    {
        for (const FDialogueNode& Node : Tree->Nodes)
        {
            OutNodeGuids.Add(Node.NodeGuid);
        }
        return OutNodeGuids.Num();
    }

    for (const FDialogueNode& Node : Tree->Nodes)
    {
        bool bMatches = Node.NodeGuid.ToString(EGuidFormats::DigitsWithHyphens).Contains(NormalizedQuery, ESearchCase::IgnoreCase)
            || GetNodeTypeDisplayName(Node.NodeType).ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase)
            || Node.ConditionExpression.Contains(NormalizedQuery, ESearchCase::IgnoreCase)
            || Node.EventTag.ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase);

        for (const FDialogueLine& Line : Node.Lines)
        {
            bMatches = bMatches
                || Line.LineID.ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase)
                || Line.SpeakerTag.ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase)
                || Line.Text.ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase);
        }

        for (const FDialogueChoice& Choice : Node.Choices)
        {
            bMatches = bMatches
                || Choice.Text.ToString().Contains(NormalizedQuery, ESearchCase::IgnoreCase)
                || Choice.VisibilityCondition.Contains(NormalizedQuery, ESearchCase::IgnoreCase);
        }

        if (bMatches)
        {
            OutNodeGuids.Add(Node.NodeGuid);
        }
    }

    return OutNodeGuids.Num();
}

FText UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(EDialogueNodeType NodeType)
{
    switch (NodeType)
    {
    case EDialogueNodeType::Root:
        return LOCTEXT("RootNode", "Root");
    case EDialogueNodeType::Speech:
        return LOCTEXT("SpeechNode", "Speech");
    case EDialogueNodeType::Choice:
        return LOCTEXT("ChoiceNode", "Choice");
    case EDialogueNodeType::Condition:
        return LOCTEXT("ConditionNode", "Condition");
    case EDialogueNodeType::Event:
        return LOCTEXT("EventNode", "Event");
    case EDialogueNodeType::Wait:
        return LOCTEXT("WaitNode", "Wait");
    case EDialogueNodeType::Random:
        return LOCTEXT("RandomNode", "Random");
    case EDialogueNodeType::Jump:
        return LOCTEXT("JumpNode", "Jump");
    case EDialogueNodeType::Sequence:
        return LOCTEXT("SequenceNode", "Sequence");
    case EDialogueNodeType::SubDialogue:
        return LOCTEXT("SubDialogueNode", "SubDialogue");
    case EDialogueNodeType::Camera:
        return LOCTEXT("CameraNode", "Camera");
    case EDialogueNodeType::Animation:
        return LOCTEXT("AnimationNode", "Animation");
    default:
        return LOCTEXT("UnknownNode", "Unknown");
    }
}

bool UChronicleDialogueEditorLibrary::CaptureDebuggerSnapshot(UDialogueRunner* Runner, FChronicleDialogueDebuggerSnapshot& OutSnapshot, FString& OutError)
{
    OutSnapshot = FChronicleDialogueDebuggerSnapshot();
    if (!Runner)
    {
        OutError = TEXT("No dialogue runner supplied.");
        return false;
    }

    UDialogueTree* CurrentTree = Runner->GetCurrentTree();
    OutSnapshot.CurrentTree = CurrentTree;
    OutSnapshot.CurrentTreeGuid = CurrentTree ? CurrentTree->TreeGuid : FGuid();
    OutSnapshot.CurrentNodeGuid = Runner->GetCurrentNodeGuid();
    OutSnapshot.RunnerState = Runner->GetRunnerState();

    if (CurrentTree)
    {
        OutSnapshot.bNodeHasBreakpoint = CurrentTree->HasBreakpoint(OutSnapshot.CurrentNodeGuid);
        if (const FDialogueNode* Node = CurrentTree->FindNode(OutSnapshot.CurrentNodeGuid))
        {
            OutSnapshot.NodeTitle = BuildNodeTitle(*Node);
        }
    }

    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::AcquireDialogueTreeLock(UDialogueTree* Tree, const FString& Note, FChronicleSoftLockMetadata& OutLock, FString& OutError)
{
    OutLock = FChronicleSoftLockMetadata();
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (IsLockedByOtherUser(Tree->EditorLock))
    {
        OutError = FString::Printf(TEXT("Dialogue tree is locked by %s on %s."), *Tree->EditorLock.OwnerUserName, *Tree->EditorLock.OwnerMachineName);
        return false;
    }

    Tree->Modify();
    FillCurrentUserLock(Tree->EditorLock, Note);
    Tree->MarkPackageDirty();
    OutLock = Tree->EditorLock;
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::ReleaseDialogueTreeLock(UDialogueTree* Tree, FString& OutError)
{
    if (!Tree)
    {
        OutError = TEXT("No dialogue tree supplied.");
        return false;
    }

    if (IsLockedByOtherUser(Tree->EditorLock))
    {
        OutError = FString::Printf(TEXT("Dialogue tree is locked by %s on %s."), *Tree->EditorLock.OwnerUserName, *Tree->EditorLock.OwnerMachineName);
        return false;
    }

    Tree->Modify();
    Tree->EditorLock = FChronicleSoftLockMetadata();
    Tree->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::IsDialogueTreeLockedByOtherUser(UDialogueTree* Tree)
{
    return Tree && IsLockedByOtherUser(Tree->EditorLock);
}

bool UChronicleDialogueEditorLibrary::AcquireDialogueDatabaseLock(UDialogueDatabase* Database, const FString& Note, FChronicleSoftLockMetadata& OutLock, FString& OutError)
{
    OutLock = FChronicleSoftLockMetadata();
    if (!Database)
    {
        OutError = TEXT("No dialogue database supplied.");
        return false;
    }

    if (IsLockedByOtherUser(Database->EditorLock))
    {
        OutError = FString::Printf(TEXT("Dialogue database is locked by %s on %s."), *Database->EditorLock.OwnerUserName, *Database->EditorLock.OwnerMachineName);
        return false;
    }

    Database->Modify();
    FillCurrentUserLock(Database->EditorLock, Note);
    Database->MarkPackageDirty();
    OutLock = Database->EditorLock;
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::ReleaseDialogueDatabaseLock(UDialogueDatabase* Database, FString& OutError)
{
    if (!Database)
    {
        OutError = TEXT("No dialogue database supplied.");
        return false;
    }

    if (IsLockedByOtherUser(Database->EditorLock))
    {
        OutError = FString::Printf(TEXT("Dialogue database is locked by %s on %s."), *Database->EditorLock.OwnerUserName, *Database->EditorLock.OwnerMachineName);
        return false;
    }

    Database->Modify();
    Database->EditorLock = FChronicleSoftLockMetadata();
    Database->MarkPackageDirty();
    OutError.Reset();
    return true;
}

bool UChronicleDialogueEditorLibrary::IsDialogueDatabaseLockedByOtherUser(UDialogueDatabase* Database)
{
    return Database && IsLockedByOtherUser(Database->EditorLock);
}

#undef LOCTEXT_NAMESPACE
