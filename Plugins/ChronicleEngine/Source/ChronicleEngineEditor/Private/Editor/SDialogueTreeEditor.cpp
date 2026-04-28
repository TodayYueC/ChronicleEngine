#include "Editor/SDialogueTreeEditor.h"

#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Data/DialogueTree.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Editor/ChronicleDialogueGraph.h"
#include "Editor/ChronicleDialogueGraphNode.h"
#include "Editor/ChronicleDialogueNodeDetails.h"
#include "EdGraph/EdGraph.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "GraphEditor.h"
#include "IDetailsView.h"
#include "InputCoreTypes.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SDialogueTreeEditor"

namespace
{
constexpr float CanvasWidth = 2400.0f;
constexpr float CanvasHeight = 1500.0f;
constexpr float NodeWidth = 276.0f;
constexpr float NodeHeight = 174.0f;
const FVector2D CanvasOrigin(800.0f, 360.0f);

struct FDialogueEdgeVisual
{
    FVector2D Start;
    FVector2D End;
    FLinearColor Color = FLinearColor::White;
};

DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnDialogueNodeCommand, FGuid);
DECLARE_DELEGATE_TwoParams(FOnDialogueNodeMoveCommitted, FGuid, FVector2D);

FString GetShortGuidText(const FGuid& Guid)
{
    return Guid.ToString(EGuidFormats::Digits).Left(8);
}

FVector2D GetNodeCanvasPosition(const FDialogueNode& Node)
{
    return Node.Position + CanvasOrigin;
}

class SDialogueEdgeLayer : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SDialogueEdgeLayer) {}
        SLATE_ARGUMENT(TArray<FDialogueEdgeVisual>, Edges)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Edges = InArgs._Edges;
    }

    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
    {
        return FVector2D(CanvasWidth, CanvasHeight);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
    {
        for (const FDialogueEdgeVisual& Edge : Edges)
        {
            TArray<FVector2D> Points;
            Points.Add(Edge.Start);
            Points.Add(Edge.End);
            FSlateDrawElement::MakeLines(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(),
                Points,
                ESlateDrawEffect::None,
                Edge.Color,
                true,
                2.5f);
        }

        return LayerId + 1;
    }

private:
    TArray<FDialogueEdgeVisual> Edges;
};

class SDialogueNodeCard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDialogueNodeCard) {}
        SLATE_ARGUMENT(FDialogueNode, Node)
        SLATE_ARGUMENT(bool, bSelected)
        SLATE_ARGUMENT(bool, bCanLinkHere)
        SLATE_ARGUMENT(FLinearColor, NodeColor)
        SLATE_ARGUMENT(FString, NodeSummary)
        SLATE_EVENT(FOnDialogueNodeCommand, OnSelected)
        SLATE_EVENT(FOnDialogueNodeCommand, OnStartLink)
        SLATE_EVENT(FOnDialogueNodeCommand, OnLinkHere)
        SLATE_EVENT(FOnDialogueNodeMoveCommitted, OnMoveCommitted)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Node = InArgs._Node;
        bSelected = InArgs._bSelected;
        bCanLinkHere = InArgs._bCanLinkHere;
        NodeColor = InArgs._NodeColor;
        NodeSummary = InArgs._NodeSummary;
        OnSelected = InArgs._OnSelected;
        OnStartLink = InArgs._OnStartLink;
        OnLinkHere = InArgs._OnLinkHere;
        OnMoveCommitted = InArgs._OnMoveCommitted;

        ChildSlot
        [
            SNew(SBorder)
            .Padding(10.0f)
            .BorderBackgroundColor(bSelected ? FLinearColor(0.95f, 0.75f, 0.18f, 1.0f) : NodeColor)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(UChronicleDialogueEditorLibrary::GetNodeTypeDisplayName(Node.NodeType))
                    .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    .ColorAndOpacity(FLinearColor::White)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 4.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(NodeSummary))
                    .AutoWrapText(true)
                    .ColorAndOpacity(FLinearColor::White)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 4.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(GetShortGuidText(Node.NodeGuid)))
                    .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 0.8f))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 6.0f, 0.0f, 0.0f)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(0.0f, 0.0f, 4.0f, 0.0f)
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("SelectNode", "Select"))
                        .OnClicked(this, &SDialogueNodeCard::HandleSelectClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(0.0f, 0.0f, 4.0f, 0.0f)
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("StartLink", "Link From"))
                        .OnClicked(this, &SDialogueNodeCard::HandleStartLinkClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("LinkHere", "Link Here"))
                        .Visibility(bCanLinkHere ? EVisibility::Visible : EVisibility::Collapsed)
                        .OnClicked(this, &SDialogueNodeCard::HandleLinkHereClicked)
                    ]
                ]
            ]
        ];
    }

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
        {
            return FReply::Unhandled();
        }

        bDragging = true;
        DragStartScreenPosition = MouseEvent.GetScreenSpacePosition();
        DragStartNodePosition = Node.Position;
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }

    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
    {
        if (!bDragging || MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
        {
            return FReply::Unhandled();
        }

        bDragging = false;
        const float GeometryScale = MyGeometry.GetAccumulatedLayoutTransform().GetScale();
        const FVector2D DragDelta = GeometryScale > KINDA_SMALL_NUMBER
            ? (MouseEvent.GetScreenSpacePosition() - DragStartScreenPosition) / GeometryScale
            : MouseEvent.GetScreenSpacePosition() - DragStartScreenPosition;
        OnMoveCommitted.ExecuteIfBound(Node.NodeGuid, DragStartNodePosition + DragDelta);
        return FReply::Handled().ReleaseMouseCapture();
    }

private:
    FReply HandleSelectClicked() const
    {
        return OnSelected.IsBound() ? OnSelected.Execute(Node.NodeGuid) : FReply::Handled();
    }

    FReply HandleStartLinkClicked() const
    {
        return OnStartLink.IsBound() ? OnStartLink.Execute(Node.NodeGuid) : FReply::Handled();
    }

    FReply HandleLinkHereClicked() const
    {
        return OnLinkHere.IsBound() ? OnLinkHere.Execute(Node.NodeGuid) : FReply::Handled();
    }

    FDialogueNode Node;
    bool bSelected = false;
    bool bCanLinkHere = false;
    bool bDragging = false;
    FVector2D DragStartScreenPosition = FVector2D::ZeroVector;
    FVector2D DragStartNodePosition = FVector2D::ZeroVector;
    FLinearColor NodeColor = FLinearColor::White;
    FString NodeSummary;
    FOnDialogueNodeCommand OnSelected;
    FOnDialogueNodeCommand OnStartLink;
    FOnDialogueNodeCommand OnLinkHere;
    FOnDialogueNodeMoveCommitted OnMoveCommitted;
};
}

