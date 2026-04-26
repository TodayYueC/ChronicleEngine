# Chronicle Dialogue Tree Editor

## Current M3 Scope

Double-clicking a `UDialogueTree` asset opens the Chronicle custom asset editor.

The current editor provides:

- a UE `SGraphEditor` view backed by Chronicle `UEdGraph`, `UEdGraphNode`, and `UEdGraphSchema` classes
- graph context menu actions for Speech, Choice, Condition, Event, and Wait node creation
- graph node positions that synchronize back to `FDialogueNode::Position`
- pin connections that create `FDialogueEdge` entries through the Chronicle graph schema
- pin disconnections that remove matching `FDialogueEdge` entries
- link authoring from a source node to a target node with output slot and optional condition expression
- selected-node outgoing edge inspection and deletion
- selected-node Details panel editing through `UChronicleDialogueNodeDetails`
- breakpoint metadata, graph breakpoint markers, and debugger snapshot support
- soft-lock metadata for Dialogue Tree and Dialogue Database assets
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
- `SetDialogueNodeBreakpoint`
- `GetDialogueNodeBreakpoints`
- `CaptureDebuggerSnapshot`
- `AcquireDialogueTreeLock`
- `ReleaseDialogueTreeLock`
- `AcquireDialogueDatabaseLock`
- `ReleaseDialogueDatabaseLock`
- `SearchDialogueNodes`
- `GetNodeTypeDisplayName`

The editor graph layer currently exposes:

- `UChronicleDialogueGraph`
- `UChronicleDialogueGraphNode`
- `UChronicleDialogueGraphSchema`
- `UChronicleDialogueNodeDetails`

Automation tests cover node creation, starter content defaults, root duplication prevention, search, node display names, node movement, edge creation, duplicate edge rejection, conditional edges, edge deletion, graph schema connection validation, graph-to-tree edge synchronization, graph-to-tree position synchronization, pin-link break synchronization, graph context actions, Details panel data application, breakpoints, debugger snapshots, and tree/database soft locks.

## Next Editor Work

M3 is complete. The next editor-facing pass belongs to M4 presentation work and later M5 hardening:

- default UMG presentation widgets
- runtime-facing debug polish in PIE
- release packaging and compatibility hardening
