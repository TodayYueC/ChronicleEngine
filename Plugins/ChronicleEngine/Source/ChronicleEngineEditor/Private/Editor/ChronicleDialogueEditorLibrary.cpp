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

