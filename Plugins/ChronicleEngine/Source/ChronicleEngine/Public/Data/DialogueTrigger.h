#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "Engine/DataAsset.h"
#include "DialogueTrigger.generated.h"

class UDialogueTree;

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UDialogueTrigger : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    FGameplayTag TriggerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    TSoftObjectPtr<UDialogueTree> TargetTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    FName EntryNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    EDialogueTriggerType TriggerType = EDialogueTriggerType::Interact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    TArray<FString> ActivationConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger", meta=(ClampMin="0.0"))
    float CooldownTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    bool bOneShot = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Trigger")
    int32 Priority = 0;
};
