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

void UChronicleDialogueGraphNode::InitializeFromDialogueNode(const FDialogueNode& Node, int32 InOutputSlotCount)
{
    DialogueNodeGuid = Node.NodeGuid;
    DialogueNodeType = Node.NodeType;
    OutputSlotCount = FMath::Max(1, InOutputSlotCount);
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
        LOCTEXT("NodeTitle", "{0}\n{1}"),
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
    default:
        return FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);
    }
}

FText UChronicleDialogueGraphNode::GetTooltipText() const
{
    return FText::Format(
        LOCTEXT("NodeTooltip", "Chronicle Dialogue Node\nGUID: {0}"),
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
