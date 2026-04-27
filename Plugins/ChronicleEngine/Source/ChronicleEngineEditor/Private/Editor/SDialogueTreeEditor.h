#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Editor/ChronicleDialogueGraph.h"
#include "Editor/ChronicleDialogueNodeDetails.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class SGraphEditor;
class UDialogueTree;
class IDetailsView;
class SBox;
class STextBlock;
struct FEdGraphEditAction;
struct FPropertyChangedEvent;

struct FDialogueEdge;

class SDialogueTreeEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDialogueTreeEditor) {}
        SLATE_ARGUMENT(UDialogueTree*, DialogueTree)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
    FReply HandleAddNodeClicked(EDialogueNodeType NodeType);
    FReply HandleValidateClicked();
    FReply HandleSaveGraphLayoutClicked();
    FReply HandleUseSelectedAsLinkSourceClicked();
    FReply HandleCreateEdgeToSelectedClicked();
    FReply HandleAcquireLockClicked();
    FReply HandleReleaseLockClicked();
    FReply HandleToggleBreakpointClicked();
    FReply HandleNodeSelected(FGuid NodeGuid);
    FReply HandleStartLinkClicked(FGuid NodeGuid);
    FReply HandleLinkHereClicked(FGuid TargetNodeGuid);
    FReply HandleDeleteEdgeClicked(FGuid FromNodeGuid, FGuid ToNodeGuid, int32 FromSlotIndex, FString ConditionExpression);
    void HandleNodeMoveCommitted(FGuid NodeGuid, FVector2D NewPosition);
    void HandleEdgeSlotChanged(int32 NewValue);
    void HandleEdgeConditionChanged(const FText& InText);
    void HandleSearchTextChanged(const FText& InText);
    void HandleGraphChanged(const FEdGraphEditAction& Action);
    void HandleGraphSelectionChanged(const TSet<UObject*>& Selection);
    void HandleNodeDetailsChanged(const FPropertyChangedEvent& PropertyChangedEvent);
    int32 GetEdgeSlotIndex() const;
    FText GetLinkStateText() const;
    void RebuildGraph();
    void RefreshCanvas();
    void RefreshInspector();
    void RefreshLockSummary();
    void RefreshDebuggerSummary();
    void RefreshValidationSummary();
    TSharedRef<SWidget> BuildCanvas();
    TSharedRef<SWidget> BuildNodeCard(const FDialogueNode& Node);
    TSharedRef<SWidget> BuildEdgeList();
    TArray<FDialogueEdge> GetVisibleEdges(const TSet<FGuid>& VisibleSet) const;
    FLinearColor GetNodeColor(EDialogueNodeType NodeType) const;
    FString GetNodeSummary(const FDialogueNode& Node) const;
    FString GetShortGuid(const FGuid& Guid) const;

    TWeakObjectPtr<UDialogueTree> DialogueTree;
    TStrongObjectPtr<UChronicleDialogueGraph> DialogueGraph;
    TStrongObjectPtr<UChronicleDialogueNodeDetails> NodeDetails;
    TSharedPtr<SGraphEditor> GraphEditor;
    TSharedPtr<IDetailsView> NodeDetailsView;
    FDelegateHandle GraphChangedHandle;
    TSharedPtr<SBox> CanvasHost;
    TSharedPtr<SBox> EdgeListHost;
    TSharedPtr<STextBlock> LockText;
    TSharedPtr<STextBlock> DebuggerText;
    TSharedPtr<STextBlock> ValidationText;
    FString SearchQuery;
    TArray<FGuid> VisibleNodeGuids;
    FGuid SelectedNodeGuid;
    FGuid PendingEdgeSourceGuid;
    int32 EdgeSlotIndex = 0;
    FString EdgeCondition;
};
