# Chronicle Dialogue Tree Editor

## Current M3 Scope

Double-clicking a `UDialogueTree` asset opens the Chronicle custom asset editor.

The current editor provides:

- a UE `SGraphEditor` view backed by Chronicle `UEdGraph`, `UEdGraphNode`, and `UEdGraphSchema` classes
- graph node positions that synchronize back to `FDialogueNode::Position`
- pin connections that create `FDialogueEdge` entries through the Chronicle graph schema
- pin disconnections that remove matching `FDialogueEdge` entries
- link authoring from a source node to a target node with output slot and optional condition expression
- selected-node outgoing edge inspection and deletion
- node color coding for Root, Speech, Choice, Condition, Event, and Wait nodes
- basic node creation buttons for Speech, Choice, and Condition nodes
- search filtering across node type, GUID, line text, speaker tag, choice text, conditions, and event tags
- validation summary using the same tree validation helper as the asset pipeline

The graph is still a transient editor model. `UDialogueTree` remains the durable asset format, while the EdGraph layer provides native Unreal graph interaction and schema validation.

## Editor Helper API

`UChronicleDialogueEditorLibrary` currently exposes:

- `AddDialogueNode`
- `SetDialogueNodePosition`
- `AddDialogueEdge`
- `RemoveDialogueEdge`
- `SearchDialogueNodes`
- `GetNodeTypeDisplayName`

The editor graph layer currently exposes:

- `UChronicleDialogueGraph`
- `UChronicleDialogueGraphNode`
- `UChronicleDialogueGraphSchema`

Automation tests cover node creation, starter content defaults, root duplication prevention, search, node display names, node movement, edge creation, duplicate edge rejection, conditional edges, edge deletion, graph schema connection validation, graph-to-tree edge synchronization, graph-to-tree position synchronization, and pin-link break synchronization.

## Next Editor Work

The next pass should add:

- Details panel customization for selected node payloads
- graph context menu actions for node creation
- breakpoint metadata and PIE debugger handoff
