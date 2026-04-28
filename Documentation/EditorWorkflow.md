# Chronicle Dialogue Tree Editor

## Current V0.6 Editor Scope

Double-clicking a `UDialogueTree` asset opens the Chronicle custom asset editor.

The current editor provides:

- a UE `SGraphEditor` view backed by Chronicle `UEdGraph`, `UEdGraphNode`, and `UEdGraphSchema` classes
- graph context menu actions for every PRD node type: Root, Speech, Choice, Condition, Event, Wait, Random, Jump, Sequence, SubDialogue, Camera, and Animation
- graph node positions that synchronize back to `FDialogueNode::Position`
- pin connections that create `FDialogueEdge` entries through the Chronicle graph schema
- pin disconnections that remove matching `FDialogueEdge` entries
- toolbar and graph-command support for copying, pasting, duplicating, and deleting selected nodes
- internal selected-edge duplication when a group of nodes is duplicated
- automatic cleanup of attached edges and breakpoint metadata when nodes are deleted
- zoom-to-fit controls for navigating larger trees
- link authoring from a source node to a target node with output slot and optional condition expression
- selected-node outgoing edge inspection and deletion
- selected-node Details panel editing through `UChronicleDialogueNodeDetails`
- breakpoint metadata, graph breakpoint markers, and debugger snapshot support
- soft-lock metadata for Dialogue Tree and Dialogue Database assets
- node color coding for all PRD node types
- basic node creation buttons for Speech, Choice, and Condition nodes
- search filtering across node type, GUID, line text, speaker tag, choice text, conditions, and event tags
- validation summary using the same tree validation helper as the asset pipeline

The graph is still a transient editor model. `UDialogueTree` remains the durable asset format, while the EdGraph layer provides native Unreal graph interaction and schema validation.

## Editor Helper API

`UChronicleDialogueEditorLibrary` currently exposes:

- `AddDialogueNode`
- `SetDialogueNodePosition`
- `RemoveDialogueNodes`
- `DuplicateDialogueNodes`
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

Automation tests cover node creation, starter content defaults, root duplication prevention, search, node display names, node movement, node duplication, node deletion cleanup, edge creation, duplicate edge rejection, conditional edges, edge deletion, graph schema connection validation, graph-to-tree edge synchronization, graph-to-tree position synchronization, pin-link break synchronization, all PRD graph context actions, Details panel data application, breakpoints, debugger snapshots, and tree/database soft locks.

## Next Editor Work

The editor is now complete for the v0.6 PRD workflow pass. Remaining editor-adjacent work moves into localization, audit, and release hardening:

- richer expression-authoring widgets for condition fields
- PIE debugger live-run polish
- localization gather commands and audit reports
- release packaging and compatibility hardening
