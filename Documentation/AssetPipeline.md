# Chronicle Asset Pipeline

## JSON Tree Workflow

`UChronicleDialogueJsonLibrary` supports exporting and importing `UDialogueTree` as JSON.

The M2 JSON format includes:

- `Format`: `ChronicleDialogueTree`
- `SchemaVersion`: currently `1`
- `TreeGuid`
- `RootNodeGuid`
- sorted `Nodes`
- sorted `Edges`
- sorted `Variables`

Nodes and edges are sorted before export so repeated exports of unchanged data are stable for Git review.

## CSV Dialogue-Line Workflow

CSV export is intended for writer and localization handoff. It exports only dialogue lines, not graph topology.

Columns:

- `NodeGuid`
- `LineIndex`
- `LineID`
- `SpeakerTag`
- `SourceText`
- `TranslatedText`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `ContextComment`

Import updates existing lines by `NodeGuid` and `LineIndex`. When `TranslatedText` is filled, it replaces the line text; otherwise `SourceText` is used. Graph nodes and edges are not created by CSV import in M2.

## Validation

The validation helper reports:

- invalid tree or root GUID
- duplicate node GUIDs
- broken edge endpoints
- unreachable nodes
- empty speech or choice nodes
- event nodes without an event tag

Validation returns `false` when any error-level issue is present. Warning-level issues are reported but do not fail validation.

