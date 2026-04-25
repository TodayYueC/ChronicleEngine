#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueJsonLibrary.generated.h"

class UDialogueTree;

UENUM(BlueprintType)
enum class EChronicleDialogueValidationSeverity : uint8
{
    Info,
    Warning,
    Error
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Validation")
    EChronicleDialogueValidationSeverity Severity = EChronicleDialogueValidationSeverity::Error;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Validation")
    FGuid NodeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Validation")
    FString Message;
};

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueJsonLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ExportDialogueTreeToJsonString(UDialogueTree* Tree, FString& OutJson, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ExportDialogueTreeToJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ImportDialogueTreeFromJsonString(UDialogueTree* Tree, const FString& Json, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|JSON")
    static bool ImportDialogueTreeFromJsonFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|CSV")
    static bool ExportDialogueLinesToCsvString(UDialogueTree* Tree, FString& OutCsv, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|CSV")
    static bool ExportDialogueLinesToCsvFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|CSV")
    static bool ImportDialogueLinesFromCsvString(UDialogueTree* Tree, const FString& Csv, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|CSV")
    static bool ImportDialogueLinesFromCsvFile(UDialogueTree* Tree, const FString& FilePath, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Validation")
    static bool ValidateDialogueTree(UDialogueTree* Tree, TArray<FChronicleDialogueValidationIssue>& OutIssues);
};
