#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueEditorLibrary.generated.h"

class UDialogueTree;
class UDialogueDatabase;
class UDialogueRunner;

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueDebuggerSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    TObjectPtr<UDialogueTree> CurrentTree = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FGuid CurrentTreeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FGuid CurrentNodeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    EDialogueRunnerState RunnerState = EDialogueRunnerState::Idle;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    bool bNodeHasBreakpoint = false;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FText NodeTitle;
};

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
    static bool RemoveDialogueNodes(UDialogueTree* Tree, const TArray<FGuid>& NodeGuids, int32& OutRemovedNodeCount, int32& OutRemovedEdgeCount, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool DuplicateDialogueNodes(UDialogueTree* Tree, const TArray<FGuid>& NodeGuids, FVector2D PositionOffset, TArray<FGuid>& OutDuplicatedNodeGuids, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool AddDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, FDialogueEdge& OutEdge, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool RemoveDialogueEdge(UDialogueTree* Tree, const FGuid& FromNodeGuid, const FGuid& ToNodeGuid, int32 FromSlotIndex, const FString& ConditionExpression, int32& OutRemovedCount, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool SetDialogueNodeBreakpoint(UDialogueTree* Tree, const FGuid& NodeGuid, bool bEnabled, const FString& Note, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static bool IsDialogueNodeBreakpointSet(UDialogueTree* Tree, const FGuid& NodeGuid);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static int32 GetDialogueNodeBreakpoints(UDialogueTree* Tree, TArray<FGuid>& OutNodeGuids);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Dialogue Tree")
    static int32 SearchDialogueNodes(UDialogueTree* Tree, const FString& Query, TArray<FGuid>& OutNodeGuids);

    UFUNCTION(BlueprintPure, Category="Chronicle|Editor|Dialogue Tree")
    static FText GetNodeTypeDisplayName(EDialogueNodeType NodeType);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Debugger")
    static bool CaptureDebuggerSnapshot(UDialogueRunner* Runner, FChronicleDialogueDebuggerSnapshot& OutSnapshot, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool AcquireDialogueTreeLock(UDialogueTree* Tree, const FString& Note, FChronicleSoftLockMetadata& OutLock, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool ReleaseDialogueTreeLock(UDialogueTree* Tree, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool IsDialogueTreeLockedByOtherUser(UDialogueTree* Tree);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool AcquireDialogueDatabaseLock(UDialogueDatabase* Database, const FString& Note, FChronicleSoftLockMetadata& OutLock, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool ReleaseDialogueDatabaseLock(UDialogueDatabase* Database, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Lock")
    static bool IsDialogueDatabaseLockedByOtherUser(UDialogueDatabase* Database);
};
