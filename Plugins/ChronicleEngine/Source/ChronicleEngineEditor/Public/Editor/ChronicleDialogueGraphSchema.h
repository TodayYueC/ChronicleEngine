#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "ChronicleDialogueGraphSchema.generated.h"

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueGraphSchema : public UEdGraphSchema
{
    GENERATED_BODY()

public:
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
    virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
};
