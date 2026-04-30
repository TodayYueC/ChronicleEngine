# Chronicle Engine

Chronicle Engine is an MIT-licensed Unreal Engine 5 source plugin for JRPG, visual-novel, AVG, and narrative RPG dialogue systems. It ships with a minimal host project so the plugin can be compiled, tested, packaged, and validated in isolation before being copied into a production game.

- Current development version: `v0.12.0-dev`
- Latest public packaged release: `v0.5.0`
- Latest locally validated package: `v0.12.0-dev`
- Primary engine baseline: UE 5.3
- Compatibility smoke target: UE 5.7
- Repository: `TodayYueC/ChronicleEngine`
- License: MIT
- PRD comparison: [Documentation/PRDCompletionMatrix.md](Documentation/PRDCompletionMatrix.md)

---

# English Manual

## 1. What Chronicle Engine Is

Chronicle Engine is designed around a simple separation:

- Data assets describe dialogue, speakers, variables, triggers, localization, and production metadata.
- Runtime systems traverse dialogue trees, evaluate variables and conditions, broadcast events, save state, and maintain rollback history.
- Presentation systems receive dialogue events and turn them into UI, camera, audio, backlog, auto, skip, and rollback behavior.
- Editor systems provide a native Unreal graph workflow, asset actions, validation, pipeline export, debugging metadata, and soft locks.

The plugin does not depend on a specific quest system, UI style, camera framework, audio middleware, or save-game architecture. It exposes GameplayTag events, Blueprint-callable APIs, source widgets, and C++ extension points so a project can integrate its own systems cleanly.

## 2. Current PRD Completion Summary

The source-plugin acceptance target is complete. The final detailed comparison is in [Documentation/PRDCompletionMatrix.md](Documentation/PRDCompletionMatrix.md).

| PRD Area | Current Status | Implementation Notes |
|---|---|---|
| Data layer | Complete | `UDialogueDatabase`, `UDialogueTree`, `USpeakerProfile`, `UDialogueTrigger`, and core structs are implemented. |
| Runtime layer | Complete | `UDialogueRunner`, `UVariableBank`, condition evaluation, events, save/load, rollback, random, jump, sub-dialogue, camera, and animation traversal are implemented. |
| Editor layer | Mostly complete | Native graph editor, Details helper, search, validation, breakpoints, snapshots, asset actions, and soft locks exist. Full live step debugger UI remains optional polish. |
| Pipeline layer | Complete | JSON, dialogue CSV, script CSV, localization CSV, validation, audit reports, and one-click pipeline export exist. Optional Articy/Twine/Ren'Py importers remain extension work. |
| Presentation layer | Complete for source-first scope | Presentation controller, source-built default widget, cue router, cue director, and demo actor exist. Binary `WBP_*` assets remain intentionally untracked. |
| Localization | Mostly complete | Stable keys, gather helpers, CSV import/export, and culture-specific voice table lookup exist. UE manifest/archive command wrapping remains optional. |
| CI/release | Complete | Local packaging script, full validation script, self-hosted GitHub Actions workflow, and release checklist exist. |

Latest verification:

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation tests: 35/35 passed.
- UE 5.3 100-node condition traversal: `0.0844ms`, meeting the PRD `<0.1ms` target.
- UE 5.7 editor build smoke: passed.
- UE 5.3 BuildPlugin package: passed.

## 3. Repository Layout

```text
ChronicleHost.uproject
Source/ChronicleHost/
Plugins/ChronicleEngine/
  ChronicleEngine.uplugin
  Config/
  Content/
  Source/
    ChronicleEngine/        Runtime module
    ChronicleEngineEditor/  Editor module
    ChronicleEngineTests/   Automation tests
Documentation/
Scripts/
.github/workflows/
```

Important files:

- `ChronicleHost.uproject`: minimal host project for compiling and testing.
- `Plugins/ChronicleEngine/ChronicleEngine.uplugin`: plugin descriptor.
- `Plugins/ChronicleEngine/Source/ChronicleEngine`: runtime, data, presentation, samples.
- `Plugins/ChronicleEngine/Source/ChronicleEngineEditor`: editor graph, factories, asset actions, import/export, audit.
- `Plugins/ChronicleEngine/Source/ChronicleEngineTests`: UE automation tests.
- `Config/DefaultGameplayTags.ini`: sample and standard Chronicle GameplayTags.
- `Scripts/PackagePlugin.ps1`: local package script.
- `Scripts/RunChronicleValidation.ps1`: build, test, package, and compatibility validation script.
- `.github/workflows/chronicle-validation.yml`: self-hosted Windows Unreal CI workflow.

Ignored local/generated content:

- `01_PRD_项目需求文档.md`
- `Artifacts/`
- `Binaries/`
- `Intermediate/`
- `Saved/`
- plugin `Binaries/` and `Intermediate/`

The PRD is intentionally not tracked in git.

## 4. Installation

### Option A: Use A Release Package

1. Download the release package from GitHub Releases.
2. Extract the archive.
3. Copy the plugin folder to your project:

```text
YourProject/Plugins/ChronicleEngine
```

4. Regenerate project files.
5. Open the Unreal project.
6. Enable `Chronicle Engine` in the Plugins panel if it is not already enabled.
7. Restart the editor.
8. Build the project for the engine version you are using.

Use this path when you want a stable packaged plugin.

### Option B: Use The Source Repository

1. Clone this repository.
2. Open `ChronicleHost.uproject` with UE 5.3.
3. Build `ChronicleHostEditor`.
4. Run the Chronicle automation suite.
5. Copy `Plugins/ChronicleEngine` into your own project when ready, or keep developing inside the host project.

Use this path when you want the newest `main` branch features.

### Option C: Add To An Existing C++ Project

1. Create `YourProject/Plugins/ChronicleEngine`.
2. Copy the packaged or source plugin into that folder.
3. Regenerate project files.
4. Build your project.
5. Confirm `Chronicle Engine` is enabled in the Plugins panel.
6. Copy or recreate the GameplayTags from `Config/DefaultGameplayTags.ini` if your project does not already define them.

### Option D: Add To A Blueprint-Only Project

Unreal still needs to compile source plugins. For a Blueprint-only project:

1. Add one empty C++ class from the Unreal Editor.
2. Close the editor.
3. Copy the plugin into `YourProject/Plugins/ChronicleEngine`.
4. Regenerate project files.
5. Build the project from your IDE or Unreal build tools.
6. Reopen the editor.

## 5. Build, Test, Package, And Validate

### Build With UE 5.3

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### Run Automation Tests

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

### Package The Plugin

```powershell
.\Scripts\PackagePlugin.ps1
```

