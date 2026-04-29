#include "ChronicleEngine.h"

#include "Modules/ModuleManager.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Test, "Chronicle.Event.Test");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Async, "Chronicle.Event.Async");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Quest_Start, "Chronicle.Event.Quest.Start");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Quest_Update, "Chronicle.Event.Quest.Update");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Quest_Complete, "Chronicle.Event.Quest.Complete");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_GameState_Change, "Chronicle.Event.GameState.Change");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Actor_Animate, "Chronicle.Event.Actor.Animate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Battle_Encounter, "Chronicle.Event.Battle.Encounter");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Event_Scene_Load, "Chronicle.Event.Scene.Load");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Camera_Cut, "Chronicle.Camera.Cut");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Camera_Blend, "Chronicle.Camera.Blend");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Animation_Play, "Chronicle.Animation.Play");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Audio_PlayVoice, "Chronicle.Audio.PlayVoice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Audio_StopVoice, "Chronicle.Audio.StopVoice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Quest_Main, "Chronicle.Quest.Main");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Speaker_Alice, "Chronicle.Speaker.Alice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Trigger_Cooldown, "Chronicle.Trigger.Cooldown");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Trigger_High, "Chronicle.Trigger.High");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Trigger_Low, "Chronicle.Trigger.Low");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Chronicle_Trigger_Test, "Chronicle.Trigger.Test");
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
