# Chronicle Engine Roadmap

## Four-Phase Execution Plan

Future iterations should follow these four phases unless the user explicitly changes the plan.

### Phase 1: Editor And Asset Pipeline Closeout

Status: complete for the `v0.11.0-dev` iteration.

- Dialogue Tree editor remains usable for creation, graph editing, Details editing, validation, search, duplicate/delete, breakpoints, and soft locks.
- Asset pipeline now has Content Browser actions for Dialogue Tree pipeline export and script CSV import.
- `UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts` exports JSON, line CSV, localization CSV, and audit JSON in one call.
- `ValidateConditionExpressionForTree` validates condition expressions against a tree's variable defaults and reports parse result, evaluation result, and variable references.

### Phase 2: Debugger And Runtime Completion

Status: complete for the `v0.11.0-dev` iteration.

- Debugger snapshots now expose current node type, current line index, variables, history, seen-line hashes, and outgoing edge condition results.
- Snapshot edge evaluation uses the runner's saved variable state, so editor debugging reflects the live traversal branch context.
- Automation now covers the expanded debugger snapshot and the editor pipeline tools.

### Phase 3: Presentation And Demo Polish

Status: complete for the `v0.12.0-dev` iteration.

- Added source-first cue routing through `UChronicleDialogueCueRouter`.
- Added `AChronicleDialogueCueDirector` as a level actor for camera/audio cue routing and optional camera view-target application.
- `AChronicleDialogueDemoActor` now defaults to the source-built HUD and can create/bind it at runtime.
- Binary sample assets remain optional and out of source control.

### Phase 4: Release, CI, Compatibility, And Final Acceptance

Status: complete for the `v0.12.0-dev` iteration.

- Added `Scripts/RunChronicleValidation.ps1` as the shared local/CI validation entry point.
- Added `.github/workflows/chronicle-validation.yml` for self-hosted Windows Unreal validation.
- Re-ran UE 5.3 build/tests, UE 5.7 smoke, package validation, release checklist, and final PRD comparison.
- Added `Documentation/PRDCompletionMatrix.md` for final requirements comparison.

## M1 Runtime Vertical Slice

Acceptance:

- The host project compiles with UE 5.3.
- Automation tests pass for variable storage, condition evaluation, dialogue traversal, events, save/load, rollback, and inline tags.
- Dialogue can be represented entirely with data structs and traversed without editor-only dependencies.

## M2 Asset Pipeline

Status: complete for the v0.11 editor pipeline tools pass.

- Stable JSON import/export with schema marker and deterministic node/edge ordering.
- CSV export/import for localization and writer text handoff.
- Stable localization gather keys through `LineID`, with automatic missing/duplicate key repair from `TreeGuid + NodeGuid + LineIndex`.
- Tree/database text gather helpers and localization CSV export/import by stable key.
- Culture-specific voice-table lookup through `UDialogueDatabase::CultureVoiceTables` and `ResolveVoiceTableForCulture`.
- Dialogue-tree validation for missing root, broken edges, duplicate node GUIDs, empty content warnings, and unreachable nodes.
- Dialogue audit reports through `UChronicleDialogueAuditLibrary`, covering node/edge counts, line/choice/word counts, speaker stats, variable usage, broken edges, unreachable nodes, and validation totals.
- CSV script import through `ImportDialogueScriptCsvString`, `ImportDialogueScriptCsvFile`, and `UChronicleCsvDialogueImporter`, creating runnable graph topology from Excel-authored rows.
- Importer extensibility through `UDialogueImporterBase`.
- Content Browser asset actions now expose one-click pipeline export and script CSV import for Dialogue Tree assets.
- `ExportDialogueTreePipelineArtifacts` writes JSON, dialogue-line CSV, localization CSV, and audit JSON together.
- Remaining pipeline polish moves to v1.0: richer external spreadsheet templates and optional XLSX helpers.

## M3 Editor Experience

Status: complete for the v0.11 editor workflow pass.

- Dialogue Tree assets open in a Chronicle custom asset editor instead of the default details-only view.
- Added a native `SGraphEditor` view backed by Chronicle EdGraph/Node/Schema classes, with search selection, validation summary, toolbar and all-PRD-node context-menu creation, graph-position persistence, pin connection edge creation, pin break edge deletion, and conditional edge create/delete controls.
- Added editor helper APIs for node creation, node movement, node duplication, node deletion cleanup, edge editing, node search, and node display names.
- Added toolbar and graph-command support for copy, paste, duplicate, delete, and zoom-to-fit operations.
- Added selected-node Details editing, breakpoint metadata, expanded debugger snapshots, and Dialogue Tree / Dialogue Database soft-lock metadata.
- Added expression validation helper APIs for editor Details panels and future expression widgets.
- Remaining editor-adjacent polish moves to release hardening: richer live PIE debugger UI and optional expression-authoring widgets.

## M4 Presentation

Status: complete for the v0.12 Phase 3 presentation polish pass.

