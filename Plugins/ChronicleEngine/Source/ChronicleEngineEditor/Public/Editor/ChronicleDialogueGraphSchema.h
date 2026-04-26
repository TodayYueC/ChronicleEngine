#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "EdGraph/EdGraphSchema.h"
#include "ChronicleDialogueGraphSchema.generated.h"

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueGraphSchema : public UEdGraphSchema
{
    GENERATED_BODY()

public:
    static void GetSupportedContextNodeTypes(TArray<EDialogueNodeType>& OutNodeTypes);

    virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
    virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
};