Custom engine root and package name:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.12.0-UE5.7"
```

Output is written to `Artifacts/`, which is ignored by git.

### Run Full Local Validation

```powershell
.\Scripts\RunChronicleValidation.ps1
```

This script runs:

- UE 5.3 editor build.
- UE 5.3 Chronicle automation tests.
- UE 5.3 BuildPlugin package.
- UE 5.7 editor build smoke.
- UE 5.3 editor rebuild after the 5.7 smoke test, so local binaries return to the baseline engine.

Useful switches:

```powershell
.\Scripts\RunChronicleValidation.ps1 -SkipAutomation
.\Scripts\RunChronicleValidation.ps1 -SkipPackage
.\Scripts\RunChronicleValidation.ps1 -Skip57Smoke
```

## 6. Ten-Minute Quick Start

### Step 1: Open The Host Project

Open `ChronicleHost.uproject` in UE 5.3. If Unreal asks whether to rebuild modules, choose rebuild.

### Step 2: Create A Dialogue Tree

1. Open the Content Browser.
2. Right-click.
3. Create a `Dialogue Tree` asset.
4. Double-click it.
5. The Chronicle graph editor opens.

### Step 3: Build A Minimal Graph

1. Create a `Root` node.
2. Create a `Speech` node.
3. Connect `Root` to `Speech`.
4. Select the `Speech` node.
5. In Details, edit the first dialogue line text.
6. Save the asset.

### Step 4: Add A Choice

1. Create a `Choice` node.
2. Connect the `Speech` node to the `Choice` node.
3. Add two choices.
4. Create two more `Speech` nodes as results.
5. Connect choice slot `0` to the first result.
6. Connect choice slot `1` to the second result.
7. Save and validate.

### Step 5: Play It At Runtime

Use the subsystem:

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

For UI, bind a widget:

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
DefaultWidget->BindPresentationController(Presentation);
```

### Step 6: Try The Source Demo

1. Place `AChronicleDialogueDemoActor` in a level.
2. Keep `bStartOnBeginPlay` enabled.
3. Keep `bCreateDefaultWidget` enabled.
4. Run PIE.
5. The actor builds a demo tree, creates the default HUD, binds it, and starts dialogue.

## 7. Core Concepts

### Dialogue Database

`UDialogueDatabase` is an optional central registry for a project or chapter. It stores:

- speaker profiles
- global variable definitions
- dialogue tree references
- localization settings
- default voice table
- culture-specific voice tables
- editor lock metadata

Use it when the game needs a central narrative database, shared speaker lookup, global variables, or voice-table routing.

### Dialogue Tree

`UDialogueTree` is the main authored dialogue asset. It stores:

- `Nodes`: `FDialogueNode` array
- `Edges`: `FDialogueEdge` array
- `RootNodeGuid`
- local variable definitions
- editor state such as breakpoints and soft locks

Nodes are stored as structs instead of UObject node instances. This keeps runtime memory light and makes traversal fast.

### Speaker Profile

`USpeakerProfile` stores presentation-facing speaker data:

- `SpeakerTag`
- display name
- portrait texture set
- full-body texture set
- voice table reference
- text color
- default screen position

The runtime does not force a specific UI. The presenter can decide how to use this data.

### Dialogue Trigger

`UDialogueTrigger` describes when dialogue should start. It includes:

- trigger tag
- target tree
- entry node
- trigger type
- activation conditions
- cooldown
- one-shot flag
- priority

`UDialogueTriggerManager` can register triggers, choose the best valid trigger, reject invalid triggers, enforce cooldowns, and start dialogue.

## 8. Dialogue Node Reference

| Node Type | Purpose | Runtime Behavior |
|---|---|---|
| `Root` | Entry point | Follows its first edge. |
| `Speech` | One or more lines | Broadcasts `OnLineStarted`, waits for input between lines, records history and seen hashes. |
| `Choice` | Player selection | Filters choices by visibility conditions and waits for `SelectChoice`. |
| `Condition` | Branch logic | Evaluates outgoing edge conditions in order and follows the first passing branch. |
| `Event` | Project event | Broadcasts GameplayTag event and payload. Can wait for external completion if async. |
| `Wait` | Pause point | Enters `WaitingForInput`. |
| `Random` | Weighted branch | Chooses a valid outgoing edge by weight. |
| `Jump` | Local or cross-tree jump | Moves to an entry node in the same tree or target tree. |
| `Sequence` | Linear grouping | Uses normal outgoing flow as a lightweight sequence pattern. |
| `SubDialogue` | Nested dialogue | Enters another tree and can return to the caller. |
| `Camera` | Camera cue | Broadcasts a camera event, then continues. |
| `Animation` | Animation cue | Broadcasts an animation event, then continues. |

## 9. Authoring Dialogue Lines

`FDialogueLine` contains:

- `LineID`
- `SpeakerTag`
- `Text`
- `EmotionTag`
- `VoiceID`
- `MetaTags`
- `WaitTime`
- parsed inline `Segments`

Recommended `LineID` style:

```text
Chapter01_Alice_Intro_001
Chapter01_Alice_Intro_002
Chapter01_Alice_ChoiceYes_001
```

Stable `LineID` values are important because localization, voice tables, audit reports, and production spreadsheets can all refer to them.

## 10. Inline Tags

`UDialogueTextParser` parses text tags into line segments.

Supported authoring style:

```text
Hello [color=#FF0000]red text[wait=1.0] after pause
```

Typical tags:

- `[speed=0.05]`
- `[color=#FF0000]`
- `[shake]`
- `[wait=1.0]`
- `[portrait=Angry]`
- `[camera=CloseUp]`
- `[sfx=HeartBeat]`
- `[event=Chronicle.Event.Quest.Start]`

The runtime parses tags and exposes segment data. The presenter decides how to render or react to each tag.

## 11. Variables

`FVariableValue` supports:

- `Bool`
- `Int32`
- `Float`
- `String`
- `Name`
- `GameplayTag`
- `Vector`
- `List`

Scopes:

- `Global`: persistent project-level state.
- `Local`: current dialogue-tree state.
- `External`: resolved through native bindings or pushed from project systems.

Set a value:

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

Read a value:

```cpp
bool bFound = false;
FVariableValue Score = Runner->GetVariable(ScoreTag, bFound);
```

Project systems can push state into the runner:

```cpp
Runner->SetVariable(QuestStageTag, FVariableValue::MakeInt(3), EChronicleVariableScope::Global);
```

## 12. Condition Expressions

Conditions are used by:

- Condition nodes
- edge conditions
- choice visibility
- trigger activation
- audit and validation tooling

Supported syntax:

- variables: `Chronicle.Variable.Score`
- booleans: `true`, `false`
- numbers: `10`, `3.14`
- strings: `"Alice"`
- comparisons: `==`, `!=`, `>`, `>=`, `<`, `<=`
- boolean operators: `AND`, `OR`, `NOT`, `&&`, `||`, `!`
- parentheses: `( ... )`

Examples:

```text
Chronicle.Variable.Score >= 50
```

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true) OR Chronicle.Variable.Name == "Alice"
```

Editor validation:

```cpp
FChronicleConditionExpressionValidationResult Result;
FString Error;
UChronicleDialogueEditorLibrary::ValidateConditionExpressionForTree(Tree, Expression, Result, Error);
```

The result includes parse status, evaluation result, variable references, and a message suitable for editor UI.

## 13. Runtime API

Use `UChronicleDialogueSubsystem` as the main Blueprint and C++ entry point.

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
```

Common calls:

