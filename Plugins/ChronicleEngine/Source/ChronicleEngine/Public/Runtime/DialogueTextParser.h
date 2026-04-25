#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogueTextParser.generated.h"

UCLASS()
class CHRONICLEENGINE_API UDialogueTextParser : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Text")
    static void ParseInlineTags(const FText& SourceText, TArray<FLineSegment>& OutSegments);
};

