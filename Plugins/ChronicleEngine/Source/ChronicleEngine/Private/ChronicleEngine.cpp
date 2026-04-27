#include "ChronicleEngine.h"

#include "Modules/ModuleManager.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Test, "Chronicle.Event.Test");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Async, "Chronicle.Event.Async");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Camera_Cut, "Chronicle.Camera.Cut");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Camera_Blend, "Chronicle.Camera.Blend");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Animation_Play, "Chronicle.Animation.Play");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Audio_PlayVoice, "Chronicle.Audio.PlayVoice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Audio_StopVoice, "Chronicle.Audio.StopVoice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Speaker_Alice, "Chronicle.Speaker.Alice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Variable_Flag, "Chronicle.Variable.Flag");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Variable_Name, "Chronicle.Variable.Name");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Variable_Score, "Chronicle.Variable.Score");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Variable_Test, "Chronicle.Variable.Test");

IMPLEMENT_MODULE(FChronicleEngineModule, ChronicleEngine)

void FChronicleEngineModule::StartupModule()
{
}

void FChronicleEngineModule::ShutdownModule()
{
}