```cpp
Runner->Initialize(Database);
Runner->StartDialogue(DialogueTree);
Runner->StartDialogue(DialogueTree, FName(TEXT("IntroEntry")));
Runner->Advance();
Runner->SelectChoice(0);
Runner->EndDialogue();
Runner->NotifyEventComplete(EventTag);
Runner->SaveState(SaveData);
Runner->LoadState(SaveData);
Runner->PerformRollback(1);
```

Runner states:

- `Idle`
- `Running`
- `WaitingForInput`
- `WaitingForChoice`
- `WaitingForEvent`
- `Paused`

Runner events:

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`

## 14. Event Bus

Event payload:

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

Common tags:

- `Chronicle.Event.Quest.Start`
- `Chronicle.Event.Quest.Update`
- `Chronicle.Event.Quest.Complete`
- `Chronicle.Event.GameState.Change`
- `Chronicle.Event.Actor.Animate`
- `Chronicle.Event.Battle.Encounter`
- `Chronicle.Event.Scene.Load`
- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

Async event flow:

1. Event node broadcasts `EventData` with `bAsync = true`.
2. Runner enters `WaitingForEvent`.
3. Project system performs work, such as scene transition or long animation.
4. Project calls:

```cpp
Runner->NotifyEventComplete(EventTag);
```

5. Runner continues.

## 15. Quest And Game-State Integration

`UChronicleExampleQuestAdapter` demonstrates project-facing integration.

```cpp
UChronicleExampleQuestAdapter* Adapter = NewObject<UChronicleExampleQuestAdapter>();
Adapter->BindToRunner(Runner);
```

It listens to dialogue events and rebroadcasts:

- quest start
- quest update
- quest complete
- game-state change
- actor animation
- battle encounter
- scene load
- generic unhandled events

It also exposes `PushExternalVariable` so outside systems can feed state into the runner.

## 16. Presentation Controller

`UChronicleDialoguePresentationController` is the bridge between runtime and UI.

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
Presentation->BindRunner(Runner);
```

It tracks:

- last line
- visible choices
- last event data
- backlog
- auto mode
- skip mode
- last reveal mode

It drives:

```cpp
Presentation->StartDialogue(Tree);
Presentation->Advance();
Presentation->SelectChoice(ChoiceIndex);
Presentation->SetAutoAdvanceEnabled(true, 1.5f);
Presentation->SetSkipModeEnabled(true);
Presentation->RequestRollback(1);
Presentation->NotifyEventComplete(EventTag);
```

Delegates:

- `OnPresentationDialogueStarted`
- `OnPresentationDialogueEnded`
- `OnPresentationLineStarted`
- `OnPresentationChoicesPresented`
- `OnBacklogChanged`
- `OnPresentationEvent`
- `OnPresentationRunnerStateChanged`
- `OnAutoAdvanceChanged`
- `OnSkipModeChanged`
- `OnRollbackPerformed`

## 17. Default UI Widget

`UChronicleDialogueWidget` is the abstract base widget and implements `IDialoguePresenter`.

`UChronicleDialogueDefaultWidget` is the source-built default HUD. It can build a runtime layout automatically, so no binary Widget Blueprint is required.

Default widget capabilities:

- speaker text
- typewriter reveal
- current dialogue line
- choice list
- local backlog
- Backlog visibility
- Advance button
- Auto button
- Skip button
- Backlog button
- Back/Rollback button
- portrait image slot
- full-body image slot

Basic setup:

```cpp
UChronicleDialogueDefaultWidget* Widget = CreateWidget<UChronicleDialogueDefaultWidget>(World, UChronicleDialogueDefaultWidget::StaticClass());
Widget->AddToViewport();
Widget->BindPresentationController(Presentation);
```

For a custom Blueprint UI:

1. Create a Widget Blueprint.
2. Set parent class to `UChronicleDialogueWidget` or `UChronicleDialogueDefaultWidget`.
3. Bind it to the presentation controller.
4. Implement Blueprint events such as `On Line Started`, `On Choices Presented`, `On Rollback`, and `On Dialogue Event`.
5. Wire buttons to `AdvanceDialogue`, `SelectDialogueChoice`, `SetAutoAdvanceEnabled`, `SetSkipModeEnabled`, and `RequestRollback`.

## 18. Cue Router And Cue Director

`UChronicleDialogueCueRouter` converts presentation events into camera/audio cue structs.

It routes:

- `Chronicle.Camera.*` events to `FChronicleCameraCue`
- `Chronicle.Audio.*` events to `FChronicleAudioCue`
- line `VoiceID` values to `FChronicleAudioCue`

Use it when a Blueprint subsystem wants cue data but does not need an actor.

```cpp
UChronicleDialogueCueRouter* CueRouter = NewObject<UChronicleDialogueCueRouter>();
CueRouter->BindPresentationController(Presentation);
```

`AChronicleDialogueCueDirector` is a level actor wrapper. It can:

- auto-bind to the Chronicle subsystem on BeginPlay
- register named camera actors
- broadcast camera/audio cue delegates
- call `SetViewTargetWithBlend` for matching camera cues

Level setup:

1. Place `AChronicleDialogueCueDirector`.
2. Keep `bAutoBindToSubsystemOnBeginPlay = true`.
3. Register camera shots by name.
4. Use a Camera node or camera event with payload:

```text
Shot=IntroCloseUp
BlendTime=0.35
```

## 19. Auto, Skip, Backlog, And Rollback

Auto mode:

- advances while the runner is `WaitingForInput`
- pauses at choices
- uses `AutoAdvanceDelay`

Skip mode:

- drains `WaitingForInput` lines up to `MaxSkipStepsPerTick`
- stops at choices, events, and dialogue end
- emits instant reveal mode while skipping

Backlog:

- presentation-facing history list
- updated on each line
- synchronized from runner save data after rollback

Rollback:

- calls `UDialogueRunner::PerformRollback`
- restores previous memento state
- rebroadcasts the restored backlog to presenters

## 20. Editor Workflow

The Dialogue Tree editor supports:

- native Unreal graph canvas
- all PRD node types in the context menu
- node movement with saved positions
- pin connection edge creation
- pin break edge deletion
- selected-node Details editing
- node search
- validation summary
- copy, paste, duplicate, delete
- zoom to fit
- breakpoint metadata
- debugger snapshots
- soft locks
- pipeline asset actions

Useful editor APIs:

- `AddDialogueNode`
- `SetDialogueNodePosition`
- `RemoveDialogueNodes`
- `DuplicateDialogueNodes`
- `AddDialogueEdge`
- `RemoveDialogueEdge`
- `SetDialogueNodeBreakpoint`
- `GetDialogueNodeBreakpoints`
- `CaptureDebuggerSnapshot`
- `ValidateConditionExpressionForTree`
- `BuildDefaultDialogueTreeExportPaths`
- `ExportDialogueTreePipelineArtifacts`
- `AcquireDialogueTreeLock`
- `ReleaseDialogueTreeLock`

## 21. Debugging

`CaptureDebuggerSnapshot` returns:

