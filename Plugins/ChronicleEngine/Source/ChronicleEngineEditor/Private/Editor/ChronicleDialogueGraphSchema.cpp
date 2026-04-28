#include "Editor/ChronicleDialogueGraphSchema.h"

#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Editor/ChronicleDialogueGraph.h"
#include "Editor/ChronicleDialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueGraphSchema"

namespace
{
bool NormalizePins(UEdGraphPin*& SourcePin, UEdGraphPin*& TargetPin)
{
    if (!SourcePin || !TargetPin)
    {
        return false;
    }

    if (SourcePin->Direction == EGPD_Input && TargetPin->Direction == EGPD_Output)
    {
        Swap(SourcePin, TargetPin);
    }

    return SourcePin->Direction == EGPD_Output && TargetPin->Direction == EGPD_Input;
}

struct FChronicleDialogueGraphSchemaAction_NewNode final : public FEdGraphSchemaAction
{
    EDialogueNodeType NodeType = EDialogueNodeType::Speech;

    FChronicleDialogueGraphSchemaAction_NewNode()
        : FEdGraphSchemaAction()
    {
    }

    explicit FChronicleDialogueGraphSchemaAction_NewNode(EDialogueNodeType InNodeType)
        : FEdGraphSchemaAction(
            LOCTEXT("DialogueNodeCategory", "Dialogue Node"),
            UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(InNodeType),
            LOCTEXT("DialogueNodeTooltip", "Create a Chronicle dialogue node."),
            0)
        , NodeType(InNodeType)
    {
    }

    virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override
    {
        UChronicleDialogueGraph* DialogueGraph = Cast<UChronicleDialogueGraph>(ParentGraph);
        if (!DialogueGraph)
        {
            return nullptr;
        }

        FString Error;
        UChronicleDialogueGraphNode* NewNode = DialogueGraph->AddDialogueNodeFromSchemaAction(NodeType, Location, Error);
        if (FromPin && NewNode)
        {
            if (UEdGraphPin* InputPin = NewNode->GetInputPin())
            {
                const UEdGraphSchema* Schema = DialogueGraph->GetSchema();
                if (Schema && FromPin->Direction == EGPD_Output)
                {
                    Schema->TryCreateConnection(FromPin, InputPin);
                }
            }
        }

        return NewNode;
    }
};
}

void UChronicleDialogueGraphSchema::GetSupportedContextNodeTypes(TArray<EDialogueNodeType>& OutNodeTypes)
{
    OutNodeTypes = {
        EDialogueNodeType::Root,
        EDialogueNodeType::Speech,
        EDialogueNodeType::Choice,
        EDialogueNodeType::Condition,
        EDialogueNodeType::Event,
        EDialogueNodeType::Wait,
        EDialogueNodeType::Random,
        EDialogueNodeType::Jump,
        EDialogueNodeType::Sequence,
        EDialogueNodeType::SubDialogue,
        EDialogueNodeType::Camera,
        EDialogueNodeType::Animation
    };
}

void UChronicleDialogueGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
    TArray<EDialogueNodeType> NodeTypes;
    GetSupportedContextNodeTypes(NodeTypes);
    for (EDialogueNodeType NodeType : NodeTypes)
    {
        ContextMenuBuilder.AddAction(MakeShared<FChronicleDialogueGraphSchemaAction_NewNode>(NodeType));
    }
}

const FPinConnectionResponse UChronicleDialogueGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
    if (!A || !B)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins are required."));
    }

    if (A == B)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("A pin cannot connect to itself."));
    }

    if (A->GetOwningNode() == B->GetOwningNode())
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("A dialogue node cannot connect to itself."));
    }

    if (A->Direction == B->Direction)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connect an output pin to an input pin."));
    }

    const UEdGraphPin* SourcePin = A->Direction == EGPD_Output ? A : B;
    const UEdGraphPin* TargetPin = A->Direction == EGPD_Input ? A : B;
    const UChronicleDialogueGraphNode* SourceNode = Cast<UChronicleDialogueGraphNode>(SourcePin->GetOwningNode());
    const UChronicleDialogueGraphNode* TargetNode = Cast<UChronicleDialogueGraphNode>(TargetPin->GetOwningNode());
    if (!SourceNode || !TargetNode)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Only Chronicle dialogue nodes can be connected."));
    }

    if (SourcePin->PinType.PinCategory != UChronicleDialogueGraphNode::GetDialoguePinCategory()
        || TargetPin->PinType.PinCategory != UChronicleDialogueGraphNode::GetDialoguePinCategory())
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Pins must be Chronicle dialogue pins."));
    }

    if (SourcePin->LinkedTo.Contains(TargetPin))
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("These nodes are already connected."));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Create dialogue edge."));
}

bool UChronicleDialogueGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
    UEdGraphPin* SourcePin = A;
    UEdGraphPin* TargetPin = B;
    if (!NormalizePins(SourcePin, TargetPin))
    {
        return false;
    }

    UChronicleDialogueGraph* DialogueGraph = Cast<UChronicleDialogueGraph>(SourcePin->GetOwningNode()->GetGraph());
    if (!DialogueGraph)
    {
        return false;
    }

    FString Error;
    if (!DialogueGraph->AddDialogueEdgeFromPins(SourcePin, TargetPin, Error))
    {
        return false;
    }

    SourcePin->MakeLinkTo(TargetPin);
    DialogueGraph->NotifyGraphChanged();
    return true;
}

void UChronicleDialogueGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
    UEdGraphPin* NormalizedSourcePin = SourcePin;
    UEdGraphPin* NormalizedTargetPin = TargetPin;
    if (NormalizePins(NormalizedSourcePin, NormalizedTargetPin))
    {
        if (UChronicleDialogueGraph* DialogueGraph = Cast<UChronicleDialogueGraph>(NormalizedSourcePin->GetOwningNode()->GetGraph()))
        {
            DialogueGraph->RemoveDialogueEdgeFromPins(NormalizedSourcePin, NormalizedTargetPin);
        }
    }

    Super::BreakSinglePinLink(SourcePin, TargetPin);
}

#undef LOCTEXT_NAMESPACE
