#include "Editor/ChronicleDialogueEditorLibrary.h"

#include "Data/DialogueTree.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueEditorLibrary"

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

#undef LOCTEXT_NAMESPACE
