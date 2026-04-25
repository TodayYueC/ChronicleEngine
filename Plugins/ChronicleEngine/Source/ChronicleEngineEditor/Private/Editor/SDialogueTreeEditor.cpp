#include "Editor/SDialogueTreeEditor.h"

#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "Data/DialogueTree.h"
#include "Editor/ChronicleDialogueEditorLibrary.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SDialogueTreeEditor"

void SDialogueTreeEditor::Construct(const FArguments& InArgs)
{
    DialogueTree = InArgs._DialogueTree;
    SearchQuery.Reset();

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
                    SNew(SScrollBox)
                    .Orientation(Orient_Horizontal)

                    + SScrollBox::Slot()
                    [
                        SNew(SScrollBox)
                        .Orientation(Orient_Vertical)

                        + SScrollBox::Slot()
                        [
                            SAssignNew(CanvasHost, SBox)
                        ]
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
    RefreshValidationSummary();
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

    RefreshCanvas();
    RefreshValidationSummary();
    return FReply::Handled();
}

FReply SDialogueTreeEditor::HandleValidateClicked()
{
    RefreshValidationSummary();
    return FReply::Handled();
}

void SDialogueTreeEditor::HandleSearchTextChanged(const FText& InText)
{
    SearchQuery = InText.ToString();
    RefreshCanvas();
}

void SDialogueTreeEditor::RefreshCanvas()
{
    if (!CanvasHost.IsValid())
    {
        return;
    }

    CanvasHost->SetContent(BuildCanvas());
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
    TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
    VisibleNodeGuids.Reset();

    if (!DialogueTree.IsValid())
    {
        return SNew(SBox)
            .WidthOverride(1600.0f)
            .HeightOverride(1000.0f)
            [
                Canvas
            ];
    }

    UChronicleDialogueEditorLibrary::SearchDialogueNodes(DialogueTree.Get(), SearchQuery, VisibleNodeGuids);
    TSet<FGuid> VisibleSet(VisibleNodeGuids);

    for (const FDialogueNode& Node : DialogueTree->Nodes)
    {
        if (!VisibleSet.Contains(Node.NodeGuid))
        {
            continue;
        }

        const FVector2D Position = Node.Position + FVector2D(800.0f, 360.0f);
        Canvas->AddSlot()
        .Offset(FMargin(Position.X, Position.Y, 260.0f, 142.0f))
        [
            BuildNodeCard(Node)
        ];
    }

    return SNew(SBox)
        .WidthOverride(2400.0f)
        .HeightOverride(1500.0f)
        [
            Canvas
        ];
}

TSharedRef<SWidget> SDialogueTreeEditor::BuildNodeCard(const FDialogueNode& Node) const
{
    return SNew(SBorder)
        .Padding(10.0f)
        .BorderBackgroundColor(GetNodeColor(Node.NodeType))
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
                .Text(FText::FromString(GetNodeSummary(Node)))
                .AutoWrapText(true)
                .ColorAndOpacity(FLinearColor::White)
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 4.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(GetShortGuid(Node.NodeGuid)))
                .ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 0.8f))
            ]
        ];
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

