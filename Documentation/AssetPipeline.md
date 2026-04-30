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

Import updates existing lines by `NodeGuid` and `LineIndex`. When `TranslatedText` is filled, it replaces the line text; otherwise `SourceText` is used.

## CSV Script Import Workflow

The v0.10 script import pass adds an Excel-friendly CSV workflow that can create graph topology, not only update existing line text.

Use:

- `ImportDialogueScriptCsvString`
- `ImportDialogueScriptCsvFile`
- `UChronicleCsvDialogueImporter`

Required columns:

- `LineID`
- `Text`

Optional columns:

- `SpeakerTag` or `Speaker`
- `EmotionTag`
- `VoiceID`
- `WaitTime`
- `NextLineID`, `NextLine`, or `TargetLineID`
- `ConditionExpression` or `Condition`
- `EventTag`
- `EventPayload`
- `bEventIsAsync`, `EventAsync`, or `Async`

Each CSV row creates a Speech node with one `FDialogueLine`. `NextLineID` creates the outgoing edge to another imported or existing line. `ConditionExpression` is copied to the outgoing edge. When `EventTag` is set, the importer inserts an Event node after the Speech node and parses `EventPayload` from semicolon-separated `Key=Value` pairs.

`UDialogueImporterBase` is the extension point for project-specific importers. `UChronicleCsvDialogueImporter` implements the built-in script CSV importer and can replace the target tree or append new rows.

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

## Audit Reports

The v0.9 audit pass adds `UChronicleDialogueAuditLibrary` for production checks before recording, localization handoff, or release.

Blueprint/C++ helpers:

- `BuildDialogueAuditReport`
- `ExportDialogueAuditReportToJsonString`
- `ExportDialogueAuditReportForTreeToJsonString`

The report includes:

- node and edge counts
- speech line count
- choice count
- total word count
- per-speaker line and word counts
- variable references found in conditions and event payloads
- broken edge count
- unreachable node count
- warning and error totals from the validation helper
- the full validation issue list

Variable scanning currently recognizes Gameplay Tag style references such as `Chronicle.Variable.Score` in node conditions, choice visibility conditions, edge conditions, and event payload strings. This keeps the first audit pass source-control friendly and easy to export as JSON.