- current tree
- current tree GUID
- current node GUID
- current node type
- current line index
- runner state
- breakpoint flag
- node title
- variables with scope and value
- outgoing edges with condition result
- history
- seen dialogue hashes

This gives editor tools and Blueprint utilities enough state to build live debugging panels.

## 22. Asset Pipeline

### JSON Tree Export/Import

Use JSON when you need source-control-friendly review, external tooling, or backup text snapshots.

```cpp
UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonFile(Tree, FilePath, Error);
UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonFile(Tree, FilePath, Error);
```

### Dialogue Line CSV

Use line CSV for writer and localization handoff when graph topology already exists.

Columns include:

- `NodeGuid`
- `LineIndex`
- `LineID`
- `SpeakerTag`
- `SourceText`
- `TranslatedText`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `ContextComment`

### Script CSV Import

Use script CSV when writers author dialogue in spreadsheets and want to create graph topology.

Required columns:

- `LineID`
- `Text`

Optional columns:

- `SpeakerTag` or `Speaker`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `NextLineID`, `NextLine`, or `TargetLineID`
- `ConditionExpression` or `Condition`
- `EventTag`
- `EventPayload`
- `bEventIsAsync`, `EventAsync`, or `Async`

`EventPayload` format:

```text
QuestTag=Chronicle.Quest.Main;ObjectiveIndex=2
```

### One-Click Pipeline Export

Right-click a Dialogue Tree and choose `Export Chronicle Pipeline Artifacts...`.

Generated files:

- `{Tree}.dialogue.json`
- `{Tree}.lines.csv`
- `{Tree}.localization.csv`
- `{Tree}.audit.json`

API:

```cpp
FChronicleDialoguePipelineExportPaths Paths;
FString Error;
UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts(Tree, ExportDirectory, TEXT("en"), Paths, Error);
```

## 23. Localization Workflow

Chronicle uses stable `LineID` keys for localization handoff.

Recommended flow:

1. Ensure dialogue lines have stable `LineID` values.
2. Export localization CSV from a tree or database.
3. Send CSV to translators.
4. Fill `TranslatedText`.
5. Import CSV back to the tree.
6. Use culture-specific voice tables when required.

Useful APIs:

- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`
- `ResolveVoiceTableForCulture`

## 24. Audit And Validation

Validation detects:

- missing root
- duplicate node GUIDs
- broken edges
- unreachable nodes
- empty speech nodes
- empty choice nodes
- event nodes without tags

Audit reports include:

- node count
- edge count
- speech line count
- choice count
- word count
- speaker line and word stats
- variable usage
- broken edge count
- unreachable node count
- warning count
- error count
- full issue list

Use audit reports before recording, localization handoff, and release packaging.

## 25. CI Workflow

GitHub Actions workflow:

```text
.github/workflows/chronicle-validation.yml
```

It expects a self-hosted runner with labels:

```text
self-hosted, windows, unreal
```

Default engine roots:

```text
R:\UE\UE_5.3
R:\UE\UE_5.7
```

The workflow calls:

```powershell
./Scripts/RunChronicleValidation.ps1
```

## 26. Troubleshooting

### Unreal says modules are missing or built with another engine version

Build the project with the same engine version you are opening.

For UE 5.3:

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### The graph editor opens in a tiny area

Use the latest `main` branch. The editor toolkit now gives the graph tab proper layout space and toolbar support.

### GameplayTags are missing

Check `Config/DefaultGameplayTags.ini`. Copy equivalent tags into the target project or add them through Project Settings.

### Choices do not appear

Check:

- `VisibilityCondition`
- variable values
- expression syntax
- whether the choice has a matching outgoing edge

Empty visibility conditions are visible.

### Condition branches go to the wrong node

Check outgoing edge order and conditions. Condition nodes select the first passing edge. Keep the default branch last.

### Async event does not continue

Call:

```cpp
Runner->NotifyEventComplete(EventTag);
```

The tag must match the event that put the runner into `WaitingForEvent`.

### UMG does not receive updates

Check that the widget is bound:

```cpp
Widget->BindPresentationController(Presentation);
```

Also confirm that the runner used by the presentation controller is the same runner that starts the dialogue.

### Voice or camera cues do nothing

Chronicle emits cue data. Your project must bind to cue delegates or use `AChronicleDialogueCueDirector`.

For camera cuts:

1. Place `AChronicleDialogueCueDirector`.
2. Register camera shots.
3. Emit `Chronicle.Camera.Cut` with `Shot` payload.

For audio:

1. Bind to `UChronicleDialogueCueRouter::OnAudioCue`.
2. Resolve `CueName` or `LineID` in your audio system.
3. Play the matching sound, Wwise event, or project-specific cue.

### Package output is large

`Artifacts/` contains generated package files and is ignored by git. It can be deleted and regenerated.

## 27. Known Source-First Choices

The PRD describes several commercial-production features that can be delivered either as source systems or binary content. This repository intentionally keeps the tracked project source-first.

Implemented as source-first equivalents:

- default HUD through `UChronicleDialogueDefaultWidget`, not committed binary `WBP_*` assets
- demo through `AChronicleDialogueDemoActor`, not committed `.umap`
- camera/audio integration through cue delegates, not a forced cinematic/audio framework
- localization through CSV gather/import, not a forced UE manifest/archive command wrapper

Optional future release polish:

- prebuilt showcase map
- prebuilt Widget Blueprint assets
- Articy/Twine/Ren'Py importer modules
- full Slate live debugger panel with Step Into, Step Over, Continue, and Goto UI
- localization manifest/archive menu command wrapper

## 28. Additional Documentation

- [Documentation/AssetPipeline.md](Documentation/AssetPipeline.md)
- [Documentation/EditorWorkflow.md](Documentation/EditorWorkflow.md)
- [Documentation/IntegrationWorkflow.md](Documentation/IntegrationWorkflow.md)
- [Documentation/PresentationWorkflow.md](Documentation/PresentationWorkflow.md)
- [Documentation/PRDCompletionMatrix.md](Documentation/PRDCompletionMatrix.md)
- [Documentation/ReleaseChecklist.md](Documentation/ReleaseChecklist.md)
- [Documentation/ReleaseNotes.md](Documentation/ReleaseNotes.md)
- [Documentation/Roadmap.md](Documentation/Roadmap.md)

## 29. License

Chronicle Engine is open source under the MIT License. See [LICENSE](LICENSE).

---

# 中文手册

## 1. Chronicle Engine 是什么

Chronicle Engine 是一个 MIT 开源的 Unreal Engine 5 源码插件，面向 JRPG、视觉小说、AVG、剧情向 RPG 和重叙事 ARPG。仓库同时包含一个最小宿主项目，用来独立编译、测试、打包和验收插件，然后再把插件复制到正式项目中使用。

它的核心分层很清晰：

- 数据资产负责描述对白、角色、变量、触发器、本地化和制作管线信息。
- 运行时系统负责遍历 Dialogue Tree、计算条件、广播事件、保存状态和维护回滚历史。
- 表现层系统负责把运行时事件转换成 UI、镜头、音频、Backlog、Auto、Skip 和 Rollback 行为。
- 编辑器系统负责提供原生 UE 节点图、资产右键操作、验证、管线导出、调试快照和软锁。

插件不强制绑定某个任务系统、UI 风格、镜头框架、音频中间件或保存系统。它通过 GameplayTag 事件、Blueprint API、源码 Widget 和 C++ 扩展点，让项目按自己的架构接入。

## 2. 当前 PRD 完成度摘要

源码插件验收目标已完成。完整对照见 [Documentation/PRDCompletionMatrix.md](Documentation/PRDCompletionMatrix.md)。

| PRD 区域 | 当前状态 | 实现说明 |
|---|---|---|
| 数据层 | 完成 | 已实现 `UDialogueDatabase`、`UDialogueTree`、`USpeakerProfile`、`UDialogueTrigger` 和核心结构体。 |
| 运行时层 | 完成 | 已实现 `UDialogueRunner`、`UVariableBank`、条件表达式、事件、保存加载、回滚、随机、跳转、子对话、镜头和动画节点遍历。 |
| 编辑器层 | 基本完成 | 已有原生图编辑器、Details 辅助对象、搜索、验证、断点、调试快照、资产菜单和软锁。完整 live step debugger 面板属于可选增强。 |
| 管线层 | 完成 | 已有 JSON、对白 CSV、脚本 CSV、本地化 CSV、验证、审计报告和一键管线导出。Articy/Twine/Ren'Py 导入器属于后续扩展。 |
| 表现层 | 源码级完成 | 已有表现层控制器、源码默认 Widget、Cue Router、Cue Director 和 Demo Actor。二进制 `WBP_*` 资产故意不纳入源码仓库。 |
| 本地化 | 基本完成 | 已有稳定 Key、Gather helper、CSV 导入导出和分语言语音表查询。UE manifest/archive 命令包装属于可选增强。 |
| CI/发布 | 完成 | 已有本地打包脚本、完整验证脚本、自托管 GitHub Actions workflow 和发布检查表。 |

最新验证结果：

- UE 5.3 Editor 编译：通过。
- UE 5.3 `Chronicle` 自动化测试：35/35 通过。
- UE 5.3 100 节点条件遍历：`0.0844ms`，满足 PRD `<0.1ms` 目标。
- UE 5.7 Editor 编译冒烟：通过。
- UE 5.3 BuildPlugin 打包：通过。

## 3. 仓库结构

```text
ChronicleHost.uproject
Source/ChronicleHost/
Plugins/ChronicleEngine/
  ChronicleEngine.uplugin
  Config/
  Content/
  Source/
    ChronicleEngine/        Runtime 模块
    ChronicleEngineEditor/  Editor 模块
    ChronicleEngineTests/   自动化测试模块
