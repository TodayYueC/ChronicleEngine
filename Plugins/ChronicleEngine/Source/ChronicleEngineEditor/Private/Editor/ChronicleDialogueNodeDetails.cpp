#include "Editor/ChronicleDialogueNodeDetails.h"

#include "Data/DialogueTree.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"

void UChronicleDialogueNodeDetails::LoadFromNode(UDialogueTree* InTree, const FGuid& InNodeGuid)
{
    SourceTree = InTree;
    TargetNodeGuid = InNodeGuid;

    const FDialogueNode* Node = SourceTree ? SourceTree->FindNode(TargetNodeGuid) : nullptr;
    if (!Node)
    {
        TargetNodeGuid.Invalidate();
        NodeType = EDialogueNodeType::Speech;
        Lines.Reset();
        Choices.Reset();
        ConditionExpression.Reset();
        EventTag = FGameplayTag();
        EventPayload.Reset();
        bEventIsAsync = false;
        WaitTime = -1.0f;
        DefaultOutputIndex = INDEX_NONE;
        bAutoSelectIfSingle = false;
        bBreakpointEnabled = false;
        BreakpointNote.Reset();
        return;
    }

    NodeType = Node->NodeType;
    Lines = Node->Lines;
    Choices = Node->Choices;
    ConditionExpression = Node->ConditionExpression;
    EventTag = Node->EventTag;
    EventPayload = Node->EventPayload;
    bEventIsAsync = Node->bEventIsAsync;
    WaitTime = Node->WaitTime;
    DefaultOutputIndex = Node->DefaultOutputIndex;
    bAutoSelectIfSingle = Node->bAutoSelectIfSingle;

    const FDialogueNodeEditorState* EditorState = SourceTree->FindEditorState(TargetNodeGuid);
    bBreakpointEnabled = EditorState && EditorState->bBreakpointEnabled;
    BreakpointNote = EditorState ? EditorState->BreakpointNote : FString();
}

bool UChronicleDialogueNodeDetails::ApplyToNode(FString& OutError)
{
    if (!SourceTree)
    {
        OutError = TEXT("No dialogue tree is attached to the details object.");
        return false;
    }

    FDialogueNode* Node = SourceTree->FindNodeMutable(TargetNodeGuid);
    if (!Node)
    {
        OutError = TEXT("Selected node no longer exists in the dialogue tree.");
        return false;
    }

    if (NodeType == EDialogueNodeType::Root)
    {
        for (const FDialogueNode& ExistingNode : SourceTree->Nodes)
        {
            if (ExistingNode.NodeGuid != TargetNodeGuid && ExistingNode.NodeType == EDialogueNodeType::Root)
            {
                OutError = TEXT("Only one root node is allowed.");
                return false;
            }
        }
    }

    SourceTree->Modify();

    Node->NodeType = NodeType;
    Node->Lines = Lines;
    Node->Choices = Choices;
    Node->ConditionExpression = ConditionExpression;
    Node->EventTag = EventTag;
    Node->EventPayload = EventPayload;
    Node->bEventIsAsync = bEventIsAsync;
    Node->WaitTime = WaitTime;
    Node->DefaultOutputIndex = DefaultOutputIndex;
    Node->bAutoSelectIfSingle = bAutoSelectIfSingle;

    if (NodeType == EDialogueNodeType::Root)
    {
        SourceTree->RootNodeGuid = TargetNodeGuid;
    }

    SourceTree->EnsureStableGuids();
    SourceTree->MarkPackageDirty();

    return UChronicleDialogueEditorLibrary::SetDialogueNodeBreakpoint(SourceTree, TargetNodeGuid, bBreakpointEnabled, BreakpointNote, OutError);
}
