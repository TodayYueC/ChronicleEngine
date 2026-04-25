#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Widgets/SCompoundWidget.h"

class UDialogueTree;
class SBox;
class STextBlock;

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
    void HandleSearchTextChanged(const FText& InText);
    void RefreshCanvas();
    void RefreshValidationSummary();
    TSharedRef<SWidget> BuildCanvas();
    TSharedRef<SWidget> BuildNodeCard(const FDialogueNode& Node) const;
    FLinearColor GetNodeColor(EDialogueNodeType NodeType) const;
    FString GetNodeSummary(const FDialogueNode& Node) const;
    FString GetShortGuid(const FGuid& Guid) const;

    TWeakObjectPtr<UDialogueTree> DialogueTree;
    TSharedPtr<SBox> CanvasHost;
    TSharedPtr<STextBlock> ValidationText;
    FString SearchQuery;
    TArray<FGuid> VisibleNodeGuids;
};
