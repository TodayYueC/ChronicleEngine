# Chronicle Engine Roadmap

## M1 Runtime Vertical Slice

Acceptance:

- The host project compiles with UE 5.3.
- Automation tests pass for variable storage, condition evaluation, dialogue traversal, events, save/load, rollback, and inline tags.
- Dialogue can be represented entirely with data structs and traversed without editor-only dependencies.

## M2 Asset Pipeline

Status: complete for the v0.10 CSV script import pass.

- Stable JSON import/export with schema marker and deterministic node/edge ordering.
- CSV export/import for localization and writer text handoff.
- Stable localization gather keys through `LineID`, with automatic missing/duplicate key repair from `TreeGuid + NodeGuid + LineIndex`.
- Tree/database text gather helpers and localization CSV export/import by stable key.
- Culture-specific voice-table lookup through `UDialogueDatabase::CultureVoiceTables` and `ResolveVoiceTableForCulture`.
- Dialogue-tree validation for missing root, broken edges, duplicate node GUIDs, empty content warnings, and unreachable nodes.
- Dialogue audit reports through `UChronicleDialogueAuditLibrary`, covering node/edge counts, line/choice/word counts, speaker stats, variable usage, broken edges, unreachable nodes, and validation totals.
- CSV script import through `ImportDialogueScriptCsvString`, `ImportDialogueScriptCsvFile`, and `UChronicleCsvDialogueImporter`, creating runnable graph topology from Excel-authored rows.
- Importer extensibility through `UDialogueImporterBase`.
- Remaining pipeline polish moves to v1.0: richer external spreadsheet templates, XLSX helpers, and editor menu commands.

## M3 Editor Experience

Status: complete for the v0.6 editor workflow pass.

- Dialogue Tree assets open in a Chronicle custom asset editor instead of the default details-only view.
- Added a native `SGraphEditor` view backed by Chronicle EdGraph/Node/Schema classes, with search selection, validation summary, toolbar and all-PRD-node context-menu creation, graph-position persistence, pin connection edge creation, pin break edge deletion, and conditional edge create/delete controls.
- Added editor helper APIs for node creation, node movement, node duplication, node deletion cleanup, edge editing, node search, and node display names.
- Added toolbar and graph-command support for copy, paste, duplicate, delete, and zoom-to-fit operations.
- Added selected-node Details editing, breakpoint metadata, debugger snapshot support, and Dialogue Tree / Dialogue Database soft-lock metadata.
- Remaining editor-adjacent polish moves to localization/audit/release hardening: richer expression widgets, live PIE debugger polish, localization gather commands, and audit reports.

## M4 Presentation

Status: complete for the v0.7 default UMG source-widget pass.

- Added `UChronicleDialoguePresentationController` as the UI-facing orchestration layer over `UDialogueRunner`.
- Added `UChronicleDialogueWidget`, an abstract UMG base widget that implements `IDialoguePresenter` and exposes Blueprint events/actions.
- Added `UChronicleDialogueDefaultWidget`, a concrete source-built dialogue HUD with typewriter reveal state, choice buttons, Backlog display, Advance, Auto, Skip, Backlog, Back controls, and portrait/full-body image slots.
- Added `UChronicleDialogueChoiceButton` for indexed choice forwarding from generated UMG buttons.
- Added auto advance, skip mode, presentation backlog, rollback sync, choice forwarding, and line-completion presenter hooks.
- Added camera and audio presentation cue tags through `Chronicle.Camera.*` and `Chronicle.Audio.*`.
- Added `AChronicleDialogueDemoActor`, which builds and starts a small runtime demo tree without tracking binary sample assets.
- Added automation coverage for backlog, auto, skip, rollback, choice forwarding, camera cue payloads, voice IDs, and default widget state.
- Remaining release polish moves to M5: packaged demo content, optional binary showcase map, and broader compatibility hardening.

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

Status: current v0.10 script import pass.

- Added an Excel-friendly CSV script importer that creates Speech nodes, optional Event nodes, and edges from fixed columns.
- Required script columns are `LineID` and `Text`; optional columns cover speaker, emotion, voice, wait time, next line, condition, event tag, payload, and async flag.
- Added `UDialogueImporterBase` for future Articy, Twine, Ren'Py, or project-specific importers.
- Added `UChronicleCsvDialogueImporter` as the default importer implementation.
- Added automation coverage proving imported CSV topology can run through `UDialogueRunner`.
- UE 5.3 automation now covers 33 Chronicle tests; latest 100-node condition traversal run recorded `0.2092ms`.
