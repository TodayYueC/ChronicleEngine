#include "Data/DialogueTree.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueTree"

UDialogueTree::UDialogueTree()
{
    EnsureStableGuids();
}

void UDialogueTree::PostLoad()
{
    Super::PostLoad();
    EnsureStableGuids();
}

#if WITH_EDITOR
void UDialogueTree::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    EnsureStableGuids();
}
#endif

bool UDialogueTree::IsValidTree(FText& OutError) const
{
    if (!TreeGuid.IsValid())
    {
        OutError = LOCTEXT("MissingTreeGuid", "Dialogue tree has no valid TreeGuid.");
        return false;
    }

    if (!RootNodeGuid.IsValid())
    {
        OutError = LOCTEXT("MissingRoot", "Dialogue tree has no root node.");
        return false;
    }

    if (!FindNode(RootNodeGuid))
    {
        OutError = LOCTEXT("RootNotFound", "RootNodeGuid does not point to an existing node.");
        return false;
    }

    for (const FDialogueEdge& Edge : Edges)
    {
        if (!FindNode(Edge.FromNodeGuid) || !FindNode(Edge.ToNodeGuid))
        {
            OutError = LOCTEXT("BrokenEdge", "Dialogue tree contains an edge with a missing endpoint.");
            return false;
        }
    }

    OutError = FText::GetEmpty();
    return true;
}

const FDialogueNode* UDialogueTree::FindNode(const FGuid& NodeGuid) const
{
    return Nodes.FindByPredicate([&NodeGuid](const FDialogueNode& Node)
    {
        return Node.NodeGuid == NodeGuid;
    });
}

FDialogueNode* UDialogueTree::FindNodeMutable(const FGuid& NodeGuid)
{
    return Nodes.FindByPredicate([&NodeGuid](const FDialogueNode& Node)
    {
        return Node.NodeGuid == NodeGuid;
    });
}

const FDialogueNodeEditorState* UDialogueTree::FindEditorState(const FGuid& NodeGuid) const
{
    return EditorStates.FindByPredicate([&NodeGuid](const FDialogueNodeEditorState& State)
    {
        return State.NodeGuid == NodeGuid;
    });
}

FDialogueNodeEditorState* UDialogueTree::FindEditorStateMutable(const FGuid& NodeGuid)
{
    return EditorStates.FindByPredicate([&NodeGuid](const FDialogueNodeEditorState& State)
    {
        return State.NodeGuid == NodeGuid;
    });
}

FDialogueNodeEditorState& UDialogueTree::FindOrAddEditorState(const FGuid& NodeGuid)
{
    if (FDialogueNodeEditorState* ExistingState = FindEditorStateMutable(NodeGuid))
    {
        return *ExistingState;
    }

    FDialogueNodeEditorState& NewState = EditorStates.AddDefaulted_GetRef();
    NewState.NodeGuid = NodeGuid;
    return NewState;
}

bool UDialogueTree::HasBreakpoint(const FGuid& NodeGuid) const
{
    const FDialogueNodeEditorState* State = FindEditorState(NodeGuid);
    return State && State->bBreakpointEnabled;
}

void UDialogueTree::GetOutgoingEdges(const FGuid& NodeGuid, TArray<FDialogueEdge>& OutEdges) const
{
    OutEdges.Reset();
    for (const FDialogueEdge& Edge : Edges)
    {
        if (Edge.FromNodeGuid == NodeGuid)
        {
            OutEdges.Add(Edge);
        }
    }

    OutEdges.Sort([](const FDialogueEdge& Left, const FDialogueEdge& Right)
    {
        return Left.FromSlotIndex < Right.FromSlotIndex;
    });
}

bool UDialogueTree::ResolveEntryNode(FName EntryNode, FGuid& OutNodeGuid) const
{
    if (EntryNode.IsNone())
    {
        OutNodeGuid = RootNodeGuid;
        return RootNodeGuid.IsValid();
    }

    const FString EntryText = EntryNode.ToString();
    FGuid ParsedGuid;
    if (FGuid::Parse(EntryText, ParsedGuid) && FindNode(ParsedGuid))
    {
        OutNodeGuid = ParsedGuid;
        return true;
    }

    for (const FDialogueNode& Node : Nodes)
    {
        for (const FDialogueLine& Line : Node.Lines)
        {
            if (Line.LineID == EntryNode)
            {
                OutNodeGuid = Node.NodeGuid;
                return true;
            }
        }
    }

    return false;
}

void UDialogueTree::EnsureStableGuids()
{
    if (!TreeGuid.IsValid())
    {
        TreeGuid = FGuid::NewGuid();
    }

    for (FDialogueNode& Node : Nodes)
    {
        if (!Node.NodeGuid.IsValid())
        {
            Node.NodeGuid = FGuid::NewGuid();
        }
    }

    if (!RootNodeGuid.IsValid())
    {
        if (FDialogueNode* RootNode = Nodes.FindByPredicate([](const FDialogueNode& Node)
            {
                return Node.NodeType == EDialogueNodeType::Root;
            }))
        {
            RootNodeGuid = RootNode->NodeGuid;
        }
    }

    TSet<FGuid> ValidNodeGuids;
    for (const FDialogueNode& Node : Nodes)
    {
        ValidNodeGuids.Add(Node.NodeGuid);
    }

    TSet<FGuid> SeenEditorStateGuids;
    EditorStates.RemoveAll([&ValidNodeGuids, &SeenEditorStateGuids](const FDialogueNodeEditorState& State)
    {
        if (!State.NodeGuid.IsValid() || !ValidNodeGuids.Contains(State.NodeGuid) || SeenEditorStateGuids.Contains(State.NodeGuid))
        {
            return true;
        }

        SeenEditorStateGuids.Add(State.NodeGuid);
        return false;
    });
}

#undef LOCTEXT_NAMESPACE
