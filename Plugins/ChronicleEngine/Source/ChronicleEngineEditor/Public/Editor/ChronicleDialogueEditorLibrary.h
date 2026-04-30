#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueEditorLibrary.generated.h"

class UDialogueTree;
class UDialogueDatabase;
class UDialogueRunner;

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueDebuggerVariableSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FGameplayTag VariableTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FVariableValue Value;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    EChronicleVariableScope Scope = EChronicleVariableScope::Global;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueDebuggerEdgeSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FGuid TargetNodeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    int32 FromSlotIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FString ConditionExpression;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    bool bConditionPasses = true;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    float Weight = 1.0f;
};

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
    EDialogueNodeType CurrentNodeType = EDialogueNodeType::Speech;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    int32 CurrentLineIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    EDialogueRunnerState RunnerState = EDialogueRunnerState::Idle;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    bool bNodeHasBreakpoint = false;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    FText NodeTitle;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    TArray<FChronicleDialogueDebuggerVariableSnapshot> Variables;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    TArray<FChronicleDialogueDebuggerEdgeSnapshot> OutgoingEdges;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    TArray<FDialogueHistoryEntry> History;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Debugger")
    TArray<FString> SeenDialogueHashes;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleConditionExpressionValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Expression")
    bool bParsed = false;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Expression")
    bool bEvaluationResult = false;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Expression")
    TArray<FString> VariableReferences;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Expression")
    FString Message;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialoguePipelineExportPaths
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Pipeline")
    FString JsonFilePath;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Pipeline")
    FString LinesCsvFilePath;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Pipeline")
    FString LocalizationCsvFilePath;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Pipeline")
    FString AuditJsonFilePath;
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

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Expression")
    static bool ValidateConditionExpressionForTree(UDialogueTree* Tree, const FString& Expression, FChronicleConditionExpressionValidationResult& OutResult, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Pipeline")
    static bool BuildDefaultDialogueTreeExportPaths(UDialogueTree* Tree, const FString& ExportDirectory, FChronicleDialoguePipelineExportPaths& OutPaths, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Pipeline")
    static bool ExportDialogueTreePipelineArtifacts(UDialogueTree* Tree, const FString& ExportDirectory, const FString& Culture, FChronicleDialoguePipelineExportPaths& OutPaths, FString& OutError);

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
