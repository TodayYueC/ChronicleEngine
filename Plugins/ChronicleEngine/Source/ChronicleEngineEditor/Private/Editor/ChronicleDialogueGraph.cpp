#include "Editor/ChronicleDialogueGraph.h"

#include "Data/DialogueTree.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Editor/ChronicleDialogueGraphNode.h"
#include "Editor/ChronicleDialogueGraphSchema.h"

void UChronicleDialogueGraph::Initialize(UDialogueTree* InDialogueTree)
{
    DialogueTree = InDialogueTree;
    Schema = UChronicleDialogueGraphSchema::StaticClass();
    bEditable = true;
    bAllowDeletion = false;
    bAllowRenaming = false;
    RebuildFromDialogueTree();
}

void UChronicleDialogueGraph::RebuildFromDialogueTree()
{
    Modify();
    Nodes.Reset();

    if (!DialogueTree)
    {
        NotifyGraphChanged();
        return;
    }

    for (const FDialogueNode& DialogueNode : DialogueTree->Nodes)
    {
        UChronicleDialogueGraphNode* GraphNode = NewObject<UChronicleDialogueGraphNode>(this);
        GraphNode->SetFlags(RF_Transactional);
        GraphNode->CreateNewGuid();
        GraphNode->InitializeFromDialogueNode(DialogueNode, GetOutputSlotCount(DialogueNode.NodeGuid), DialogueTree->HasBreakpoint(DialogueNode.NodeGuid));
        GraphNode->NodePosX = FMath::RoundToInt(DialogueNode.Position.X);
        GraphNode->NodePosY = FMath::RoundToInt(DialogueNode.Position.Y);
        GraphNode->AllocateDefaultPins();
        Nodes.Add(GraphNode);
    }

    LinkExistingEdges();
    NotifyGraphChanged();
}

void UChronicleDialogueGraph::SynchronizeNodePositionsToDialogueTree() const
{
    if (!DialogueTree)
    {
        return;
    }

    for (UEdGraphNode* Node : Nodes)
    {
        const UChronicleDialogueGraphNode* DialogueNode = Cast<UChronicleDialogueGraphNode>(Node);
        if (!DialogueNode)
        {
            continue;
        }

        FString Error;
        UChronicleDialogueEditorLibrary::SetDialogueNodePosition(
            DialogueTree,
            DialogueNode->DialogueNodeGuid,
            FVector2D(static_cast<float>(DialogueNode->NodePosX), static_cast<float>(DialogueNode->NodePosY)),
            Error);
    }
}

UChronicleDialogueGraphNode* UChronicleDialogueGraph::FindDialogueGraphNode(const FGuid& NodeGuid) const
{
    for (UEdGraphNode* Node : Nodes)
    {
        UChronicleDialogueGraphNode* DialogueNode = Cast<UChronicleDialogueGraphNode>(Node);
        if (DialogueNode && DialogueNode->DialogueNodeGuid == NodeGuid)
        {
            return DialogueNode;
        }
    }

    return nullptr;
}

UChronicleDialogueGraphNode* UChronicleDialogueGraph::AddDialogueNodeFromSchemaAction(EDialogueNodeType NodeType, FVector2D Position, FString& OutError)
{
    if (!DialogueTree)
    {
        OutError = TEXT("No dialogue tree is attached to the graph.");
        return nullptr;
    }

    FGuid NewNodeGuid;
    if (!UChronicleDialogueEditorLibrary::AddDialogueNode(DialogueTree, NodeType, Position, NewNodeGuid, OutError))
    {
        return nullptr;
    }

    RebuildFromDialogueTree();
    return FindDialogueGraphNode(NewNodeGuid);
}

bool UChronicleDialogueGraph::AddDialogueEdgeFromPins(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin, FString& OutError)
{
    if (!DialogueTree || !SourcePin || !TargetPin)
    {
        OutError = TEXT("Graph, source pin, or target pin is invalid.");
        return false;
    }

    UChronicleDialogueGraphNode* SourceNode = Cast<UChronicleDialogueGraphNode>(SourcePin->GetOwningNode());
    UChronicleDialogueGraphNode* TargetNode = Cast<UChronicleDialogueGraphNode>(TargetPin->GetOwningNode());
    if (!SourceNode || !TargetNode)
    {
        OutError = TEXT("Pins must belong to Chronicle dialogue graph nodes.");
        return false;
    }

    const int32 SlotIndex = SourceNode->GetOutputSlotIndex(SourcePin);
    if (SlotIndex == INDEX_NONE)
    {
        OutError = TEXT("Source pin does not map to a dialogue output slot.");
        return false;
    }

    FDialogueEdge NewEdge;
    return UChronicleDialogueEditorLibrary::AddDialogueEdge(
        DialogueTree,
        SourceNode->DialogueNodeGuid,
        TargetNode->DialogueNodeGuid,
        SlotIndex,
        FString(),
        NewEdge,
        OutError);
}

bool UChronicleDialogueGraph::RemoveDialogueEdgeFromPins(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin)
{
    if (!DialogueTree || !SourcePin || !TargetPin)
    {
        return false;
    }

    UChronicleDialogueGraphNode* SourceNode = Cast<UChronicleDialogueGraphNode>(SourcePin->GetOwningNode());
    UChronicleDialogueGraphNode* TargetNode = Cast<UChronicleDialogueGraphNode>(TargetPin->GetOwningNode());
    if (!SourceNode || !TargetNode)
    {
        return false;
    }

    const int32 SlotIndex = SourceNode->GetOutputSlotIndex(SourcePin);
    if (SlotIndex == INDEX_NONE)
    {
        return false;
    }

    int32 RemovedCount = 0;
    FString Error;
    return UChronicleDialogueEditorLibrary::RemoveDialogueEdge(
        DialogueTree,
        SourceNode->DialogueNodeGuid,
        TargetNode->DialogueNodeGuid,
        SlotIndex,
        FString(),
        RemovedCount,
        Error);
}

int32 UChronicleDialogueGraph::GetOutputSlotCount(const FGuid& NodeGuid) const
{
    int32 OutputSlotCount = 1;

    if (const FDialogueNode* Node = DialogueTree ? DialogueTree->FindNode(NodeGuid) : nullptr)
    {
        OutputSlotCount = FMath::Max(OutputSlotCount, Node->Choices.Num());
    }

    if (DialogueTree)
    {
        for (const FDialogueEdge& Edge : DialogueTree->Edges)
        {
            if (Edge.FromNodeGuid == NodeGuid)
            {
                OutputSlotCount = FMath::Max(OutputSlotCount, Edge.FromSlotIndex + 1);
            }
        }
    }

    return OutputSlotCount;
}

void UChronicleDialogueGraph::LinkExistingEdges()
{
    if (!DialogueTree)
    {
        return;
    }

    for (const FDialogueEdge& Edge : DialogueTree->Edges)
    {
        UChronicleDialogueGraphNode* SourceNode = FindDialogueGraphNode(Edge.FromNodeGuid);
        UChronicleDialogueGraphNode* TargetNode = FindDialogueGraphNode(Edge.ToNodeGuid);
        if (!SourceNode || !TargetNode)
        {
            continue;
        }

        UEdGraphPin* SourcePin = SourceNode->GetOutputPinBySlot(Edge.FromSlotIndex);
        UEdGraphPin* TargetPin = TargetNode->GetInputPin();
        if (!SourcePin || !TargetPin || SourcePin->LinkedTo.Contains(TargetPin))
        {
            continue;
        }

        SourcePin->MakeLinkTo(TargetPin);
    }
}
