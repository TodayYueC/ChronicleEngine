# Chronicle Engine Release Notes

## 0.9.0-dev Integration And Audit Pass

This development pass adds source-first project integration examples and production audit reporting.

### Highlights

- Added standard Gameplay Tags for quest, game-state, actor animation, battle encounter, and scene-load dialogue events.
- Added `UChronicleExampleQuestAdapter`, a Runtime sample that binds to `UDialogueRunner::OnDialogueEvent`.
- The adapter rebroadcasts quest start/update/complete, game-state change, actor animation, battle encounter, scene load, and unhandled dialogue events.
- The adapter exposes `PushExternalVariable` for project systems that need to feed runner variables.
- Added `UChronicleDialogueAuditLibrary` in the Editor module.
- Audit reports include node/edge counts, speech line counts, choice counts, word counts, speaker stats, condition/payload variable usage, broken edges, unreachable nodes, and validation issue totals.
- Added JSON export for audit reports.
- Added automation coverage for the example quest adapter and audit reports.

### Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 32 tests passed.
- UE 5.7 editor build smoke: passed.
- 100-node condition traversal recorded `0.0986ms`.

## 0.8.0-dev Localization Pipeline Pass

This development pass adds the first complete localization gather/import workflow.

### Highlights

- Added stable `LineID` repair through `EnsureStableLineIds`.
- Added tree-level and database-level text gather helpers.
- Added localization CSV export/import using stable `Key` / `LineID`.
- Localization import ignores empty `TranslatedText` cells, so partial translation sheets do not overwrite source text.
- Added `CultureVoiceTables` to `UDialogueDatabase`.
- Added `ResolveVoiceTableForCulture`, with culture-specific lookup and default `VoiceTable` fallback.
- Added automation coverage for stable key repair, gather, localization CSV import, database gather, and voice-table culture fallback.

### Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 30 tests passed.
- UE 5.7 editor build smoke: passed.
- 100-node condition traversal recorded `0.0557ms` after warm-up.

## 0.7.0-dev Default UMG Presentation Pass

This development pass adds a concrete, source-built default dialogue HUD on top of the existing presentation controller.

### Highlights

- Added `UChronicleDialogueDefaultWidget`, a ready-to-use UMG dialogue HUD that can build its own runtime layout.
- Added `UChronicleDialogueChoiceButton`, an indexed choice button used by generated choice lists.
- The default HUD tracks speaker text, typewriter reveal state, current choices, local backlog, Backlog visibility, Auto, and Skip mode.
- The default HUD includes Advance, Auto, Skip, Backlog, and Back controls.
- The default HUD exposes portrait and full-body image slots for project-specific speaker art.
- Added automation coverage for default widget state, choice forwarding, local backlog, Auto, and Skip controls.

### Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 30 tests passed.
- UE 5.7 editor build smoke: passed.
- 100-node condition traversal recorded `0.0557ms` after warm-up.

## 0.6.0-dev Editor Workflow Pass

This development pass expands the Dialogue Tree editor toward the full PRD editing workflow.

### Highlights

- The graph context menu now exposes every PRD node type: Root, Speech, Choice, Condition, Event, Wait, Random, Jump, Sequence, SubDialogue, Camera, and Animation.
- Editor helper APIs now support deleting selected nodes and duplicating selected node groups.
- Duplicating a node group preserves internal selected edges and assigns fresh node GUIDs and line IDs.
- Deleting selected nodes now removes attached edges and stale breakpoint metadata.
- The editor toolbar and graph command list now expose copy, paste, duplicate, delete, and zoom-to-fit actions.
- Graph node summaries and colors now cover Random, Jump, Sequence, SubDialogue, Camera, and Animation nodes.

### Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 30 tests passed.
- 100-node condition traversal recorded `0.0557ms` after warm-up.

## 0.5.0 Release Notes

Chronicle Engine 0.5.0 is the M5 hardening pass for the UE5 JRPG dialogue plugin.

## Highlights

- Random, Jump, SubDialogue, Camera, and Animation nodes now have runtime behavior instead of falling back to generic first-edge traversal.
- SubDialogue nodes can enter a target dialogue tree and return to the caller's next node when the sub-dialogue branch completes.
- Camera and Animation nodes broadcast presentation events with default cue tags, so the existing presentation controller can route them like other dialogue events.
- Dialogue Trigger assets are now available, backed by `UDialogueTriggerManager` for priority selection, activation conditions, cooldowns, one-shot consumption, tag activation, and default presentation-controller startup.
- Runtime traversal now builds per-run node and outgoing-edge lookup tables instead of scanning arrays on every hop.
- Condition expressions compile to a cached AST after first use.
- Variable name to Gameplay Tag resolution is cached in `UVariableBank`.
- Repeated condition results are cached during a dialogue run and invalidated when variables, save state, or events can change runtime state.
- Rollback mementos are now stored at player-visible pause points instead of every internal node transition.
- The 100-node condition traversal automation budget is tightened to `0.25ms`; the latest UE 5.3 editor automation run recorded `0.0557ms` after warm-up.
- Packaging is scripted with `Scripts/PackagePlugin.ps1`.

## Verification

- UE 5.3 editor build: passed.
- UE 5.3 `Chronicle` automation suite: 30 tests passed.
- UE 5.7 editor build smoke: passed.
- UE 5.3 `BuildPlugin` package: passed for Win64 Editor, Development Game, and Shipping Game targets.

## Packaging

Use the default UE 5.3 install path:

```powershell
.\Scripts\PackagePlugin.ps1
```

Or pass a different engine root:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

Generated packages are written under `Artifacts/`, which is intentionally ignored by git.
