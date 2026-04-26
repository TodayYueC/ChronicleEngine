# Chronicle Engine Roadmap

## M1 Runtime Vertical Slice

Acceptance:

- The host project compiles with UE 5.3.
- Automation tests pass for variable storage, condition evaluation, dialogue traversal, events, save/load, rollback, and inline tags.
- Dialogue can be represented entirely with data structs and traversed without editor-only dependencies.

## M2 Asset Pipeline

Status: current implementation pass.

- Stable JSON import/export with schema marker and deterministic node/edge ordering.
- CSV export/import for localization and writer text handoff.
- Dialogue-tree validation for missing root, broken edges, duplicate node GUIDs, empty content warnings, and unreachable nodes.
- Remaining: localization gather helpers and richer external spreadsheet creation workflows.

## M3 Editor Experience

Status: complete for the planned M3 scope.

- Dialogue Tree assets open in a Chronicle custom asset editor instead of the default details-only view.
- Added a native `SGraphEditor` view backed by Chronicle EdGraph/Node/Schema classes, with search selection, validation summary, toolbar and context-menu node creation, graph-position persistence, pin connection edge creation, pin break edge deletion, and conditional edge create/delete controls.
- Added editor helper APIs for node creation, node movement, edge editing, node search, and node display names.
- Added selected-node Details editing, breakpoint metadata, debugger snapshot support, and Dialogue Tree / Dialogue Database soft-lock metadata.
- Remaining editor polish moves to M4/M5: richer presentation widgets, PIE debug UX polish, and release hardening.

## M4 Presentation

Status: complete for the planned source-first M4 scope.

- Added `UChronicleDialoguePresentationController` as the UI-facing orchestration layer over `UDialogueRunner`.
- Added `UChronicleDialogueWidget`, an abstract UMG base widget that implements `IDialoguePresenter` and exposes Blueprint events/actions.
- Added auto advance, skip mode, presentation backlog, rollback sync, choice forwarding, and line-completion presenter hooks.
- Added camera and audio presentation cue tags through `Chronicle.Camera.*` and `Chronicle.Audio.*`.
- Added `AChronicleDialogueDemoActor`, which builds and starts a small runtime demo tree without tracking binary sample assets.
- Added automation coverage for backlog, auto, skip, rollback, choice forwarding, camera cue payloads, and voice IDs.
- Remaining release polish moves to M5: packaged demo content, optional binary showcase map, and broader compatibility hardening.

## M5 Hardening And Release

Status: current release-candidate hardening pass.

- Runtime traversal now uses per-run node and outgoing-edge lookup tables.
- Condition expressions compile once into a cached AST, with per-dialogue condition result caching and variable-name tag caching.
- Rollback mementos are limited to player-visible pause points instead of internal node hops.
- The 100-node condition traversal test now enforces a hardened `0.25ms` editor automation budget; the latest UE 5.3 run recorded `0.1181ms`.
- Added release notes, release checklist, and a packaging script.
- UE 5.7 smoke and UE 5.3 BuildPlugin packaging pass for this release-candidate pass.
- Remaining before public release: optionally tag `v0.5.0` and attach the package to a GitHub Release.
