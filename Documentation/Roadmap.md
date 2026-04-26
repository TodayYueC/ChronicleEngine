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

- Add default UMG widgets.
- Add camera, audio, skip, auto, backlog, and rollback presentation flows.
- Add sample map and example content.

## M5 Hardening And Release

- Optimize traversal and memory behavior.
- Validate compatibility across UE 5.3 and UE 5.7.
- Publish release notes and package the plugin.
