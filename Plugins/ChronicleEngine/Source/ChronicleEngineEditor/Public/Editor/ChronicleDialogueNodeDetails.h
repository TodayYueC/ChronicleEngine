#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "UObject/Object.h"
#include "ChronicleDialogueNodeDetails.generated.h"

class UDialogueTree;

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueNodeDetails : public UObject
{
    GENERATED_BODY()

public:
    void LoadFromNode(UDialogueTree* InTree, const FGuid& InNodeGuid);
    bool ApplyToNode(FString& OutError);
    FGuid GetTargetNodeGuid() const { return TargetNodeGuid; }

    UPROPERTY(VisibleAnywhere, Category="Chronicle|Node")
    FGuid TargetNodeGuid;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    EDialogueNodeType NodeType = EDialogueNodeType::Speech;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    TArray<FDialogueLine> Lines;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    TArray<FDialogueChoice> Choices;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    FString ConditionExpression;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    TMap<FName, FString> EventPayload;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    bool bEventIsAsync = false;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    float WaitTime = -1.0f;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    int32 DefaultOutputIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, Category="Chronicle|Node")
    bool bAutoSelectIfSingle = false;

    UPROPERTY(EditAnywhere, Category="Chronicle|Breakpoint")
    bool bBreakpointEnabled = false;

    UPROPERTY(EditAnywhere, Category="Chronicle|Breakpoint")
    FString BreakpointNote;

private:
    UPROPERTY()
    TObjectPtr<UDialogueTree> SourceTree;
};
