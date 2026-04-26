#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SpeakerProfile.generated.h"

class UDataTable;
class UTexture2D;

UENUM(BlueprintType)
enum class ESpeakerPosition : uint8
{
    Left,
    Center,
    Right,
    OffScreen
};

UCLASS(BlueprintType)
class CHRONICLEENGINE_API USpeakerProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    FGameplayTag SpeakerTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    TMap<FName, TSoftObjectPtr<UTexture2D>> PortraitSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    TMap<FName, TSoftObjectPtr<UTexture2D>> FullBodySet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    TSoftObjectPtr<UDataTable> VoiceSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    FLinearColor TextColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chronicle|Speaker")
    ESpeakerPosition DefaultPosition = ESpeakerPosition::Left;
};