- Added `UChronicleDialoguePresentationController` as the UI-facing orchestration layer over `UDialogueRunner`.
- Added `UChronicleDialogueWidget`, an abstract UMG base widget that implements `IDialoguePresenter` and exposes Blueprint events/actions.
- Added `UChronicleDialogueDefaultWidget`, a concrete source-built dialogue HUD with typewriter reveal state, choice buttons, Backlog display, Advance, Auto, Skip, Backlog, Back controls, and portrait/full-body image slots.
- Added `UChronicleDialogueChoiceButton` for indexed choice forwarding from generated UMG buttons.
- Added auto advance, skip mode, presentation backlog, rollback sync, choice forwarding, and line-completion presenter hooks.
- Added camera and audio presentation cue tags through `Chronicle.Camera.*` and `Chronicle.Audio.*`.
- Added `UChronicleDialogueCueRouter` and `AChronicleDialogueCueDirector` for Blueprint-friendly camera/audio cue routing.
- Added `AChronicleDialogueDemoActor`, which builds and starts a small runtime demo tree and can create/bind the default HUD without tracking binary sample assets.
- Added automation coverage for backlog, auto, skip, rollback, choice forwarding, camera cue payloads, voice IDs, cue routing, demo HUD defaults, and default widget state.
- Remaining optional showcase work: packaged binary demo map/assets for a public release page.

## M5 Hardening And Release

Status: current release-candidate hardening pass.

- Runtime traversal now uses per-run node and outgoing-edge lookup tables.
- Runtime traversal now includes explicit Random, Jump, SubDialogue, Camera, and Animation node behavior.
- SubDialogue nodes support target-tree entry and return-to-caller flow.
- Camera and Animation nodes emit presentation events with default cue tags.
- Added `UDialogueTrigger` assets and `UDialogueTriggerManager` runtime activation flow with priority selection, condition checks, cooldowns, one-shot consumption, tag activation, and subsystem access.
- Condition expressions compile once into a cached AST, with per-dialogue condition result caching and variable-name tag caching.
- Rollback mementos are limited to player-visible pause points instead of internal node hops.
- The 100-node condition traversal test now enforces a hardened `0.25ms` editor automation budget; the latest UE 5.3 run recorded `0.0557ms` after warm-up.
- Added release notes, release checklist, and a packaging script.
- UE 5.7 smoke and UE 5.3 BuildPlugin packaging pass for this release-candidate pass.
- Remaining before public release: optionally tag `v0.5.0` and attach the package to a GitHub Release.

## M6 Integration And Audit

Status: complete for the v0.9 integration pass.

- Added standard dialogue event tags for quest, game-state, actor animation, battle encounter, and scene load integration.
- Added `UChronicleExampleQuestAdapter`, a source example that binds to `UDialogueRunner::OnDialogueEvent` and rebroadcasts project-facing quest/state delegates.
- Added a variable push helper on the example adapter so outside systems can feed runner variables without owning the runner directly.
- Added editor audit reports for production review, localization readiness, and release checks.
- Added automation coverage for the integration adapter and audit report JSON export.
- UE 5.3 automation now covers 32 Chronicle tests; latest 100-node condition traversal run recorded `0.0986ms`.

## M7 Script Import Workflow

Status: complete for the v0.10 script import pass.

- Added an Excel-friendly CSV script importer that creates Speech nodes, optional Event nodes, and edges from fixed columns.
- Required script columns are `LineID` and `Text`; optional columns cover speaker, emotion, voice, wait time, next line, condition, event tag, payload, and async flag.
- Added `UDialogueImporterBase` for future Articy, Twine, Ren'Py, or project-specific importers.
- Added `UChronicleCsvDialogueImporter` as the default importer implementation.
- Added automation coverage proving imported CSV topology can run through `UDialogueRunner`.
- UE 5.3 automation covered 33 Chronicle tests; latest 100-node condition traversal run recorded `0.2092ms`.

## M8 Editor Pipeline And Debugger Tools

Status: complete for the `v0.11.0-dev` Phase 1 + Phase 2 iteration.

- Added Dialogue Tree Content Browser actions for `Export Chronicle Pipeline Artifacts...` and `Import Chronicle Script CSV...`.
- Added `FChronicleDialoguePipelineExportPaths` and `ExportDialogueTreePipelineArtifacts` for one-call production handoff files.
- Added `FChronicleConditionExpressionValidationResult` and `ValidateConditionExpressionForTree`.
- Expanded `FChronicleDialogueDebuggerSnapshot` with node type, line index, variables, outgoing edges, history, and seen dialogue hashes.
- Added automation coverage for editor pipeline tools and expanded debugger snapshots.
- UE 5.3 automation now covers 34 Chronicle tests; latest 100-node condition traversal run recorded `0.0458ms`.

## M9 Phase 3/4 Finalization

Status: current `v0.12.0-dev` Phase 3 + Phase 4 iteration.

- Added source-first presentation cue routing with `UChronicleDialogueCueRouter`.
- Added `AChronicleDialogueCueDirector` for level camera/audio cue handling.
- Updated `AChronicleDialogueDemoActor` so the demo can create and bind the default HUD at runtime.
- Added `Scripts/RunChronicleValidation.ps1` and a self-hosted GitHub Actions workflow.
- Added final PRD completion matrix documentation.
- UE 5.3 automation now covers 35 Chronicle tests; latest 100-node condition traversal run recorded `0.0844ms`.