void SDialogueTreeEditor::Construct(const FArguments& InArgs)
{
    DialogueTree = InArgs._DialogueTree;
    SearchQuery.Reset();
    SelectedNodeGuid.Invalidate();
    PendingEdgeSourceGuid.Invalidate();
    EdgeSlotIndex = 0;
    EdgeCondition.Reset();
    RebuildGraph();
    BindGraphCommands();

    NodeDetails = TStrongObjectPtr<UChronicleDialogueNodeDetails>(NewObject<UChronicleDialogueNodeDetails>(GetTransientPackage()));
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;
    DetailsViewArgs.bAllowSearch = true;
    DetailsViewArgs.bShowOptions = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
    NodeDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    NodeDetailsView->OnFinishedChangingProperties().AddSP(this, &SDialogueTreeEditor::HandleNodeDetailsChanged);

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("AddSpeech", "Add Speech"))
                .OnClicked(this, &SDialogueTreeEditor::HandleAddNodeClicked, EDialogueNodeType::Speech)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("AddChoice", "Add Choice"))
                .OnClicked(this, &SDialogueTreeEditor::HandleAddNodeClicked, EDialogueNodeType::Choice)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("AddCondition", "Add Condition"))
                .OnClicked(this, &SDialogueTreeEditor::HandleAddNodeClicked, EDialogueNodeType::Condition)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("Validate", "Validate"))
                .OnClicked(this, &SDialogueTreeEditor::HandleValidateClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("SaveLayout", "Save Layout"))
                .OnClicked(this, &SDialogueTreeEditor::HandleSaveGraphLayoutClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("UseSelectedAsSource", "Use As Source"))
                .OnClicked(this, &SDialogueTreeEditor::HandleUseSelectedAsLinkSourceClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("CreateEdgeToSelected", "Link To Selected"))
                .OnClicked(this, &SDialogueTreeEditor::HandleCreateEdgeToSelectedClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("CopySelected", "Copy"))
                .IsEnabled(this, &SDialogueTreeEditor::HasSelectedDialogueNodes)
                .OnClicked(this, &SDialogueTreeEditor::HandleCopySelectedNodesClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("PasteNodes", "Paste"))
                .IsEnabled(this, &SDialogueTreeEditor::CanPasteNodes)
                .OnClicked(this, &SDialogueTreeEditor::HandlePasteNodesClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("DuplicateSelected", "Duplicate"))
                .IsEnabled(this, &SDialogueTreeEditor::HasSelectedDialogueNodes)
                .OnClicked(this, &SDialogueTreeEditor::HandleDuplicateSelectedNodesClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("DeleteSelected", "Delete"))
                .IsEnabled(this, &SDialogueTreeEditor::HasSelectedDialogueNodes)
                .OnClicked(this, &SDialogueTreeEditor::HandleDeleteSelectedNodesClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("ZoomToFit", "Zoom To Fit"))
                .OnClicked(this, &SDialogueTreeEditor::HandleZoomToFitClicked)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(8.0f, 2.0f)
            [
                SNew(SSearchBox)
                .HintText(LOCTEXT("SearchHint", "Search nodes, speakers, line text, GUIDs"))
                .OnTextChanged(this, &SDialogueTreeEditor::HandleSearchTextChanged)
            ]
        ]

        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SSplitter)

            + SSplitter::Slot()
            .Value(0.76f)
            [
                SNew(SBorder)
                .Padding(0.0f)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                [
                    SNew(SBox)
                    .MinDesiredWidth(900.0f)
                    .MinDesiredHeight(600.0f)
                    [
                        SAssignNew(CanvasHost, SBox)
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.24f)
            [
                SNew(SBorder)
                .Padding(10.0f)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("InspectorTitle", "Chronicle Tree"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f)
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this]()
                        {
                            if (!DialogueTree.IsValid())
                            {
                                return LOCTEXT("NoTree", "No tree loaded.");
                            }

                            return FText::Format(
                                LOCTEXT("TreeStats", "Nodes: {0}\nEdges: {1}\nVisible: {2}"),
                                FText::AsNumber(DialogueTree->Nodes.Num()),
                                FText::AsNumber(DialogueTree->Edges.Num()),
                                FText::AsNumber(VisibleNodeGuids.Num()));
                        })
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("LockTitle", "Soft Lock"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SAssignNew(LockText, STextBlock)
                        .AutoWrapText(true)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 4.0f)
                    [
                        SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0.0f, 0.0f, 4.0f, 0.0f)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("AcquireLock", "Acquire"))
                            .OnClicked(this, &SDialogueTreeEditor::HandleAcquireLockClicked)
                        ]

                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("ReleaseLock", "Release"))
                            .OnClicked(this, &SDialogueTreeEditor::HandleReleaseLockClicked)
                        ]
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("DebuggerTitle", "PIE Debugger"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SAssignNew(DebuggerText, STextBlock)
                        .AutoWrapText(true)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 4.0f)
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("ToggleBreakpoint", "Toggle Breakpoint"))
                        .OnClicked(this, &SDialogueTreeEditor::HandleToggleBreakpointClicked)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("LinkAuthoringTitle", "Link Authoring"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(this, &SDialogueTreeEditor::GetLinkStateText)
                        .AutoWrapText(true)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 4.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("EdgeSlotLabel", "Output Slot"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SNew(SSpinBox<int32>)
                        .MinValue(0)
                        .MaxValue(128)
                        .Value(this, &SDialogueTreeEditor::GetEdgeSlotIndex)
                        .OnValueChanged(this, &SDialogueTreeEditor::HandleEdgeSlotChanged)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 4.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("EdgeConditionLabel", "Edge Condition"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SNew(SEditableTextBox)
                        .HintText(LOCTEXT("EdgeConditionHint", "Optional condition expression"))
                        .OnTextChanged(this, &SDialogueTreeEditor::HandleEdgeConditionChanged)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("OutgoingEdgesTitle", "Selected Node Edges"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 2.0f)
                    [
                        SAssignNew(EdgeListHost, SBox)
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 2.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("NodeDetailsTitle", "Selected Node Details"))
                        .Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                    ]

                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    .Padding(0.0f, 2.0f)
                    [
                        NodeDetailsView.ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f)
                    [
                        SAssignNew(ValidationText, STextBlock)
                        .AutoWrapText(true)
                    ]
                ]
            ]
        ]
    ];

    RefreshCanvas();
    RefreshInspector();
    RefreshLockSummary();
    RefreshDebuggerSummary();
    RefreshValidationSummary();
}

