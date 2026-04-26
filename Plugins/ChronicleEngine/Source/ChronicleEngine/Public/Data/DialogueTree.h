#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Engine/DataAsset.h"
#include "DialogueTree.generated.h"

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UDialogueTree : public UDataAsset
{
    GENERATED_BODY()

public:
    UDialogueTree();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree")
    FGuid TreeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree")
    TArray<FDialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree")
    TArray<FDialogueEdge> Edges;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree")
    FGuid RootNodeGuid;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree")
    TArray<FVariableDefinition> Variables;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree|Editor")
    TArray<FDialogueNodeEditorState> EditorStates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Tree|Editor")
    FChronicleSoftLockMetadata EditorLock;

    virtual void PostLoad() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UFUNCTION(BlueprintCallable, Category="Chronicle|Tree")
    bool IsValidTree(FText& OutError) const;

    const FDialogueNode* FindNode(const FGuid& NodeGuid) const;
    FDialogueNode* FindNodeMutable(const FGuid& NodeGuid);
    const FDialogueNodeEditorState* FindEditorState(const FGuid& NodeGuid) const;
    FDialogueNodeEditorState* FindEditorStateMutable(const FGuid& NodeGuid);
    FDialogueNodeEditorState& FindOrAddEditorState(const FGuid& NodeGuid);
    bool HasBreakpoint(const FGuid& NodeGuid) const;
    void GetOutgoingEdges(const FGuid& NodeGuid, TArray<FDialogueEdge>& OutEdges) const;
    bool ResolveEntryNode(FName EntryNode, FGuid& OutNodeGuid) const;

    void EnsureStableGuids();
};
