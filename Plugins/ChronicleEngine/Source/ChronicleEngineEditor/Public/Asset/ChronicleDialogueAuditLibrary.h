#pragma once

#include "CoreMinimal.h"
#include "Asset/ChronicleDialogueJsonLibrary.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChronicleDialogueAuditLibrary.generated.h"

class UDialogueTree;

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueSpeakerLineStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    FGameplayTag SpeakerTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 LineCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 WordCount = 0;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueVariableUsage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    FString VariableName;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    FGameplayTag VariableTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 ConditionUsageCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 EventPayloadUsageCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    TArray<FGuid> NodeGuids;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINEEDITOR_API FChronicleDialogueAuditReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 NodeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 EdgeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 SpeechLineCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 ChoiceCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 WordCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 BrokenEdgeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 UnreachableNodeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    TArray<FChronicleDialogueSpeakerLineStats> SpeakerLineStats;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    TArray<FChronicleDialogueVariableUsage> VariableUsages;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Audit")
    TArray<FChronicleDialogueValidationIssue> Issues;
};

UCLASS()
class CHRONICLEENGINEEDITOR_API UChronicleDialogueAuditLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Audit")
    static bool BuildDialogueAuditReport(UDialogueTree* Tree, FChronicleDialogueAuditReport& OutReport, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Audit")
    static bool ExportDialogueAuditReportToJsonString(const FChronicleDialogueAuditReport& Report, FString& OutJson, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Editor|Audit")
    static bool ExportDialogueAuditReportForTreeToJsonString(UDialogueTree* Tree, FString& OutJson, FString& OutError);
};
