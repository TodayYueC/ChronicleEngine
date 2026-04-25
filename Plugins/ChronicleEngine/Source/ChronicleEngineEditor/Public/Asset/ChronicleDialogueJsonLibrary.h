#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueJsonLibrary.generated.h"

class UDialogueTree;

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueJsonLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ExportDialogueTreeToJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ImportDialogueTreeFromJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);
};

