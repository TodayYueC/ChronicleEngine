#include "Presentation/ChronicleDialogueCueDirector.h"

#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagsManager.h"
#include "Presentation/ChronicleDialoguePresentationController.h"
#include "Runtime/ChronicleDialogueSubsystem.h"

void UChronicleDialogueCueRouter::BindPresentationController(UChronicleDialoguePresentationController* InController)
{
    if (PresentationController == InController)
    {
        return;
    }

    UnbindPresentationController();
    PresentationController = InController;

    if (!PresentationController)
    {
        return;
    }

    PresentationController->OnPresentationEvent.AddDynamic(this, &UChronicleDialogueCueRouter::HandlePresentationEvent);
    PresentationController->OnPresentationLineStarted.AddDynamic(this, &UChronicleDialogueCueRouter::HandleLineStarted);
}

void UChronicleDialogueCueRouter::UnbindPresentationController()
{
    if (!PresentationController)
    {
        return;
    }

    PresentationController->OnPresentationEvent.RemoveAll(this);
    PresentationController->OnPresentationLineStarted.RemoveAll(this);
    PresentationController = nullptr;
}

void UChronicleDialogueCueRouter::RegisterCameraShot(FName ShotName, AActor* CameraActor)
{
    if (!ShotName.IsNone() && CameraActor)
    {
        CameraShots.Add(ShotName, CameraActor);
    }
}

void UChronicleDialogueCueRouter::UnregisterCameraShot(FName ShotName)
{
    CameraShots.Remove(ShotName);
}

void UChronicleDialogueCueRouter::ClearCameraShots()
{
    CameraShots.Reset();
}

void UChronicleDialogueCueRouter::HandlePresentationEvent(const FDialogueEventData& EventData)
{
    if (IsCameraCueTag(EventData.EventTag))
    {
        FChronicleCameraCue Cue;
        Cue.EventTag = EventData.EventTag;
        Cue.Payload = EventData.Payload;
        Cue.ShotName = FName(*GetPayloadValue(EventData.Payload, FName(TEXT("Shot"))));
        if (Cue.ShotName.IsNone())
        {
            Cue.ShotName = FName(*GetPayloadValue(EventData.Payload, FName(TEXT("CameraShotName"))));
        }
        Cue.BlendTime = GetPayloadFloat(EventData.Payload, FName(TEXT("BlendTime")), DefaultBlendTime);
        if (TObjectPtr<AActor>* CameraActor = CameraShots.Find(Cue.ShotName))
        {
            Cue.CameraActor = CameraActor->Get();
        }

        LastCameraCue = Cue;
        ++CameraCueCount;
        OnCameraCue.Broadcast(LastCameraCue);
        return;
    }

    if (IsAudioCueTag(EventData.EventTag))
    {
        FChronicleAudioCue Cue;
        Cue.EventTag = EventData.EventTag;
        Cue.Payload = EventData.Payload;
        Cue.CueName = FName(*GetPayloadValue(EventData.Payload, FName(TEXT("Cue"))));
        if (Cue.CueName.IsNone())
        {
            Cue.CueName = FName(*GetPayloadValue(EventData.Payload, FName(TEXT("VoiceID"))));
        }
        Cue.LineID = FName(*GetPayloadValue(EventData.Payload, FName(TEXT("LineID"))));

        const FString SpeakerTagName = GetPayloadValue(EventData.Payload, FName(TEXT("SpeakerTag")));
        if (!SpeakerTagName.IsEmpty())
        {
            Cue.SpeakerTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(*SpeakerTagName), false);
        }

        LastAudioCue = Cue;
        ++AudioCueCount;
        OnAudioCue.Broadcast(LastAudioCue);
    }
}

void UChronicleDialogueCueRouter::HandleLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode)
{
    if (!bBroadcastLineVoiceCues || Line.VoiceID.IsNone())
    {
        return;
    }

    FChronicleAudioCue Cue;
    Cue.EventTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TEXT("Chronicle.Audio.PlayVoice")), false);
    Cue.CueName = Line.VoiceID;
    Cue.LineID = Line.LineID;
    Cue.SpeakerTag = Line.SpeakerTag;
    Cue.bFromDialogueLine = true;
    Cue.Payload.Add(TEXT("VoiceID"), Line.VoiceID.ToString());
    Cue.Payload.Add(TEXT("LineID"), Line.LineID.ToString());
    Cue.Payload.Add(TEXT("SpeakerTag"), Line.SpeakerTag.ToString());

    LastAudioCue = Cue;
    ++AudioCueCount;
    OnAudioCue.Broadcast(LastAudioCue);
}

