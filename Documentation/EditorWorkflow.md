# Chronicle Dialogue Tree Editor

## Current M3 Scope

Double-clicking a `UDialogueTree` asset opens the Chronicle custom asset editor.

The current editor provides:

- a Slate canvas that positions node cards from each node's stored `Position`
- node color coding for Root, Speech, Choice, Condition, Event, and Wait nodes
- basic node creation buttons for Speech, Choice, and Condition nodes
- search filtering across node type, GUID, line text, speaker tag, choice text, conditions, and event tags
- validation summary using the same tree validation helper as the asset pipeline

This is an editor foundation, not the final pin-based graph. It is intentionally wired through reusable editor helper APIs so the later GraphEditor implementation can keep the same behavior.

## Editor Helper API

`UChronicleDialogueEditorLibrary` currently exposes:

- `AddDialogueNode`
- `SearchDialogueNodes`
- `GetNodeTypeDisplayName`

Automation tests cover node creation, starter content defaults, root duplication prevention, search, and node display names.

## Next Editor Work

The next pass should add:

- true `SGraphEditor` or graph-like pin connection behavior
- node dragging that writes back to `FDialogueNode::Position`
- edge creation/deletion UI
- Details panel customization for selected node payloads
- breakpoint metadata and PIE debugger handoff