FVector2D SDialogueTreeEditor::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return FVector2D(1280.0f, 720.0f);
}

FReply SDialogueTreeEditor::HandleAddNodeClicked(EDialogueNodeType NodeType)
{
    if (!DialogueTree.IsValid())
    {
        return FReply::Handled();
    }

    FGuid NewNodeGuid;
    FString Error;
    const FVector2D Position(120.0f + DialogueTree->Nodes.Num() * 40.0f, 120.0f + DialogueTree->Nodes.Num() * 28.0f);
    UChronicleDialogueEditorLibrary::AddDialogueNode(DialogueTree.Get(), NodeType, Position, NewNodeGuid, Error);
    if (NewNodeGuid.IsValid())
    {
        SelectedNodeGuid = NewNodeGuid;
    }

    RebuildGraph();
    RefreshCanvas();
    RefreshInspector();
    RefreshValidationSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleValidateClicked()
{
    RefreshValidationSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleSaveGraphLayoutClicked()
{
    if (DialogueGraph.IsValid())
    {
        DialogueGraph->SynchronizeNodePositionsToDialogueTree();
    }

    RefreshInspector();
    RefreshValidationSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleUseSelectedAsLinkSourceClicked()
{
    if (SelectedNodeGuid.IsValid())
    {
        PendingEdgeSourceGuid = SelectedNodeGuid;
    }

    RefreshInspector();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleCreateEdgeToSelectedClicked()
{
    if (!DialogueTree.IsValid() || !PendingEdgeSourceGuid.IsValid() || !SelectedNodeGuid.IsValid() || PendingEdgeSourceGuid == SelectedNodeGuid)
    {
        return FReply::Handled();
    }

    FDialogueEdge NewEdge;
    FString Error;
    if (UChronicleDialogueEditorLibrary::AddDialogueEdge(DialogueTree.Get(), PendingEdgeSourceGuid, SelectedNodeGuid, EdgeSlotIndex, EdgeCondition, NewEdge, Error))
    {
        RebuildGraph();
        RefreshCanvas();
        RefreshInspector();
        RefreshValidationSummary();
    }

    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleCopySelectedNodesClicked()
{
    ExecuteCopySelectedNodes();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandlePasteNodesClicked()
{
    ExecutePasteNodes();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleDuplicateSelectedNodesClicked()
{
    ExecuteDuplicateSelectedNodes();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleDeleteSelectedNodesClicked()
{
    ExecuteDeleteSelectedNodes();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleZoomToFitClicked()
{
    ExecuteZoomToFit();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleAcquireLockClicked()
{
    if (DialogueTree.IsValid())
    {
        FChronicleSoftLockMetadata Lock;
        FString Error;
        UChronicleDialogueEditorLibrary::AcquireDialogueTreeLock(DialogueTree.Get(), TEXT("Editing in Chronicle Dialogue Tree Editor"), Lock, Error);
    }

    RefreshLockSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleReleaseLockClicked()
{
    if (DialogueTree.IsValid())
    {
        FString Error;
        UChronicleDialogueEditorLibrary::ReleaseDialogueTreeLock(DialogueTree.Get(), Error);
    }

    RefreshLockSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleToggleBreakpointClicked()
{
    if (DialogueTree.IsValid() && SelectedNodeGuid.IsValid())
    {
        FString Error;
        const bool bNewBreakpointState = !UChronicleDialogueEditorLibrary::IsDialogueNodeBreakpointSet(DialogueTree.Get(), SelectedNodeGuid);
        UChronicleDialogueEditorLibrary::SetDialogueNodeBreakpoint(DialogueTree.Get(), SelectedNodeGuid, bNewBreakpointState, FString(), Error);
        RefreshInspector();
        RefreshDebuggerSummary();
    }

    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleNodeSelected(FGuid NodeGuid)
{
    SelectedNodeGuid = NodeGuid;
    RefreshCanvas();
    RefreshInspector();
    RefreshDebuggerSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleStartLinkClicked(FGuid NodeGuid)
{
    PendingEdgeSourceGuid = NodeGuid;
    SelectedNodeGuid = NodeGuid;
    RefreshCanvas();
    RefreshInspector();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleLinkHereClicked(FGuid TargetNodeGuid)
{
    if (!DialogueTree.IsValid() || !PendingEdgeSourceGuid.IsValid() || PendingEdgeSourceGuid == TargetNodeGuid)
    {
        return FReply::Handled();
    }

    FDialogueEdge NewEdge;
    FString Error;
    if (UChronicleDialogueEditorLibrary::AddDialogueEdge(DialogueTree.Get(), PendingEdgeSourceGuid, TargetNodeGuid, EdgeSlotIndex, EdgeCondition, NewEdge, Error))
    {
        SelectedNodeGuid = PendingEdgeSourceGuid;
        RebuildGraph();
    }

    RefreshCanvas();
    RefreshInspector();
    RefreshValidationSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleDeleteEdgeClicked(FGuid FromNodeGuid, FGuid ToNodeGuid, int32 FromSlotIndex, FString ConditionExpression)
{
    if (!DialogueTree.IsValid())
    {
        return FReply::Handled();
    }

    int32 RemovedCount = 0;
    FString Error;
    UChronicleDialogueEditorLibrary::RemoveDialogueEdge(DialogueTree.Get(), FromNodeGuid, ToNodeGuid, FromSlotIndex, ConditionExpression, RemovedCount, Error);
    if (RemovedCount > 0)
    {
        RebuildGraph();
    }

    RefreshCanvas();
    RefreshInspector();
    RefreshValidationSummary();
    return FReply::Handled();
}

void SDialogueTreeEditor::HandleNodeMoveCommitted(FGuid NodeGuid, FVector2D NewPosition)
{
    if (!DialogueTree.IsValid())
    {
        return;
    }

    FString Error;
    if (UChronicleDialogueEditorLibrary::SetDialogueNodePosition(DialogueTree.Get(), NodeGuid, NewPosition, Error))
    {
        RefreshCanvas();
        RefreshInspector();
    }
}

void SDialogueTreeEditor::HandleEdgeSlotChanged(int32 NewValue)
{
    EdgeSlotIndex = FMath::Max(0, NewValue);
}

void SDialogueTreeEditor::HandleEdgeConditionChanged(const FText& InText)
{
    EdgeCondition = InText.ToString();
}

void SDialogueTreeEditor::HandleSearchTextChanged(const FText& InText)
{
    SearchQuery = InText.ToString();
    if (GraphEditor.IsValid() && DialogueGraph.IsValid() && DialogueTree.IsValid())
    {
        GraphEditor->ClearSelectionSet();

        if (SearchQuery.TrimStartAndEnd().IsEmpty())
        {
            SelectedNodeGuid.Invalidate();
            RefreshInspector();
            return;
        }

        TArray<FGuid> SearchResults;
        UChronicleDialogueEditorLibrary::SearchDialogueNodes(DialogueTree.Get(), SearchQuery, SearchResults);
        for (const FGuid& NodeGuid : SearchResults)
        {
            if (UChronicleDialogueGraphNode* GraphNode = DialogueGraph->FindDialogueGraphNode(NodeGuid))
            {
                GraphEditor->SetNodeSelection(GraphNode, true);
            }
        }

        if (SearchResults.Num() > 0)
        {
            SelectedNodeGuid = SearchResults[0];
            GraphEditor->ZoomToFit(true);
        }
    }

    RefreshInspector();
}

void SDialogueTreeEditor::HandleGraphChanged(const FEdGraphEditAction& Action)
{
    if (DialogueGraph.IsValid())
    {
        DialogueGraph->SynchronizeNodePositionsToDialogueTree();
    }

    RefreshInspector();
    RefreshValidationSummary();
}

void SDialogueTreeEditor::HandleGraphSelectionChanged(const TSet<UObject*>& Selection)
{
    SelectedNodeGuid.Invalidate();
    for (UObject* SelectedObject : Selection)
    {
        if (const UChronicleDialogueGraphNode* GraphNode = Cast<UChronicleDialogueGraphNode>(SelectedObject))
        {
            SelectedNodeGuid = GraphNode->DialogueNodeGuid;
            break;
        }
    }

    RefreshInspector();
    RefreshDebuggerSummary();
}

void SDialogueTreeEditor::HandleNodeDetailsChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
    if (!NodeDetails.IsValid())
    {
        return;
    }

    FString Error;
    if (NodeDetails->ApplyToNode(Error))
    {
        RebuildGraph();
        RefreshCanvas();
        RefreshInspector();
        RefreshDebuggerSummary();
        RefreshValidationSummary();
    }
}

int32 SDialogueTreeEditor::GetEdgeSlotIndex() const
{
    return EdgeSlotIndex;
}

FText SDialogueTreeEditor::GetLinkStateText() const
{
    if (!PendingEdgeSourceGuid.IsValid())
    {
        return LOCTEXT("NoPendingLink", "Click Link From on a source node, then Link Here on a target node.");
    }

    return FText::Format(
        LOCTEXT("PendingLink", "Source: {0}\nSlot: {1}"),
        FText::FromString(GetShortGuid(PendingEdgeSourceGuid)),
        FText::AsNumber(EdgeSlotIndex));
}

bool SDialogueTreeEditor::HasSelectedDialogueNodes() const
{
    return GetSelectedDialogueNodeGuids().Num() > 0;
}

bool SDialogueTreeEditor::CanPasteNodes() const
{
    return DialogueTree.IsValid() && CopiedNodeGuids.Num() > 0;
}

void SDialogueTreeEditor::BindGraphCommands()
{
    GraphCommandList = MakeShared<FUICommandList>();
    const FGenericCommands& GenericCommands = FGenericCommands::Get();

    GraphCommandList->MapAction(
        GenericCommands.Copy,
        FExecuteAction::CreateSP(this, &SDialogueTreeEditor::ExecuteCopySelectedNodes),
        FCanExecuteAction::CreateSP(this, &SDialogueTreeEditor::HasSelectedDialogueNodes));

    GraphCommandList->MapAction(
        GenericCommands.Paste,
        FExecuteAction::CreateSP(this, &SDialogueTreeEditor::ExecutePasteNodes),
        FCanExecuteAction::CreateSP(this, &SDialogueTreeEditor::CanPasteNodes));

    GraphCommandList->MapAction(
        GenericCommands.Duplicate,
        FExecuteAction::CreateSP(this, &SDialogueTreeEditor::ExecuteDuplicateSelectedNodes),
        FCanExecuteAction::CreateSP(this, &SDialogueTreeEditor::HasSelectedDialogueNodes));

    GraphCommandList->MapAction(
        GenericCommands.Delete,
        FExecuteAction::CreateSP(this, &SDialogueTreeEditor::ExecuteDeleteSelectedNodes),
        FCanExecuteAction::CreateSP(this, &SDialogueTreeEditor::HasSelectedDialogueNodes));
}

void SDialogueTreeEditor::ExecuteCopySelectedNodes()
{
    CopiedNodeGuids = GetSelectedDialogueNodeGuids();
}

void SDialogueTreeEditor::ExecutePasteNodes()
{
    if (!DialogueTree.IsValid() || CopiedNodeGuids.Num() == 0)
    {
        return;
    }

    TArray<FGuid> DuplicatedNodeGuids;
    FString Error;
    if (!UChronicleDialogueEditorLibrary::DuplicateDialogueNodes(DialogueTree.Get(), CopiedNodeGuids, FVector2D(160.0f, 80.0f), DuplicatedNodeGuids, Error))
    {
        return;
    }

    CopiedNodeGuids = DuplicatedNodeGuids;
    RebuildGraph();
    RefreshCanvas();
    SelectGraphNodes(DuplicatedNodeGuids);
    SelectedNodeGuid = DuplicatedNodeGuids.Num() > 0 ? DuplicatedNodeGuids[0] : FGuid();
    RefreshInspector();
    RefreshValidationSummary();
}

void SDialogueTreeEditor::ExecuteDuplicateSelectedNodes()
{
    if (!DialogueTree.IsValid())
    {
        return;
    }

    const TArray<FGuid> SelectedNodeGuids = GetSelectedDialogueNodeGuids();
    if (SelectedNodeGuids.Num() == 0)
    {
        return;
    }

    TArray<FGuid> DuplicatedNodeGuids;
    FString Error;
    if (!UChronicleDialogueEditorLibrary::DuplicateDialogueNodes(DialogueTree.Get(), SelectedNodeGuids, FVector2D(160.0f, 80.0f), DuplicatedNodeGuids, Error))
    {
        return;
    }

    CopiedNodeGuids = SelectedNodeGuids;
    RebuildGraph();
    RefreshCanvas();
    SelectGraphNodes(DuplicatedNodeGuids);
    SelectedNodeGuid = DuplicatedNodeGuids.Num() > 0 ? DuplicatedNodeGuids[0] : FGuid();
    RefreshInspector();
    RefreshValidationSummary();
}

void SDialogueTreeEditor::ExecuteDeleteSelectedNodes()
{
    if (!DialogueTree.IsValid())
    {
        return;
    }

    const TArray<FGuid> SelectedNodeGuids = GetSelectedDialogueNodeGuids();
    if (SelectedNodeGuids.Num() == 0)
    {
        return;
    }

    int32 RemovedNodeCount = 0;
    int32 RemovedEdgeCount = 0;
    FString Error;
    if (!UChronicleDialogueEditorLibrary::RemoveDialogueNodes(DialogueTree.Get(), SelectedNodeGuids, RemovedNodeCount, RemovedEdgeCount, Error))
    {
        return;
    }

    SelectedNodeGuid.Invalidate();
    PendingEdgeSourceGuid.Invalidate();
    RebuildGraph();
    RefreshCanvas();
    RefreshInspector();
    RefreshDebuggerSummary();
    RefreshValidationSummary();
}

void SDialogueTreeEditor::ExecuteZoomToFit()
{
    if (GraphEditor.IsValid())
    {
        GraphEditor->ZoomToFit(false);
    }
}

TArray<FGuid> SDialogueTreeEditor::GetSelectedDialogueNodeGuids() const
{
    TArray<FGuid> SelectedNodeGuids;
    TSet<FGuid> SeenNodeGuids;

    if (GraphEditor.IsValid())
    {
        const FGraphPanelSelectionSet Selection = GraphEditor->GetSelectedNodes();
        for (UObject* SelectedObject : Selection)
        {
            if (const UChronicleDialogueGraphNode* GraphNode = Cast<UChronicleDialogueGraphNode>(SelectedObject))
            {
                if (GraphNode->DialogueNodeGuid.IsValid() && !SeenNodeGuids.Contains(GraphNode->DialogueNodeGuid))
                {
                    SeenNodeGuids.Add(GraphNode->DialogueNodeGuid);
                    SelectedNodeGuids.Add(GraphNode->DialogueNodeGuid);
                }
            }
        }
    }

    if (SelectedNodeGuids.Num() == 0 && SelectedNodeGuid.IsValid())
    {
        SelectedNodeGuids.Add(SelectedNodeGuid);
    }

    return SelectedNodeGuids;
}

void SDialogueTreeEditor::SelectGraphNodes(const TArray<FGuid>& NodeGuids)
{
    if (!GraphEditor.IsValid() || !DialogueGraph.IsValid())
    {
        return;
    }

    GraphEditor->ClearSelectionSet();
    for (const FGuid& NodeGuid : NodeGuids)
    {
        if (UChronicleDialogueGraphNode* GraphNode = DialogueGraph->FindDialogueGraphNode(NodeGuid))
        {
            GraphEditor->SetNodeSelection(GraphNode, true);
        }
    }

    if (NodeGuids.Num() > 0)
    {
        GraphEditor->ZoomToFit(true);
    }
}

void SDialogueTreeEditor::RebuildGraph()
{
    if (DialogueGraph.IsValid() && GraphChangedHandle.IsValid())
    {
        DialogueGraph->RemoveOnGraphChangedHandler(GraphChangedHandle);
        GraphChangedHandle.Reset();
    }

    DialogueGraph.Reset();
    if (!DialogueTree.IsValid())
    {
        return;
    }

    UChronicleDialogueGraph* NewGraph = NewObject<UChronicleDialogueGraph>(GetTransientPackage());
    NewGraph->Initialize(DialogueTree.Get());
    GraphChangedHandle = NewGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &SDialogueTreeEditor::HandleGraphChanged));
    DialogueGraph = TStrongObjectPtr<UChronicleDialogueGraph>(NewGraph);
}

void SDialogueTreeEditor::RefreshCanvas()
{
    if (!CanvasHost.IsValid())
    {
        return;
    }

    CanvasHost->SetContent(BuildCanvas());
}

void SDialogueTreeEditor::RefreshInspector()
{
    if (!EdgeListHost.IsValid())
    {
        return;
    }

    EdgeListHost->SetContent(BuildEdgeList());

    if (NodeDetailsView.IsValid() && NodeDetails.IsValid())
    {
        if (DialogueTree.IsValid() && SelectedNodeGuid.IsValid())
        {
            NodeDetails->LoadFromNode(DialogueTree.Get(), SelectedNodeGuid);
            NodeDetailsView->SetObject(NodeDetails.Get(), true);
        }
        else
        {
            NodeDetailsView->SetObject(nullptr, true);
        }
    }
}

void SDialogueTreeEditor::RefreshLockSummary()
{
    if (!LockText.IsValid())
    {
        return;
    }

    if (!DialogueTree.IsValid() || !DialogueTree->EditorLock.bLocked)
    {
        LockText->SetText(LOCTEXT("LockOpen", "Unlocked"));
        return;
    }

    LockText->SetText(FText::Format(
        LOCTEXT("LockSummary", "Locked by {0} on {1}\n{2}"),
        FText::FromString(DialogueTree->EditorLock.OwnerUserName),
        FText::FromString(DialogueTree->EditorLock.OwnerMachineName),
        FText::FromString(DialogueTree->EditorLock.Note)));
}

void SDialogueTreeEditor::RefreshDebuggerSummary()
{
    if (!DebuggerText.IsValid())
    {
        return;
    }

    if (!DialogueTree.IsValid() || !SelectedNodeGuid.IsValid())
    {
        DebuggerText->SetText(LOCTEXT("DebuggerNoSelection", "No selected node."));
        return;
    }

    const bool bBreakpointSet = UChronicleDialogueEditorLibrary::IsDialogueNodeBreakpointSet(DialogueTree.Get(), SelectedNodeGuid);
    DebuggerText->SetText(FText::Format(
        LOCTEXT("DebuggerSelection", "Selected: {0}\nBreakpoint: {1}"),
        FText::FromString(GetShortGuid(SelectedNodeGuid)),
        bBreakpointSet ? LOCTEXT("BreakpointOn", "on") : LOCTEXT("BreakpointOff", "off")));
}

void SDialogueTreeEditor::RefreshValidationSummary()
{
    if (!ValidationText.IsValid())
    {
        return;
    }

    if (!DialogueTree.IsValid())
    {
        ValidationText->SetText(LOCTEXT("ValidationNoTree", "Validation unavailable."));
        return;
    }

    TArray<FChronicleDialogueValidationIssue> Issues;
    const bool bValid = UChronicleDialogueJsonLibrary::ValidateDialogueTree(DialogueTree.Get(), Issues);
    if (Issues.Num() == 0)
    {
        ValidationText->SetText(LOCTEXT("ValidationClean", "Validation: clean"));
        return;
    }

    int32 ErrorCount = 0;
    int32 WarningCount = 0;
    for (const FChronicleDialogueValidationIssue& Issue : Issues)
    {
        ErrorCount += Issue.Severity == EChronicleDialogueValidationSeverity::Error ? 1 : 0;
        WarningCount += Issue.Severity == EChronicleDialogueValidationSeverity::Warning ? 1 : 0;
    }

    ValidationText->SetText(FText::Format(
        LOCTEXT("ValidationSummary", "Validation: {0}\nErrors: {1}\nWarnings: {2}\nFirst: {3}"),
        bValid ? LOCTEXT("Valid", "valid") : LOCTEXT("Invalid", "invalid"),
        FText::AsNumber(ErrorCount),
        FText::AsNumber(WarningCount),
        FText::FromString(Issues[0].Message)));
}

TSharedRef<SWidget> SDialogueTreeEditor::BuildCanvas()
{
    VisibleNodeGuids.Reset();

    if (!DialogueGraph.IsValid() || !DialogueTree.IsValid())
    {
        return SNew(STextBlock)
            .Text(LOCTEXT("GraphUnavailable", "Dialogue graph unavailable."));
    }

    UChronicleDialogueEditorLibrary::SearchDialogueNodes(DialogueTree.Get(), SearchQuery, VisibleNodeGuids);

    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = LOCTEXT("GraphCorner", "Chronicle Dialogue Tree");
    AppearanceInfo.InstructionText = LOCTEXT("GraphInstructions", "Drag from output pins to input pins to create dialogue edges.");

    SGraphEditor::FGraphEditorEvents GraphEvents;
    GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &SDialogueTreeEditor::HandleGraphSelectionChanged);

    return SAssignNew(GraphEditor, SGraphEditor)
        .GraphToEdit(DialogueGraph.Get())
        .AdditionalCommands(GraphCommandList)
        .GraphEvents(GraphEvents)
        .Appearance(AppearanceInfo)
        .IsEditable(true);
}

TSharedRef<SWidget> SDialogueTreeEditor::BuildNodeCard(const FDialogueNode& Node)
{
    return SNew(SDialogueNodeCard)
        .Node(Node)
        .bSelected(Node.NodeGuid == SelectedNodeGuid)
        .bCanLinkHere(PendingEdgeSourceGuid.IsValid() && PendingEdgeSourceGuid != Node.NodeGuid)
        .NodeColor(GetNodeColor(Node.NodeType))
        .NodeSummary(GetNodeSummary(Node))
        .OnSelected(this, &SDialogueTreeEditor::HandleNodeSelected)
        .OnStartLink(this, &SDialogueTreeEditor::HandleStartLinkClicked)
        .OnLinkHere(this, &SDialogueTreeEditor::HandleLinkHereClicked)
        .OnMoveCommitted(this, &SDialogueTreeEditor::HandleNodeMoveCommitted);
}

TSharedRef<SWidget> SDialogueTreeEditor::BuildEdgeList()
{
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    if (!DialogueTree.IsValid() || !SelectedNodeGuid.IsValid())
    {
        List->AddSlot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("NoSelectedNode", "Select a node to inspect outgoing edges."))
            .AutoWrapText(true)
        ];
        return List;
    }

    TArray<FDialogueEdge> OutgoingEdges;
    DialogueTree->GetOutgoingEdges(SelectedNodeGuid, OutgoingEdges);
    if (OutgoingEdges.Num() == 0)
    {
        List->AddSlot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("NoOutgoingEdges", "No outgoing edges."))
            .AutoWrapText(true)
        ];
        return List;
    }

    for (const FDialogueEdge& Edge : OutgoingEdges)
    {
        const FString EdgeSummary = FString::Printf(
            TEXT("Slot %d -> %s%s"),
            Edge.FromSlotIndex,
            *GetShortGuid(Edge.ToNodeGuid),
            Edge.ConditionExpression.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("\nif %s"), *Edge.ConditionExpression));

        List->AddSlot()
        .AutoHeight()
        .Padding(0.0f, 2.0f)
        [
            SNew(SBorder)
            .Padding(6.0f)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(EdgeSummary))
                    .AutoWrapText(true)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(6.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("DeleteEdge", "Delete"))
                    .OnClicked(this, &SDialogueTreeEditor::HandleDeleteEdgeClicked, Edge.FromNodeGuid, Edge.ToNodeGuid, Edge.FromSlotIndex, Edge.ConditionExpression)
                ]
            ]
        ];
    }

    return List;
}

TArray<FDialogueEdge> SDialogueTreeEditor::GetVisibleEdges(const TSet<FGuid>& VisibleSet) const
{
    TArray<FDialogueEdge> VisibleEdges;
    if (!DialogueTree.IsValid())
    {
        return VisibleEdges;
    }

    for (const FDialogueEdge& Edge : DialogueTree->Edges)
    {
        if (VisibleSet.Contains(Edge.FromNodeGuid) && VisibleSet.Contains(Edge.ToNodeGuid))
        {
            VisibleEdges.Add(Edge);
        }
    }

    return VisibleEdges;
}

FLinearColor SDialogueTreeEditor::GetNodeColor(EDialogueNodeType NodeType) const
{
    switch (NodeType)
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

FString SDialogueTreeEditor::GetNodeSummary(const FDialogueNode& Node) const
{
    if (Node.NodeType == EDialogueNodeType::Speech)
    {
        const FString Text = Node.Lines.Num() > 0 ? Node.Lines[0].Text.ToString() : TEXT("No lines");
        return FString::Printf(TEXT("%d line(s)\n%s"), Node.Lines.Num(), *Text.Left(72));
    }

    if (Node.NodeType == EDialogueNodeType::Choice)
    {
        const FString Text = Node.Choices.Num() > 0 ? Node.Choices[0].Text.ToString() : TEXT("No choices");
        return FString::Printf(TEXT("%d choice(s)\n%s"), Node.Choices.Num(), *Text.Left(72));
    }

    if (Node.NodeType == EDialogueNodeType::Condition)
    {
        return Node.ConditionExpression.IsEmpty() ? TEXT("No expression") : Node.ConditionExpression.Left(92);
    }

    if (Node.NodeType == EDialogueNodeType::Event)
    {
        return Node.EventTag.IsValid() ? Node.EventTag.ToString() : TEXT("No event tag");
    }

    return FString::Printf(TEXT("Outgoing slot default: %d"), Node.DefaultOutputIndex);
}

FString SDialogueTreeEditor::GetShortGuid(const FGuid& Guid) const
{
    return Guid.ToString(EGuidFormats::Digits).Left(8);
}

#undef LOCTEXT_NAMESPACE
