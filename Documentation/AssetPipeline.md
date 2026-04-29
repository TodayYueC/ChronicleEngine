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

## Localization Gather Workflow

The v0.8 localization pass adds key-stable gather and translation import helpers:

- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`

Gathered localization entries use `LineID` as the stable key. If a dialogue line has no `LineID`, or if duplicate `LineID` values are found, the helper repairs the key from `TreeGuid + NodeGuid + LineIndex`. This keeps keys stable when the source text changes.

Localization CSV columns:

- `Namespace`
- `Key`
- `Culture`
- `TreeGuid`
- `NodeGuid`
- `LineIndex`
- `LineID`
- `SpeakerTag`
- `SourceText`
- `TranslatedText`
- `ContextComment`

`ImportLocalizationCsvToTree` updates existing lines by `Key` / `LineID`, falling back to `NodeGuid + LineIndex`. Empty `TranslatedText` cells are ignored so partially translated sheets do not overwrite source text.

`UDialogueDatabase` also exposes `CultureVoiceTables` and `ResolveVoiceTableForCulture`. A culture-specific table is returned when present; otherwise the database-level default `VoiceTable` is used.

## Validation

The validation helper reports:

- invalid tree or root GUID
- duplicate node GUIDs
- broken edge endpoints
- unreachable nodes
- empty speech or choice nodes
- event nodes without an event tag

Validation returns `false` when any error-level issue is present. Warning-level issues are reported but do not fail validation.
