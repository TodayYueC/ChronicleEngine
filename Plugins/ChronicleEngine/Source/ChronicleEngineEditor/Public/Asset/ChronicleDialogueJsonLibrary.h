#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueJsonLibrary.generated.h"

class UDialogueTree;
class UDialogueDatabase;

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueLocalizationEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FString Namespace;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FName Key;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FString Culture;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FGuid TreeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FGuid NodeGuid;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    int32 LineIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FName LineID;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FGameplayTag SpeakerTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FText SourceText;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FText TranslatedText;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Localization")
    FString ContextComment;
};

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

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Localization")
    static bool EnsureStableLineIds(UDialogueTree* Tree, int32& OutUpdatedCount, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Localization")
    static bool GatherDialogueTextsFromTree(UDialogueTree* Tree, const FString& Namespace, const FString& Culture, TArray<FChronicleDialogueLocalizationEntry>& OutEntries, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Localization")
    static bool GatherDialogueTextsFromDatabase(UDialogueDatabase* Database, const FString& Culture, TArray<FChronicleDialogueLocalizationEntry>& OutEntries, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Localization")
    static bool ExportLocalizationCsvFromTree(UDialogueTree* Tree, const FString& Namespace, const FString& Culture, FString& OutCsv, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Localization")
    static bool ImportLocalizationCsvToTree(UDialogueTree* Tree, const FString& Csv, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Validation")
    static bool ValidateDialogueTree(UDialogueTree* Tree, TArray<FChronicleDialogueValidationIssue>& OutIssues);
};
