# Chronicle Presentation Workflow

M4 added the source-first presentation controller. The v0.12 Phase 3 pass adds cue routing and demo HUD bootstrapping while keeping the workflow source-first and friendly to source control.

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
- `UChronicleDialogueDefaultWidget`
  - Concrete source-built dialogue HUD.
  - Can auto-build a default layout at runtime.
  - Tracks speaker text, typewriter reveal state, choices, local backlog, Auto, Skip, and Backlog visibility.
  - Provides portrait and full-body image slots for project-specific speaker art.
- `UChronicleDialogueChoiceButton`
  - Indexed UMG button used by the default HUD for choice forwarding.
- `UChronicleDialogueCueRouter`
  - Routes presentation events into camera/audio cue structs.
  - Turns line `VoiceID` values into source-first audio cues.
  - Can be used directly from Blueprint or C++ without owning a level actor.
- `AChronicleDialogueCueDirector`
  - Level Actor wrapper around the cue router.
  - Can auto-bind to the Chronicle subsystem on BeginPlay.
  - Can register named camera actors and call `SetViewTargetWithBlend` for matching camera cues.
- `AChronicleDialogueDemoActor`
  - Builds a small runtime demo tree in source.
  - Demonstrates a camera cue, voiced lines, choices, presentation-controller startup, and default HUD creation without requiring binary sample assets.

## Subsystem Entry

Blueprint and C++ callers can keep using the game instance subsystem:

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

`GetPresentationController()` returns a controller bound to the current runner.

## UI Flow

### Default Widget

1. Create a Widget Blueprint derived from `UChronicleDialogueDefaultWidget`, or instantiate the C++ class directly.
2. Bind it to `GetPresentationController()`.
3. Optionally turn off `bBuildDefaultLayout` and bind your own named widgets to the optional fields.

The built-in layout includes speaker text, dialogue text, choice list, Backlog scroll box, Advance, Auto, Skip, Backlog, and Back buttons.

### Custom Widget

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

Voice playback can also use `FDialogueLine::VoiceID` from the line-started event. `UChronicleDialogueCueRouter` emits that as an `FChronicleAudioCue`, so Blueprint audio systems can bind to one cue stream. Camera and audio payload keys are project-defined; the demo actor uses `Shot` and `BlendTime`.

To use a source-first level camera director:

1. Place `AChronicleDialogueCueDirector` in the level.
2. Leave `bAutoBindToSubsystemOnBeginPlay` enabled.
3. Register camera actors by `ShotName`.
4. Emit a camera node or event with `Payload["Shot"] = "ShotName"`.

## Automated Coverage

M4 automation covers:

- Presentation backlog creation.
- Auto advance from one line to the next.
- Rollback syncing the presentation backlog to runner history.
- Skip mode fast-forwarding to choices.
- Choice forwarding through the presentation controller.
- Camera cue and payload preservation.
- Voice ID preservation on presented lines.
- Cue router conversion of camera events and line `VoiceID` values.
- Demo actor default HUD class bootstrapping.
- Default widget state for typewriter reveal, local backlog, choice forwarding, Auto, and Skip controls.