bool UChronicleDialogueCueRouter::IsCameraCueTag(FGameplayTag EventTag)
{
    return EventTag.IsValid() && EventTag.ToString().StartsWith(TEXT("Chronicle.Camera."));
}

bool UChronicleDialogueCueRouter::IsAudioCueTag(FGameplayTag EventTag)
{
    return EventTag.IsValid() && EventTag.ToString().StartsWith(TEXT("Chronicle.Audio."));
}

FString UChronicleDialogueCueRouter::GetPayloadValue(const TMap<FName, FString>& Payload, FName Key)
{
    if (const FString* Value = Payload.Find(Key))
    {
        return *Value;
    }
    return FString();
}

float UChronicleDialogueCueRouter::GetPayloadFloat(const TMap<FName, FString>& Payload, FName Key, float DefaultValue)
{
    const FString Value = GetPayloadValue(Payload, Key);
    if (Value.IsEmpty())
    {
        return DefaultValue;
    }
    return FCString::Atof(*Value);
}

AChronicleDialogueCueDirector::AChronicleDialogueCueDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AChronicleDialogueCueDirector::BeginPlay()
{
    Super::BeginPlay();

    if (!CueRouter)
    {
        CueRouter = NewObject<UChronicleDialogueCueRouter>(this, TEXT("ChronicleCueRouter"));
        CueRouter->OnCameraCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterCameraCue);
        CueRouter->OnAudioCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterAudioCue);
    }

    CueRouter->bBroadcastLineVoiceCues = bBroadcastLineVoiceCues;
    CueRouter->DefaultBlendTime = DefaultBlendTime;

    if (bAutoBindToSubsystemOnBeginPlay)
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UChronicleDialogueSubsystem* DialogueSubsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>())
            {
                BindPresentationController(DialogueSubsystem->GetPresentationController());
            }
        }
    }
}

void AChronicleDialogueCueDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindPresentationController();
    Super::EndPlay(EndPlayReason);
}

void AChronicleDialogueCueDirector::BindPresentationController(UChronicleDialoguePresentationController* InController)
{
    if (!CueRouter)
    {
        CueRouter = NewObject<UChronicleDialogueCueRouter>(this, TEXT("ChronicleCueRouter"));
        CueRouter->OnCameraCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterCameraCue);
        CueRouter->OnAudioCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterAudioCue);
    }

    CueRouter->bBroadcastLineVoiceCues = bBroadcastLineVoiceCues;
    CueRouter->DefaultBlendTime = DefaultBlendTime;
    CueRouter->BindPresentationController(InController);
}

void AChronicleDialogueCueDirector::UnbindPresentationController()
{
    if (CueRouter)
    {
        CueRouter->UnbindPresentationController();
    }
}

void AChronicleDialogueCueDirector::RegisterCameraShot(FName ShotName, AActor* CameraActor)
{
    if (!CueRouter)
    {
        CueRouter = NewObject<UChronicleDialogueCueRouter>(this, TEXT("ChronicleCueRouter"));
        CueRouter->OnCameraCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterCameraCue);
        CueRouter->OnAudioCue.AddUniqueDynamic(this, &AChronicleDialogueCueDirector::HandleRouterAudioCue);
    }

    CueRouter->RegisterCameraShot(ShotName, CameraActor);
}

void AChronicleDialogueCueDirector::HandleRouterCameraCue(const FChronicleCameraCue& Cue)
{
    if (bApplyCameraViewTargets && Cue.CameraActor)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PlayerController = World->GetFirstPlayerController())
            {
                PlayerController->SetViewTargetWithBlend(Cue.CameraActor, Cue.BlendTime);
            }
        }
    }

    OnCameraCue.Broadcast(Cue);
}

void AChronicleDialogueCueDirector::HandleRouterAudioCue(const FChronicleAudioCue& Cue)
{
    OnAudioCue.Broadcast(Cue);
}
