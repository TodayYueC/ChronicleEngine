#include "Editor/ChronicleDialogueGraphNode.h"

#include "Editor/ChronicleDialogueEditorLibrary.h"

#define LOCTEXT_NAMESPACE "ChronicleDialogueGraphNode"

namespace
{
FName MakeOutputPinName(int32 SlotIndex)
{
    return FName(*FString::Printf(TEXT("Out_%d"), SlotIndex));
}
}

void UChronicleDialogueGraphNode::InitializeFromDialogueNode(const FDialogueNode& Node, int32 InOutputSlotCount, bool bInBreakpointEnabled)
{
    DialogueNodeGuid = Node.NodeGuid;
    DialogueNodeType = Node.NodeType;
    OutputSlotCount = FMath::Max(1, InOutputSlotCount);
    bBreakpointEnabled = bInBreakpointEnabled;
    DisplayTitle = UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(Node.NodeType);

    switch (Node.NodeType)
    {
    case EDialogueNodeType::Speech:
        Summary = Node.Lines.Num() > 0 ? Node.Lines[0].Text.ToString().Left(120) : TEXT("No lines");
        break;
    case EDialogueNodeType::Choice:
        Summary = FString::Printf(TEXT("%d choice(s)"), Node.Choices.Num());
        break;
    case EDialogueNodeType::Condition:
        Summary = Node.ConditionExpression.IsEmpty() ? TEXT("No expression") : Node.ConditionExpression.Left(120);
        break;
    case EDialogueNodeType::Event:
        Summary = Node.EventTag.IsValid() ? Node.EventTag.ToString() : TEXT("No event tag");
        break;
    case EDialogueNodeType::Wait:
        Summary = Node.WaitTime >= 0.0f ? FString::Printf(TEXT("Wait %.2fs"), Node.WaitTime) : TEXT("Wait for input");
        break;
    case EDialogueNodeType::Random:
        Summary = FString::Printf(TEXT("Random branch\nDefault: %d"), Node.DefaultOutputIndex);
        break;
    case EDialogueNodeType::Jump:
        Summary = Node.TargetEntryNode.IsNone() ? TEXT("Jump to root") : FString::Printf(TEXT("Jump to %s"), *Node.TargetEntryNode.ToString());
        break;
    case EDialogueNodeType::SubDialogue:
        Summary = Node.TargetTree.IsNull() ? TEXT("No target tree") : Node.TargetTree.ToSoftObjectPath().ToString();
        break;
    case EDialogueNodeType::Camera:
        Summary = Node.EventTag.IsValid() ? Node.EventTag.ToString() : TEXT("Camera cue");
        break;
    case EDialogueNodeType::Animation:
        Summary = Node.EventTag.IsValid() ? Node.EventTag.ToString() : TEXT("Animation cue");
        break;
    default:
        Summary = DialogueNodeGuid.ToString(EGuidFormats::Digits).Left(8);
        break;
    }
}

void UChronicleDialogueGraphNode::AllocateDefaultPins()
{
    Pins.Reset();

    UEdGraphPin* InputPin = CreatePin(EGPD_Input, GetDialoguePinCategory(), FName(TEXT("In")));
    InputPin->PinFriendlyName = LOCTEXT("InputPin", "In");

    for (int32 SlotIndex = 0; SlotIndex < OutputSlotCount; ++SlotIndex)
    {
        UEdGraphPin* OutputPin = CreatePin(EGPD_Output, GetDialoguePinCategory(), MakeOutputPinName(SlotIndex));
        OutputPin->PinFriendlyName = FText::Format(LOCTEXT("OutputPin", "Out {0}"), FText::AsNumber(SlotIndex));
    }
}

FText UChronicleDialogueGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::Format(
        bBreakpointEnabled ? LOCTEXT("BreakpointNodeTitle", "[B] {0}\n{1}") : LOCTEXT("NodeTitle", "{0}\n{1}"),
        DisplayTitle.IsEmpty() ? LOCTEXT("DialogueNode", "Dialogue Node") : DisplayTitle,
        FText::FromString(Summary));
}

FLinearColor UChronicleDialogueGraphNode::GetNodeTitleColor() const
{
    switch (DialogueNodeType)
    {
    case EDialogueNodeType::Root:
        return FLinearColor(0.10f, 0.45f, 0.20f, 1.0f);
    case EDialogueNodeType::Speech:
        return FLinearColor(0.12f, 0.28f, 0.70f, 1.0f);
    case EDialogueNodeType::Choice:
        return FLinearColor(0.82f, 0.38f, 0.08f, 1.0f);
    case EDialogueNodeType::Condition:
        return FLinearColor(0.45f, 0.18f, 0.62f, 1.0f);
    case EDialogueNodeType::Event:
        return FLinearColor(0.66f, 0.12f, 0.12f, 1.0f);
    case EDialogueNodeType::Wait:
        return FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);
    case EDialogueNodeType::Random:
        return FLinearColor(0.05f, 0.52f, 0.55f, 1.0f);
    case EDialogueNodeType::Jump:
        return FLinearColor(0.80f, 0.64f, 0.10f, 1.0f);
    case EDialogueNodeType::Sequence:
        return FLinearColor(0.82f, 0.82f, 0.82f, 1.0f);
    case EDialogueNodeType::SubDialogue:
        return FLinearColor(0.07f, 0.31f, 0.48f, 1.0f);
    case EDialogueNodeType::Camera:
        return FLinearColor(0.78f, 0.24f, 0.54f, 1.0f);
    case EDialogueNodeType::Animation:
        return FLinearColor(0.45f, 0.29f, 0.15f, 1.0f);
    default:
        return FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);
    }
}

FText UChronicleDialogueGraphNode::GetTooltipText() const
{
    return FText::Format(
        bBreakpointEnabled ? LOCTEXT("BreakpointNodeTooltip", "Chronicle Dialogue Node\nGUID: {0}\nBreakpoint enabled") : LOCTEXT("NodeTooltip", "Chronicle Dialogue Node\nGUID: {0}"),
        FText::FromString(DialogueNodeGuid.ToString(EGuidFormats::DigitsWithHyphens)));
}

UEdGraphPin* UChronicleDialogueGraphNode::GetInputPin() const
{
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin && Pin->Direction == EGPD_Input)
        {
            return Pin;
        }
    }
    return nullptr;
}

UEdGraphPin* UChronicleDialogueGraphNode::GetOutputPinBySlot(int32 SlotIndex) const
{
    const FName PinName = MakeOutputPinName(SlotIndex);
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin && Pin->Direction == EGPD_Output && Pin->PinName == PinName)
        {
            return Pin;
        }
    }
    return nullptr;
}

int32 UChronicleDialogueGraphNode::GetOutputSlotIndex(const UEdGraphPin* Pin) const
{
    if (!Pin || Pin->Direction != EGPD_Output)
    {
        return INDEX_NONE;
    }

    FString PinName = Pin->PinName.ToString();
    PinName.RemoveFromStart(TEXT("Out_"));
    return FCString::Atoi(*PinName);
}

FName UChronicleDialogueGraphNode::GetDialoguePinCategory()
{
    static const FName PinCategory(TEXT("ChronicleDialogue"));
    return PinCategory;
}

#undef LOCTEXT_NAMESPACE