Documentation/
Scripts/
.github/workflows/
```

重要文件：

- `ChronicleHost.uproject`：最小宿主项目。
- `Plugins/ChronicleEngine/ChronicleEngine.uplugin`：插件描述文件。
- `Plugins/ChronicleEngine/Source/ChronicleEngine`：运行时、数据、表现层和示例。
- `Plugins/ChronicleEngine/Source/ChronicleEngineEditor`：编辑器图、资产工厂、资产操作、导入导出和审计。
- `Plugins/ChronicleEngine/Source/ChronicleEngineTests`：UE 自动化测试。
- `Config/DefaultGameplayTags.ini`：Chronicle 标准和示例 GameplayTags。
- `Scripts/PackagePlugin.ps1`：本地打包脚本。
- `Scripts/RunChronicleValidation.ps1`：构建、测试、打包和兼容性验证脚本。
- `.github/workflows/chronicle-validation.yml`：自托管 Windows Unreal CI workflow。

被忽略的本地/生成内容：

- `01_PRD_项目需求文档.md`
- `Artifacts/`
- `Binaries/`
- `Intermediate/`
- `Saved/`
- 插件 `Binaries/` 和 `Intermediate/`

PRD 文件故意不提交到 git。

## 4. 安装方式

### 方式 A：使用 Release 包

1. 从 GitHub Releases 下载发布包。
2. 解压。
3. 复制插件文件夹到项目：

```text
YourProject/Plugins/ChronicleEngine
```

4. 重新生成项目文件。
5. 打开 Unreal 项目。
6. 在 Plugins 面板启用 `Chronicle Engine`。
7. 重启编辑器。
8. 使用当前引擎版本编译项目。

适合想使用稳定打包版的项目。

### 方式 B：使用源码仓库

1. 克隆本仓库。
2. 用 UE 5.3 打开 `ChronicleHost.uproject`。
3. 编译 `ChronicleHostEditor`。
4. 运行 Chronicle 自动化测试。
5. 准备接入正式项目时，把 `Plugins/ChronicleEngine` 复制过去；也可以继续在宿主项目中开发。

适合需要 `main` 最新功能的项目。

### 方式 C：接入已有 C++ 项目

1. 创建 `YourProject/Plugins/ChronicleEngine`。
2. 把打包版或源码版插件复制进去。
3. 重新生成项目文件。
4. 编译项目。
5. 确认 Plugins 面板中 `Chronicle Engine` 已启用。
6. 如果目标项目还没有 Chronicle Tags，把 `Config/DefaultGameplayTags.ini` 中的标签复制或重新创建。

### 方式 D：接入纯蓝图项目

源码插件仍需要编译。纯蓝图项目建议：

1. 在 UE 编辑器里添加一个空 C++ 类。
2. 关闭编辑器。
3. 把插件复制到 `YourProject/Plugins/ChronicleEngine`。
4. 重新生成项目文件。
5. 用 IDE 或 Unreal build tools 编译。
6. 重新打开编辑器。

## 5. 编译、测试、打包与验证

### UE 5.3 编译

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### 运行自动化测试

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

### 打包插件

```powershell
.\Scripts\PackagePlugin.ps1
```

指定引擎路径和包名：

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.12.0-UE5.7"
```

输出目录是 `Artifacts/`，该目录不会进入 git。

### 运行完整本地验证

```powershell
.\Scripts\RunChronicleValidation.ps1
```

该脚本会执行：

- UE 5.3 Editor 编译。
- UE 5.3 Chronicle 自动化测试。
- UE 5.3 BuildPlugin 打包。
- UE 5.7 Editor 编译冒烟。
- UE 5.7 冒烟后重新运行 UE 5.3 编译，让本地二进制回到基线版本。

常用参数：

```powershell
.\Scripts\RunChronicleValidation.ps1 -SkipAutomation
.\Scripts\RunChronicleValidation.ps1 -SkipPackage
.\Scripts\RunChronicleValidation.ps1 -Skip57Smoke
```

## 6. 十分钟快速上手

### 第一步：打开宿主项目

用 UE 5.3 打开 `ChronicleHost.uproject`。如果 Unreal 提示是否重建模块，选择重建。

### 第二步：创建 Dialogue Tree

1. 打开 Content Browser。
2. 右键。
3. 创建 `Dialogue Tree` 资产。
4. 双击资产。
5. 打开 Chronicle 图编辑器。

### 第三步：搭建最小图

