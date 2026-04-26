#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DialogueDatabase.generated.h"

class UDialogueTree;
class USpeakerProfile;

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FDialogueLocalizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Localization")
    FString Namespace = TEXT("Project.Dialogue");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Localization")
    TArray<FString> TargetCultures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Localization")
    FDirectoryPath VoiceTablePath;
};

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UDialogueDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database")
    TArray<TSoftObjectPtr<USpeakerProfile>> SpeakerProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database")
    TArray<FVariableDefinition> GlobalVariables;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database")
    TArray<TSoftObjectPtr<UDialogueTree>> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database")
    FDialogueLocalizationSettings LocalizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database")
    TSoftObjectPtr<UDataTable> VoiceTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Database|Editor")
    FChronicleSoftLockMetadata EditorLock;

    UFUNCTION(BlueprintCallable, Category="Chronicle|Database")
    bool FindVariableDefinition(FGameplayTag VariableTag, FVariableDefinition& OutDefinition) const;
};
