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

- Implement Slate graph editing for `UDialogueTree`.
- Add details customizations, search, breakpoints, and a PIE debugger panel.
- Add soft-lock metadata for Database and Tree editing.

## M4 Presentation

- Add default UMG widgets.
- Add camera, audio, skip, auto, backlog, and rollback presentation flows.
- Add sample map and example content.

## M5 Hardening And Release

- Optimize traversal and memory behavior.
- Validate compatibility across UE 5.3 and UE 5.7.
- Publish release notes and package the plugin.
