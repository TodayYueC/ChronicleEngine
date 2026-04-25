#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Widgets/SCompoundWidget.h"

class UDialogueTree;
class SBox;
class STextBlock;

struct FDialogueEdge;

class SDialogueTreeEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDialogueTreeEditor) {}
        SLATE_ARGUMENT(UDialogueTree*, DialogueTree)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    FReply HandleAddNodeClicked(EDialogueNodeType NodeType);
    FReply HandleValidateClicked();
    FReply HandleNodeSelected(FGuid NodeGuid);
    FReply HandleStartLinkClicked(FGuid NodeGuid);
    FReply HandleLinkHereClicked(FGuid TargetNodeGuid);
    FReply HandleDeleteEdgeClicked(FGuid FromNodeGuid, FGuid ToNodeGuid, int32 FromSlotIndex, FString ConditionExpression);
    void HandleNodeMoveCommitted(FGuid NodeGuid, FVector2D NewPosition);
    void HandleEdgeSlotChanged(int32 NewValue);
    void HandleEdgeConditionChanged(const FText& InText);
    void HandleSearchTextChanged(const FText& InText);
    int32 GetEdgeSlotIndex() const;
    FText GetLinkStateText() const;
    void RefreshCanvas();
    void RefreshInspector();
    void RefreshValidationSummary();
    TSharedRef<SWidget> BuildCanvas();
    TSharedRef<SWidget> BuildNodeCard(const FDialogueNode& Node);
    TSharedRef<SWidget> BuildEdgeList();
    TArray<FDialogueEdge> GetVisibleEdges(const TSet<FGuid>& VisibleSet) const;
    FLinearColor GetNodeColor(EDialogueNodeType NodeType) const;
    FString GetNodeSummary(const FDialogueNode& Node) const;
    FString GetShortGuid(const FGuid& Guid) const;

    TWeakObjectPtr<UDialogueTree> DialogueTree;
    TSharedPtr<SBox> CanvasHost;
    TSharedPtr<SBox> EdgeListHost;
    TSharedPtr<STextBlock> ValidationText;
    FString SearchQuery;
    TArray<FGuid> VisibleNodeGuids;
    FGuid SelectedNodeGuid;
    FGuid PendingEdgeSourceGuid;
    int32 EdgeSlotIndex = 0;
    FString EdgeCondition;
};
