#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueEditorLibrary.generated.h"

class UDialogueTree;

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool AddDialogueNode(UDialogueTree* Tree, EDialogueNodeType NodeType, FVector2D Position, FGuid& OutNodeGuid, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool SetDialogueNodePosition(UDialogueTree* Tree, const FGuid& NodeGuid, FVector2D Position, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool AddDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, FDialogueEdge& OutEdge, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool RemoveDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, int32& OutRemovedCount, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static int32 SearchDialogueNodes(UDialogueTree* Tree, const FString& Query, TArray<FGuid>& OutNodeGuids);

    UFUNCTION(BlueprintPure, Category="Chronicle|Editor|Dialogue Tree")
    static FText GetNodeTypeDisplayName(EDialogueNodeType NodeType);
};
