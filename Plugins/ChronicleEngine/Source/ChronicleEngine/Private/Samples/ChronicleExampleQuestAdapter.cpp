#include "Samples/ChronicleExampleQuestAdapter.h"

#include "Runtime/DialogueRunner.h"

#include "GameplayTagsManager.h"

void UChronicleExampleQuestAdapter::BindToRunner(UDialogueRunner* Runner)
{
    if (BoundRunner == Runner)
    {
        return;
    }

    UnbindFromRunner();
    BoundRunner = Runner;
    if (BoundRunner)
    {
        BoundRunner->OnDialogueEvent.AddDynamic(this, &UChronicleExampleQuestAdapter::HandleDialogueEvent);
    }
}

void UChronicleExampleQuestAdapter::UnbindFromRunner()
{
    if (BoundRunner)
    {
        BoundRunner->OnDialogueEvent.RemoveDynamic(this, &UChronicleExampleQuestAdapter::HandleDialogueEvent);
        BoundRunner = nullptr;
    }
}

void UChronicleExampleQuestAdapter::PushExternalVariable(FGameplayTag VariableTag, const FVariableValue& Value, EChronicleVariableScope Scope)
{
    if (BoundRunner && VariableTag.IsValid())
    {
        BoundRunner->SetVariable(VariableTag, Value, Scope);
    }
}

void UChronicleExampleQuestAdapter::HandleDialogueEvent(const FDialogueEventData& EventData)
{
    ++HandledEventCount;
    LastEvent = EventData;

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Quest.Start")))
    {
        LastQuestTag = ResolvePayloadTag(EventData, TEXT("QuestTag"));
        OnQuestStarted.Broadcast(LastQuestTag, EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Quest.Update")))
    {
        LastQuestTag = ResolvePayloadTag(EventData, TEXT("QuestTag"));
        OnQuestUpdated.Broadcast(LastQuestTag, EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Quest.Complete")))
    {
        LastQuestTag = ResolvePayloadTag(EventData, TEXT("QuestTag"));
        OnQuestCompleted.Broadcast(LastQuestTag, EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.GameState.Change")))
    {
        LastStateName = ResolvePayloadName(EventData, TEXT("State"));
        OnGameStateChanged.Broadcast(LastStateName, EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Actor.Animate")))
    {
        OnActorAnimationRequested.Broadcast(EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Battle.Encounter")))
    {
        OnBattleEncounterRequested.Broadcast(EventData);
        return;
    }

    if (MatchesEventTag(EventData, TEXT("Chronicle.Event.Scene.Load")))
    {
        OnSceneLoadRequested.Broadcast(EventData);
        return;
    }

    OnUnhandledDialogueEvent.Broadcast(EventData);
}

FGameplayTag UChronicleExampleQuestAdapter::ResolvePayloadTag(const FDialogueEventData& EventData, FName Key) const
{
    const FString* Value = EventData.Payload.Find(Key);
    if (!Value || Value->TrimStartAndEnd().IsEmpty())
    {
        return FGameplayTag();
    }

    return UGameplayTagsManager::Get().RequestGameplayTag(FName(*(*Value)), false);
}

FName UChronicleExampleQuestAdapter::ResolvePayloadName(const FDialogueEventData& EventData, FName Key) const
{
    const FString* Value = EventData.Payload.Find(Key);
    return Value ? FName(*(*Value)) : NAME_None;
}

bool UChronicleExampleQuestAdapter::MatchesEventTag(const FDialogueEventData& EventData, const TCHAR* TagName) const
{
    const FGameplayTag ExpectedTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TagName), false);
    return ExpectedTag.IsValid() && EventData.EventTag.MatchesTagExact(ExpectedTag);
}
