# Chronicle Dialogue Tree Editor

## Current V0.11 Editor Scope

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
- breakpoint metadata and graph breakpoint markers
- debugger snapshots with current node type, line index, variable values/scopes, history, seen-line hashes, and outgoing edge condition results
- soft-lock metadata for Dialogue Tree and Dialogue Database assets
- Dialogue Tree Content Browser actions for pipeline export and script CSV import
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
- `ValidateConditionExpressionForTree`
- `BuildDefaultDialogueTreeExportPaths`
- `ExportDialogueTreePipelineArtifacts`
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

Automation tests cover node creation, starter content defaults, root duplication prevention, search, node display names, node movement, node duplication, node deletion cleanup, edge creation, duplicate edge rejection, conditional edges, edge deletion, graph schema connection validation, graph-to-tree edge synchronization, graph-to-tree position synchronization, pin-link break synchronization, all PRD graph context actions, Details panel data application, breakpoints, expanded debugger snapshots, expression validation, pipeline artifact export, and tree/database soft locks.

## Next Editor Work

The editor is complete for the current Phase 1 and Phase 2 iteration. Remaining editor-adjacent work moves into release hardening:

- richer expression-authoring widgets for condition fields
- PIE debugger live-run polish
- release packaging and compatibility hardening