1. 创建 `Root` 节点。
2. 创建 `Speech` 节点。
3. 连接 `Root` 到 `Speech`。
4. 选中 `Speech`。
5. 在 Details 中编辑第一行对白文本。
6. 保存资产。

### 第四步：添加选项

1. 创建 `Choice` 节点。
2. 连接 `Speech` 到 `Choice`。
3. 添加两个选项。
4. 再创建两个 `Speech` 节点作为结果。
5. 把 choice slot `0` 连接到第一个结果。
6. 把 choice slot `1` 连接到第二个结果。
7. 保存并运行验证。

### 第五步：运行时播放

通过子系统启动：

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

绑定 UI：

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
DefaultWidget->BindPresentationController(Presentation);
```

### 第六步：使用源码 Demo

1. 把 `AChronicleDialogueDemoActor` 放进关卡。
2. 保持 `bStartOnBeginPlay` 开启。
3. 保持 `bCreateDefaultWidget` 开启。
4. 运行 PIE。
5. 该 Actor 会生成 demo tree，创建默认 HUD，绑定表现层控制器并启动对话。

## 7. 核心概念

### Dialogue Database

`UDialogueDatabase` 是可选的项目/章节叙事数据库，包含：

- 角色档案
- 全局变量定义
- Dialogue Tree 引用
- 本地化设置
- 默认语音表
- 分语言语音表
- 编辑器锁定元数据

当项目需要集中管理叙事数据库、角色查询、全局变量或语音表路由时使用它。

### Dialogue Tree

`UDialogueTree` 是主要对白资产，包含：

- `Nodes`：`FDialogueNode` 数组
- `Edges`：`FDialogueEdge` 数组
- `RootNodeGuid`
- 局部变量定义
- 编辑器状态，如断点和软锁

节点使用结构体存储，而不是每个节点一个 UObject。这样运行时内存更轻，遍历更快。

### Speaker Profile

`USpeakerProfile` 保存表现层角色信息：

- `SpeakerTag`
- 显示名
- 头像贴图表
- 立绘贴图表
- 语音表引用
- 文本颜色
- 默认屏幕位置

运行时不会强制某种 UI，Presenter 自己决定如何使用这些数据。

### Dialogue Trigger

`UDialogueTrigger` 描述对话何时启动，包含：

- 触发器标签
- 目标 Dialogue Tree
- 入口节点
- 触发类型
- 激活条件
- 冷却时间
- 一次性标记
- 优先级

`UDialogueTriggerManager` 可以注册触发器、选择最合适的触发器、拒绝无效触发、执行冷却并启动对话。

## 8. 节点类型参考

| 节点类型 | 用途 | 运行时行为 |
|---|---|---|
| `Root` | 入口 | 跟随第一条输出边。 |
| `Speech` | 一行或多行对白 | 广播 `OnLineStarted`，行间等待输入，记录历史和已读哈希。 |
| `Choice` | 玩家选项 | 按可见性条件过滤选项，等待 `SelectChoice`。 |
| `Condition` | 分支逻辑 | 按顺序评估输出边条件，进入第一条通过的分支。 |
| `Event` | 项目事件 | 广播 GameplayTag 事件和载荷，可异步等待外部完成。 |
| `Wait` | 暂停点 | 进入 `WaitingForInput`。 |
| `Random` | 权重随机 | 按权重选择有效输出边。 |
| `Jump` | 本树或跨树跳转 | 移动到当前树或目标树的入口节点。 |
| `Sequence` | 线性组合 | 使用普通输出流作为轻量 sequence 模式。 |
| `SubDialogue` | 嵌套对话 | 进入另一棵树，并可返回调用方。 |
| `Camera` | 镜头 cue | 广播镜头事件后继续。 |
| `Animation` | 动画 cue | 广播动画事件后继续。 |

## 9. 编写对白行

`FDialogueLine` 包含：

- `LineID`
- `SpeakerTag`
- `Text`
- `EmotionTag`
- `VoiceID`
- `MetaTags`
- `WaitTime`
- 解析后的内联 `Segments`

推荐的 `LineID` 风格：

```text
Chapter01_Alice_Intro_001
Chapter01_Alice_Intro_002
Chapter01_Alice_ChoiceYes_001
```

稳定的 `LineID` 很重要，因为本地化、语音表、审计报告和制作表格都可以引用它。

## 10. 内联标签

`UDialogueTextParser` 会把文本标签解析成 line segments。

示例：

```text
Hello [color=#FF0000]red text[wait=1.0] after pause
```

常见标签：

- `[speed=0.05]`
- `[color=#FF0000]`
- `[shake]`
- `[wait=1.0]`
- `[portrait=Angry]`
- `[camera=CloseUp]`
- `[sfx=HeartBeat]`
- `[event=Chronicle.Event.Quest.Start]`

运行时负责解析标签并暴露段数据。具体如何显示或响应，由 Presenter 决定。

## 11. 变量

`FVariableValue` 支持：

- `Bool`
- `Int32`
- `Float`
- `String`
- `Name`
- `GameplayTag`
- `Vector`
- `List`

作用域：

- `Global`：项目级持久状态。
- `Local`：当前 Dialogue Tree 状态。
- `External`：通过原生绑定解析，或由项目系统推送。

设置变量：

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

读取变量：

```cpp
bool bFound = false;
FVariableValue Score = Runner->GetVariable(ScoreTag, bFound);
```

项目系统向 Runner 推送状态：

```cpp
Runner->SetVariable(QuestStageTag, FVariableValue::MakeInt(3), EChronicleVariableScope::Global);
```

## 12. 条件表达式

条件表达式用于：

- Condition 节点
- 边条件
- 选项可见性
- 触发器激活条件
- 审计和验证工具

支持语法：

- 变量：`Chronicle.Variable.Score`
- 布尔值：`true`、`false`
- 数字：`10`、`3.14`
- 字符串：`"Alice"`
- 比较：`==`、`!=`、`>`、`>=`、`<`、`<=`
- 布尔运算：`AND`、`OR`、`NOT`、`&&`、`||`、`!`
- 括号：`( ... )`

示例：

```text
Chronicle.Variable.Score >= 50
```

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true) OR Chronicle.Variable.Name == "Alice"
```

编辑器校验：

```cpp
FChronicleConditionExpressionValidationResult Result;
FString Error;
UChronicleDialogueEditorLibrary::ValidateConditionExpressionForTree(Tree, Expression, Result, Error);
```

返回结果包含解析状态、求值结果、变量引用列表和可用于编辑器 UI 的消息。

## 13. 运行时 API

建议用 `UChronicleDialogueSubsystem` 作为 Blueprint 和 C++ 入口。

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
```

常用调用：

```cpp
Runner->Initialize(Database);
Runner->StartDialogue(DialogueTree);
Runner->StartDialogue(DialogueTree, FName(TEXT("IntroEntry")));
Runner->Advance();
Runner->SelectChoice(0);
Runner->EndDialogue();
Runner->NotifyEventComplete(EventTag);
Runner->SaveState(SaveData);
Runner->LoadState(SaveData);
Runner->PerformRollback(1);
```

Runner 状态：

- `Idle`
- `Running`
- `WaitingForInput`
- `WaitingForChoice`
- `WaitingForEvent`
- `Paused`

Runner 事件：

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`

## 14. 事件总线

事件载荷：

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

常用标签：

- `Chronicle.Event.Quest.Start`
- `Chronicle.Event.Quest.Update`
- `Chronicle.Event.Quest.Complete`
- `Chronicle.Event.GameState.Change`
- `Chronicle.Event.Actor.Animate`
- `Chronicle.Event.Battle.Encounter`
- `Chronicle.Event.Scene.Load`
- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

异步事件流程：

1. Event 节点广播 `EventData`，其中 `bAsync = true`。
2. Runner 进入 `WaitingForEvent`。
3. 项目系统执行场景切换、长动画等外部工作。
4. 项目调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

5. Runner 继续运行。

## 15. 任务和游戏状态接入

`UChronicleExampleQuestAdapter` 展示项目侧接入方式。

```cpp
UChronicleExampleQuestAdapter* Adapter = NewObject<UChronicleExampleQuestAdapter>();
Adapter->BindToRunner(Runner);
```

它监听对话事件并重新广播：

- 任务开始
- 任务更新
- 任务完成
- 游戏状态变化
- 角色动画
- 战斗遭遇
- 场景加载
- 未处理的通用事件

它也提供 `PushExternalVariable`，方便外部系统把状态写入 Runner。

## 16. 表现层控制器

`UChronicleDialoguePresentationController` 是运行时和 UI 之间的桥。

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
Presentation->BindRunner(Runner);
```

它保存：

- 最新对白行
- 可见选项
- 最新事件数据
- Backlog
- Auto 模式
- Skip 模式
- 最新显示模式

它驱动：

```cpp
Presentation->StartDialogue(Tree);
Presentation->Advance();
Presentation->SelectChoice(ChoiceIndex);
Presentation->SetAutoAdvanceEnabled(true, 1.5f);
Presentation->SetSkipModeEnabled(true);
Presentation->RequestRollback(1);
Presentation->NotifyEventComplete(EventTag);
```

委托：

- `OnPresentationDialogueStarted`
- `OnPresentationDialogueEnded`
- `OnPresentationLineStarted`
- `OnPresentationChoicesPresented`
- `OnBacklogChanged`
- `OnPresentationEvent`
- `OnPresentationRunnerStateChanged`
- `OnAutoAdvanceChanged`
- `OnSkipModeChanged`
- `OnRollbackPerformed`

## 17. 默认 UI Widget

`UChronicleDialogueWidget` 是抽象基础 Widget，并实现 `IDialoguePresenter`。

`UChronicleDialogueDefaultWidget` 是源码默认 HUD。它可以运行时自动构建布局，所以仓库不需要提交二进制 Widget Blueprint。

默认 Widget 能力：

- 说话人文本
- 打字机显示
- 当前对白
- 选项列表
- 本地 Backlog
- Backlog 显示/隐藏
- Advance 按钮
- Auto 按钮
- Skip 按钮
- Backlog 按钮
- Back/Rollback 按钮
- 头像槽位
- 立绘槽位

基础设置：

```cpp
UChronicleDialogueDefaultWidget* Widget = CreateWidget<UChronicleDialogueDefaultWidget>(World, UChronicleDialogueDefaultWidget::StaticClass());
Widget->AddToViewport();
Widget->BindPresentationController(Presentation);
```

自定义蓝图 UI：

1. 创建 Widget Blueprint。
2. 父类选择 `UChronicleDialogueWidget` 或 `UChronicleDialogueDefaultWidget`。
3. 绑定表现层控制器。
4. 实现 `On Line Started`、`On Choices Presented`、`On Rollback`、`On Dialogue Event` 等 Blueprint 事件。
5. 把按钮连接到 `AdvanceDialogue`、`SelectDialogueChoice`、`SetAutoAdvanceEnabled`、`SetSkipModeEnabled` 和 `RequestRollback`。

## 18. Cue Router 与 Cue Director

`UChronicleDialogueCueRouter` 会把表现层事件转换成镜头/音频 cue 结构体。

它会路由：

- `Chronicle.Camera.*` 事件到 `FChronicleCameraCue`
- `Chronicle.Audio.*` 事件到 `FChronicleAudioCue`
- 对白行 `VoiceID` 到 `FChronicleAudioCue`

当 Blueprint 子系统需要 cue 数据但不需要关卡 Actor 时，使用它。

```cpp
UChronicleDialogueCueRouter* CueRouter = NewObject<UChronicleDialogueCueRouter>();
CueRouter->BindPresentationController(Presentation);
```

`AChronicleDialogueCueDirector` 是关卡 Actor 包装。它可以：

- BeginPlay 自动绑定 Chronicle 子系统
- 注册命名 Camera Actor
- 广播镜头/音频 cue 委托
- 对匹配镜头 cue 调用 `SetViewTargetWithBlend`

关卡设置：

1. 放置 `AChronicleDialogueCueDirector`。
2. 保持 `bAutoBindToSubsystemOnBeginPlay = true`。
3. 注册 camera shot 名称。
4. 使用 Camera 节点或镜头事件，并填写载荷：

```text
Shot=IntroCloseUp
BlendTime=0.35
```

## 19. Auto、Skip、Backlog 与 Rollback

Auto 模式：

- 只在 Runner `WaitingForInput` 时自动推进
- 遇到选项会暂停
- 使用 `AutoAdvanceDelay`

Skip 模式：

- 每 tick 最多推进 `MaxSkipStepsPerTick` 条 `WaitingForInput` 文本
- 遇到选项、事件等待或对话结束会停止
- Skip 时使用瞬显模式

Backlog：

- 表现层历史列表
- 每行对白出现时更新
- 回滚后从 Runner SaveData 同步

Rollback：

- 调用 `UDialogueRunner::PerformRollback`
- 恢复上一个 memento 状态
- 把恢复后的 Backlog 重新广播给 Presenter

## 20. 编辑器工作流

Dialogue Tree 编辑器支持：

- 原生 UE 图画布
- 右键菜单创建全部 PRD 节点类型
- 拖动节点并保存位置
- 连接 Pin 创建边
- 断开 Pin 删除边
- 选中节点 Details 编辑
- 节点搜索
- 验证摘要
- 复制、粘贴、复制副本、删除
- 缩放到合适视图
- 断点元数据
- 调试快照
- 软锁
- 管线资产操作

常用编辑器 API：

- `AddDialogueNode`
- `SetDialogueNodePosition`
- `RemoveDialogueNodes`
- `DuplicateDialogueNodes`
- `AddDialogueEdge`
- `RemoveDialogueEdge`
- `SetDialogueNodeBreakpoint`
- `GetDialogueNodeBreakpoints`
- `CaptureDebuggerSnapshot`
- `ValidateConditionExpressionForTree`
- `BuildDefaultDialogueTreeExportPaths`
- `ExportDialogueTreePipelineArtifacts`
- `AcquireDialogueTreeLock`
- `ReleaseDialogueTreeLock`

## 21. 调试

`CaptureDebuggerSnapshot` 返回：

- 当前 Tree
- 当前 Tree GUID
- 当前 Node GUID
- 当前 Node 类型
- 当前行号
- Runner 状态
- 是否有断点
- 节点标题
- 变量值和作用域
- 输出边及条件结果
- 历史
- 已看对白哈希

这些数据足够编辑器工具或蓝图工具构建实时调试面板。

## 22. 资产管线

### JSON Tree 导入导出

当需要源码友好审查、外部工具或文本备份时使用 JSON。

```cpp
UChronicleDialogueJsonLibrary::ExportDialogueTreeToJsonFile(Tree, FilePath, Error);
UChronicleDialogueJsonLibrary::ImportDialogueTreeFromJsonFile(Tree, FilePath, Error);
```

### 对白行 CSV

当图拓扑已经存在，只需要给编剧或本地化人员交接文本时，使用对白行 CSV。

列包括：

- `NodeGuid`
- `LineIndex`
- `LineID`
- `SpeakerTag`
- `SourceText`
- `TranslatedText`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `ContextComment`

### 脚本 CSV 导入

当编剧在表格里写对白并希望创建图拓扑时，使用脚本 CSV。

必填列：

- `LineID`
- `Text`

可选列：

- `SpeakerTag` 或 `Speaker`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `NextLineID`、`NextLine` 或 `TargetLineID`
- `ConditionExpression` 或 `Condition`
- `EventTag`
- `EventPayload`
- `bEventIsAsync`、`EventAsync` 或 `Async`

`EventPayload` 格式：

```text
QuestTag=Chronicle.Quest.Main;ObjectiveIndex=2
```

### 一键管线导出

右键 Dialogue Tree，选择 `Export Chronicle Pipeline Artifacts...`。

生成文件：

- `{Tree}.dialogue.json`
- `{Tree}.lines.csv`
- `{Tree}.localization.csv`
- `{Tree}.audit.json`

API：

```cpp
FChronicleDialoguePipelineExportPaths Paths;
FString Error;
UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts(Tree, ExportDirectory, TEXT("en"), Paths, Error);
```

## 23. 本地化流程

Chronicle 使用稳定的 `LineID` 作为本地化交接 Key。

推荐流程：

1. 确保对白行有稳定 `LineID`。
2. 从 Tree 或 Database 导出本地化 CSV。
3. 发送 CSV 给翻译。
4. 填写 `TranslatedText`。
5. 把 CSV 导回 Tree。
6. 如有需要，使用分语言语音表。

常用 API：

- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`
- `ResolveVoiceTableForCulture`

## 24. 审计与验证

验证会检测：

- 缺少 Root
- 重复 Node GUID
- 断边
- 不可达节点
- 空 Speech 节点
- 空 Choice 节点
- Event 节点缺少 Tag

审计报告包含：

- 节点数
- 边数
- 对白行数
- 选项数
- 词数
- 说话人台词和词数统计
- 变量使用情况
- 断边数量
- 不可达节点数量
- Warning 数量
- Error 数量
- 完整问题列表

建议在录音、本地化交接和发布打包前生成审计报告。

## 25. CI 工作流

GitHub Actions workflow：

```text
.github/workflows/chronicle-validation.yml
```

需要自托管 runner 标签：

```text
self-hosted, windows, unreal
```

默认引擎路径：

```text
R:\UE\UE_5.3
R:\UE\UE_5.7
```

workflow 调用：

```powershell
./Scripts/RunChronicleValidation.ps1
```

## 26. 常见问题

### Unreal 提示模块缺失或由其他引擎版本构建

用你正在打开项目的同一引擎版本编译项目。

UE 5.3：

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### 图编辑器打开后区域很小

使用最新 `main` 分支。现在编辑器 toolkit 已给图 Tab 正确布局空间，并包含工具栏支持。

### 找不到 GameplayTags

检查 `Config/DefaultGameplayTags.ini`。把等价标签复制到目标项目，或在 Project Settings 中添加。

### Choice 没显示

检查：

- `VisibilityCondition`
- 变量值
- 表达式语法
- 该选项是否有对应输出边

空可见性条件表示可见。

### Condition 分支走错节点

检查输出边顺序和条件。Condition 节点会选择第一条通过的边。默认分支建议放最后。

### Async Event 后对话不继续

调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

Tag 必须和让 Runner 进入 `WaitingForEvent` 的事件匹配。

### UMG 收不到更新

检查 Widget 是否已绑定：

```cpp
Widget->BindPresentationController(Presentation);
```

也要确认表现层控制器绑定的 Runner，和启动对话的 Runner 是同一个。

### 语音或镜头 cue 没反应

Chronicle 负责发出 cue 数据。项目需要绑定 cue 委托，或使用 `AChronicleDialogueCueDirector`。

镜头切换：

1. 放置 `AChronicleDialogueCueDirector`。
2. 注册 camera shots。
3. 发出 `Chronicle.Camera.Cut`，并带上 `Shot` 载荷。

音频：

1. 绑定 `UChronicleDialogueCueRouter::OnAudioCue`。
2. 在音频系统中解析 `CueName` 或 `LineID`。
3. 播放匹配的 Sound、Wwise 事件或项目自定义 cue。

### 打包输出太大

`Artifacts/` 是生成目录，已被 git 忽略。可以删除后重新生成。

## 27. 已知源码优先取舍

PRD 中有一些商业项目功能既可以用源码系统交付，也可以用二进制内容交付。本仓库故意保持源码优先。

已用源码等价实现：

- 通过 `UChronicleDialogueDefaultWidget` 提供默认 HUD，而不是提交二进制 `WBP_*` 资产。
- 通过 `AChronicleDialogueDemoActor` 提供 demo，而不是提交 `.umap`。
- 通过 cue 委托接入镜头/音频，而不是强制绑定某个 cinematic/audio 框架。
- 通过 CSV gather/import 接入本地化，而不是强制 UE manifest/archive 命令包装。

后续可选发布增强：

- 预制展示地图
- 预制 Widget Blueprint 资产
- Articy/Twine/Ren'Py 导入器模块
- 包含 Step Into、Step Over、Continue、Goto UI 的完整 Slate live debugger 面板
- 本地化 manifest/archive 菜单命令包装

## 28. 更多文档

- [Documentation/AssetPipeline.md](Documentation/AssetPipeline.md)
- [Documentation/EditorWorkflow.md](Documentation/EditorWorkflow.md)
- [Documentation/IntegrationWorkflow.md](Documentation/IntegrationWorkflow.md)
- [Documentation/PresentationWorkflow.md](Documentation/PresentationWorkflow.md)
- [Documentation/PRDCompletionMatrix.md](Documentation/PRDCompletionMatrix.md)
- [Documentation/ReleaseChecklist.md](Documentation/ReleaseChecklist.md)
- [Documentation/ReleaseNotes.md](Documentation/ReleaseNotes.md)
- [Documentation/Roadmap.md](Documentation/Roadmap.md)

## 29. 许可证

Chronicle Engine 使用 MIT License 开源。见 [LICENSE](LICENSE)。
