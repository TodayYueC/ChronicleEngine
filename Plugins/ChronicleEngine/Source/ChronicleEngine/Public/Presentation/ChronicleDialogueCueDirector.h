#pragma once

#include "CoreMinimal.h"
#include "Core/ChronicleTypes.h"
#include "GameFramework/Actor.h"
#include "ChronicleDialogueCueDirector.generated.h"

class UChronicleDialoguePresentationController;
class UDialogueTree;

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FChronicleCameraCue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FGameplayTag EventTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FName ShotName;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    float BlendTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    TMap<FName, FString> Payload;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    TObjectPtr<AActor> CameraActor = nullptr;
};

USTRUCT(BlueprintType)
struct CHRONICLEENGINE_API FChronicleAudioCue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FGameplayTag EventTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FName CueName;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FName LineID;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    FGameplayTag SpeakerTag;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    bool bFromDialogueLine = false;

    UPROPERTY(BlueprintReadOnly, Category="Chronicle|Presentation")
    TMap<FName, FString> Payload;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleCameraCueRouted, const FChronicleCameraCue&, Cue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChronicleAudioCueRouted, const FChronicleAudioCue&, Cue);

UCLASS(BlueprintType)
class CHRONICLEENGINE_API UChronicleDialogueCueRouter : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void BindPresentationController(UChronicleDialoguePresentationController* InController);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void UnbindPresentationController();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void RegisterCameraShot(FName ShotName, AActor* CameraActor);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void UnregisterCameraShot(FName ShotName);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void ClearCameraShots();

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    UChronicleDialoguePresentationController* GetPresentationController() const { return PresentationController; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    FChronicleCameraCue GetLastCameraCue() const { return LastCameraCue; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    FChronicleAudioCue GetLastAudioCue() const { return LastAudioCue; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    int32 GetCameraCueCount() const { return CameraCueCount; }

    UFUNCTION(BlueprintPure, Category="Chronicle|Presentation")
    int32 GetAudioCueCount() const { return AudioCueCount; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    bool bBroadcastLineVoiceCues = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    float DefaultBlendTime = 0.25f;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChronicleCameraCueRouted OnCameraCue;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChronicleAudioCueRouted OnAudioCue;

private:
    UFUNCTION()
    void HandlePresentationEvent(const FDialogueEventData& EventData);

    UFUNCTION()
    void HandleLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode);

    static bool IsCameraCueTag(FGameplayTag EventTag);
    static bool IsAudioCueTag(FGameplayTag EventTag);
    static FString GetPayloadValue(const TMap<FName, FString>& Payload, FName Key);
    static float GetPayloadFloat(const TMap<FName, FString>& Payload, FName Key, float DefaultValue);

    UPROPERTY(Transient)
    TObjectPtr<UChronicleDialoguePresentationController> PresentationController;

    UPROPERTY(Transient)
    TMap<FName, TObjectPtr<AActor>> CameraShots;

    UPROPERTY(Transient)
    FChronicleCameraCue LastCameraCue;

    UPROPERTY(Transient)
    FChronicleAudioCue LastAudioCue;

    UPROPERTY(Transient)
    int32 CameraCueCount = 0;

    UPROPERTY(Transient)
    int32 AudioCueCount = 0;
};

UCLASS(Blueprintable)
class CHRONICLEENGINE_API AChronicleDialogueCueDirector : public AActor
{
    GENERATED_BODY()

public:
    AChronicleDialogueCueDirector();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void BindPresentationController(UChronicleDialoguePresentationController* InController);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void UnbindPresentationController();

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    void RegisterCameraShot(FName ShotName, AActor* CameraActor);

    UFUNCTION(BlueprintCallable, Category="Chronicle|Presentation")
    UChronicleDialogueCueRouter* GetCueRouter() const { return CueRouter; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    bool bAutoBindToSubsystemOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    bool bApplyCameraViewTargets = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    bool bBroadcastLineVoiceCues = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronicle|Presentation")
    float DefaultBlendTime = 0.25f;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChronicleCameraCueRouted OnCameraCue;

    UPROPERTY(BlueprintAssignable, Category="Chronicle|Presentation")
    FOnChronicleAudioCueRouted OnAudioCue;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION()
    void HandleRouterCameraCue(const FChronicleCameraCue& Cue);

    UFUNCTION()
    void HandleRouterAudioCue(const FChronicleAudioCue& Cue);

    UPROPERTY(Transient)
    TObjectPtr<UChronicleDialogueCueRouter> CueRouter;
};
