# Chronicle Dialogue Tree Editor

## Current M3 Scope

Double-clicking a `UDialogueTree` asset opens the Chronicle custom asset editor.

The current editor provides:

- a Slate canvas that positions node cards from each node's stored `Position`
- drag-to-move node cards that write back to `FDialogueNode::Position`
- visual lines for currently visible `FDialogueEdge` links
- link authoring from a source node to a target node with output slot and optional condition expression
- selected-node outgoing edge inspection and deletion
- node color coding for Root, Speech, Choice, Condition, Event, and Wait nodes
- basic node creation buttons for Speech, Choice, and Condition nodes
- search filtering across node type, GUID, line text, speaker tag, choice text, conditions, and event tags
- validation summary using the same tree validation helper as the asset pipeline

This is an editor foundation, not the final Unreal `SGraphEditor` implementation. It is intentionally wired through reusable editor helper APIs so the later GraphEditor implementation can keep the same behavior.

## Editor Helper API

`UChronicleDialogueEditorLibrary` currently exposes:

- `AddDialogueNode`
- `SetDialogueNodePosition`
- `AddDialogueEdge`
- `RemoveDialogueEdge`
- `SearchDialogueNodes`
- `GetNodeTypeDisplayName`

Automation tests cover node creation, starter content defaults, root duplication prevention, search, node display names, node movement, edge creation, duplicate edge rejection, conditional edges, and edge deletion.

## Next Editor Work

The next pass should add:

- true `SGraphEditor` pin widgets and schema-backed connection validation
- Details panel customization for selected node payloads
- breakpoint metadata and PIE debugger handoff
