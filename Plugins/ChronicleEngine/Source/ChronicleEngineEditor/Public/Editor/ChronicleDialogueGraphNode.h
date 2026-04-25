#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "EdGraph/EdGraphNode.h"
#include "ChronicleDialogueGraphNode.generated.h"

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueGraphNode : public UEdGraphNode
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FGuid DialogueNodeGuid;

    UPROPERTY()
    EDialogueNodeType DialogueNodeType = EDialogueNodeType::Speech;

    UPROPERTY()
    int32 OutputSlotCount = 1;

    UPROPERTY()
    FText DisplayTitle;

    UPROPERTY()
    FString Summary;

    void InitializeFromDialogueNode(const FDialogueNode& Node, int32 InOutputSlotCount);

    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FText GetTooltipText() const override;

    UEdGraphPin* GetInputPin() const;
    UEdGraphPin* GetOutputPinBySlot(int32 SlotIndex) const;
    int32 GetOutputSlotIndex(const UEdGraphPin* Pin) const;

    static FName GetDialoguePinCategory();
};
