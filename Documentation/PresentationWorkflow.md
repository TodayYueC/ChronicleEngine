# Chronicle Presentation Workflow

M4 adds a source-first presentation layer for UI, camera, voice, skip, auto, backlog, and rollback flows.

## Core Classes

- `UChronicleDialoguePresentationController`
  - Binds to a `UDialogueRunner`.
  - Re-broadcasts runner events as presentation-focused delegates.
  - Keeps UI-facing state: last line, last event, visible choices, backlog, auto mode, skip mode, and last reveal mode.
  - Drives `Advance`, `SelectChoice`, `NotifyEventComplete`, and `RequestRollback`.
- `UChronicleDialogueWidget`
  - Abstract `UUserWidget` base class.
  - Implements `IDialoguePresenter`.
  - Exposes Blueprint events for dialogue started/ended, line started/completed, choices, waiting-for-input, rollback, and presentation events.
  - Can drive `TickPresentation` for auto and skip behavior.
- `AChronicleDialogueDemoActor`
  - Builds a small runtime demo tree in source.
  - Demonstrates a camera cue, voiced lines, choices, and presentation-controller startup without requiring binary sample assets.

## Subsystem Entry

Blueprint and C++ callers can keep using the game instance subsystem:

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

`GetPresentationController()` returns a controller bound to the current runner.

## UI Flow

1. Create a Blueprint widget derived from `UChronicleDialogueWidget`.
2. Bind it to `GetPresentationController()`.
3. Implement the Blueprint events:
   - `On Line Started`
   - `On Choices Presented`
   - `On Waiting For Input`
   - `On Rollback`
   - `On Dialogue Event`
4. Wire UI buttons to:
   - `AdvanceDialogue`
   - `SelectDialogueChoice`
   - `SetAutoAdvanceEnabled`
   - `SetSkipModeEnabled`
   - `RequestRollback`

## Skip, Auto, Backlog, Rollback

- Auto mode advances only while the runner is `WaitingForInput`; it does not auto-select choices.
- Skip mode drains `WaitingForInput` lines until a choice, event wait, dialogue end, or `MaxSkipStepsPerTick`.
- Lines shown during skip are emitted with `ETextRevealMode::Instant`.
- Backlog is presentation-facing and syncs from runner save data after rollback.
- Rollback calls `UDialogueRunner::PerformRollback` and then re-broadcasts the restored backlog snapshot.

## Camera And Voice Cues

Presentation cues use Gameplay Tags and string payloads so projects can bind their own camera and audio systems.

Default cue tags added in M4:

- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

Voice playback can also use `FDialogueLine::VoiceID` from the line-started event. Camera and audio payload keys are project-defined; the demo actor uses `Shot` and `BlendTime`.

## Automated Coverage

M4 automation covers:

- Presentation backlog creation.
- Auto advance from one line to the next.
- Rollback syncing the presentation backlog to runner history.
- Skip mode fast-forwarding to choices.
- Choice forwarding through the presentation controller.
- Camera cue and payload preservation.
- Voice ID preservation on presented lines.
