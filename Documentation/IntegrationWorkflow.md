# Chronicle Integration Workflow

The v0.9 pass adds a source-first example for connecting dialogue events to project systems such as quests, game state, battle flow, scene loading, and actor animation.

## Standard Event Tags

The built-in tag list now includes:

- `Chronicle.Event.Quest.Start`
- `Chronicle.Event.Quest.Update`
- `Chronicle.Event.Quest.Complete`
- `Chronicle.Event.GameState.Change`
- `Chronicle.Event.Actor.Animate`
- `Chronicle.Event.Battle.Encounter`
- `Chronicle.Event.Scene.Load`

These tags are intentionally generic. Projects can either use them directly or treat them as examples for their own `Chronicle.Event.*` hierarchy.

## Example Quest Adapter

`UChronicleExampleQuestAdapter` lives in the Runtime module under `Samples`.

It binds to:

```cpp
UDialogueRunner::OnDialogueEvent
```

and exposes:

- `OnQuestStarted`
- `OnQuestUpdated`
- `OnQuestCompleted`
- `OnGameStateChanged`
- `OnActorAnimationRequested`
- `OnBattleEncounterRequested`
- `OnSceneLoadRequested`
- `OnUnhandledDialogueEvent`

Basic setup:

```cpp
UChronicleExampleQuestAdapter* Adapter = NewObject<UChronicleExampleQuestAdapter>();
Adapter->BindToRunner(Runner);
```

Quest events expect a `QuestTag` payload:

```text
QuestTag=Chronicle.Quest.Main
```

Game-state events expect a `State` payload:

```text
State=Exploration
```

Other payload keys are project-defined. The adapter keeps the original `FDialogueEventData` intact so downstream systems can read their own data.

## Variable Push

Project systems can feed runner variables through the adapter:

```cpp
Adapter->PushExternalVariable(ScoreTag, FVariableValue::MakeInt(12));
```

The default scope is `Global`. Pass `Local` when the value should reset with the current dialogue tree. Pass `External` only when the variable has an external getter/setter binding in `UVariableBank`.

## Async Events

If an event node has `bEventIsAsync = true`, the runner waits until the project system calls:

```cpp
Runner->NotifyEventComplete(EventTag);
```

Use this for scene loading, battle encounters, timeline playback, or any action that must finish before dialogue continues.
