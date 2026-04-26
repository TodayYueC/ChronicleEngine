#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "EdGraph/EdGraph.h"
#include "ChronicleDialogueGraph.generated.h"

class UChronicleDialogueGraphNode;
class UDialogueTree;

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueGraph : public UEdGraph
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TObjectPtr<UDialogueTree> DialogueTree;

    void Initialize(UDialogueTree* InDialogueTree);
    void RebuildFromDialogueTree();
    void SynchronizeNodePositionsToDialogueTree() const;

    UChronicleDialogueGraphNode* FindDialogueGraphNode(const FGuid& NodeGuid) const;
    UChronicleDialogueGraphNode* AddDialogueNodeFromSchemaAction(EDialogueNodeType NodeType, FVector2D Position, FString& OutError);
    bool AddDialogueEdgeFromPins(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin, FString& OutError);
    bool RemoveDialogueEdgeFromPins(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin);

private:
    int32 GetOutputSlotCount(const FGuid& NodeGuid) const;
    void LinkExistingEdges();
};
