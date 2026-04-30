# Chronicle Engine Step-By-Step Usage Tutorial

This document is the hands-on companion to the main README. It assumes you are starting from the source repository and want to operate Chronicle Engine inside Unreal Editor, create dialogue assets, test them, connect UI and gameplay events, export pipeline files, and package the plugin.

Jump links:

- [English Step-By-Step Tutorial](#english-step-by-step-tutorial)
- [中文超详细逐步使用教程](#chinese-step-by-step-tutorial)

---

<a id="english-step-by-step-tutorial"></a>

# English Step-By-Step Tutorial

## 1. What You Will Build

By the end of this tutorial you will have:

1. Opened the host project.
2. Verified that the Chronicle Engine plugin is enabled.
3. Created a Dialogue Database, Speaker Profile, and Dialogue Tree.
4. Built a small playable dialogue graph with speech, choices, conditions, and events.
5. Played the dialogue through the presentation controller and default widget.
6. Exported JSON, CSV, localization CSV, and audit files.
7. Run automated validation.
8. Packaged the plugin for use in another Unreal project.

The tutorial uses the bundled host project:

```text
R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject
```

If your repository is in another folder, replace that path with your own checkout path.

## 2. Prerequisites

Before you begin, check these items.

1. Install Unreal Engine 5.3.
2. Make sure this path exists, or adjust commands later:

```text
R:\UE\UE_5.3
```

3. Install Visual Studio with C++ game development support.
4. Make sure the repository contains these files and folders:

```text
ChronicleHost.uproject
Plugins\ChronicleEngine\ChronicleEngine.uplugin
Plugins\ChronicleEngine\Source\ChronicleEngine
Plugins\ChronicleEngine\Source\ChronicleEngineEditor
Plugins\ChronicleEngine\Source\ChronicleEngineTests
Scripts\RunChronicleValidation.ps1
Scripts\PackagePlugin.ps1
```

5. Optional compatibility smoke test: install Unreal Engine 5.7 at:

```text
R:\UE\UE_5.7
```

UE 5.3 is the primary baseline. UE 5.7 is used only for compatibility smoke testing.

## 3. First Open and Rebuild

### 3.1 Open the project

1. Double-click:

```text
ChronicleHost.uproject
```

2. If Unreal shows this message:

```text
The following modules are missing or built with a different engine version:
ChronicleHost
ChronicleEngine
ChronicleEngineEditor
ChronicleEngineTests
Would you like to rebuild them now?
```

3. Click **Yes**.
4. Wait for Unreal Build Tool to finish.
5. If the build succeeds, Unreal Editor opens.

### 3.2 If the rebuild dialog fails

Use a manual build from PowerShell.

1. Close Unreal Editor.
2. Open PowerShell in the repository root:

```text
R:\AI_Agent\Codex\JRPGtalking
```

3. Run:

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

4. When the command finishes successfully, double-click `ChronicleHost.uproject` again.

### 3.3 Expected result

The editor should open with the Chronicle Engine plugin loaded. The first launch may take longer because Unreal generates project files, intermediate files, and binaries.

## 4. Verify the Plugin Is Enabled

1. In Unreal Editor, open **Edit > Plugins**.
2. Search for:

```text
Chronicle Engine
```

3. Confirm the plugin is enabled.
4. If it is disabled, enable it and restart the editor.
5. After restart, open the Content Browser and right-click in an empty folder.
6. Search the asset creation menu for these asset types:

```text
Dialogue Tree
Dialogue Database
Speaker Profile
Dialogue Trigger
```

If these asset types appear, the editor module is active.

## 5. Recommended Content Folder Layout

Create a clean folder structure before authoring assets.

1. Open the Content Browser.
2. Go to the project `Content` root.
3. Create this folder:

```text
Content/Chronicle
```

4. Inside it, create:

```text
Content/Chronicle/Dialogues
Content/Chronicle/Speakers
Content/Chronicle/Databases
Content/Chronicle/Triggers
Content/Chronicle/UI
Content/Chronicle/Exports
```

Suggested naming convention:

```text
DT_Intro
DB_Chronicle
SP_Alice
TR_IntroInteract
WBP_ChronicleDialogue
```

Chronicle Engine does not require these exact names, but consistent prefixes make large dialogue libraries easier to search.

## 6. Verify Gameplay Tags

Chronicle Engine uses Gameplay Tags for speakers, events, variables, triggers, camera cues, animation cues, and audio cues.

The host project already includes sample tags in:

```text
Config/DefaultGameplayTags.ini
```

Useful sample tags:

```text
Chronicle.Speaker.Alice
Chronicle.Variable.Score
Chronicle.Variable.Flag
Chronicle.Event.Quest.Start
Chronicle.Event.Quest.Update
Chronicle.Event.Quest.Complete
Chronicle.Camera.Cut
Chronicle.Camera.Blend
Chronicle.Animation.Play
Chronicle.Audio.PlayVoice
Chronicle.Trigger.Test
```

To view or add tags in the editor:

1. Open **Edit > Project Settings**.
2. Search for **Gameplay Tags**.
3. Open **Gameplay Tags**.
4. Confirm that the `Chronicle.*` tags are present.
5. Add project-specific tags under the same root if needed.

Recommended project tag roots:

```text
Chronicle.Speaker.*
Chronicle.Variable.*
Chronicle.Event.*
Chronicle.Trigger.*
Chronicle.Camera.*
Chronicle.Animation.*
Chronicle.Audio.*
Chronicle.Quest.*
```

## 7. Create a Speaker Profile

The Speaker Profile stores display name, portrait sets, full-body art, color, and default position.

1. Open:

```text
Content/Chronicle/Speakers
```

2. Right-click in the empty area.
3. Search for:

```text
Speaker Profile
```

4. Create the asset.
5. Name it:

```text
SP_Alice
```

6. Double-click `SP_Alice`.
7. Fill the fields:

| Field | Example |
|---|---|
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Display Name | `Alice` |
| Text Color | White or a character-specific color |
| Default Position | `Left` |
| Portrait Set | Optional texture map, for example `Neutral`, `Happy`, `Sad` |
| Full Body Set | Optional texture map |
| Voice Set | Optional Data Table |

8. Save the asset.

Expected result: `SP_Alice` can now be referenced by a Dialogue Database and by dialogue lines that use `Chronicle.Speaker.Alice`.

## 8. Create a Dialogue Database

The Dialogue Database is the central registry for speakers, global variables, dialogue trees, localization settings, and voice tables.

1. Open:

```text
Content/Chronicle/Databases
```

2. Right-click in the empty area.
3. Search for:

```text
Dialogue Database
```

4. Create the asset.
5. Name it:

```text
DB_Chronicle
```

6. Double-click `DB_Chronicle`.
7. Add `SP_Alice` to **Speaker Profiles**.
8. Add global variables under **Global Variables**.

Example global variable:

| Field | Value |
|---|---|
| Variable Tag | `Chronicle.Variable.Score` |
| Scope | `Global` |
| Default Value Type | `Int32` |
| Int Value | `0` |
| Display Name | `Score` |

Example boolean variable:

| Field | Value |
|---|---|
| Variable Tag | `Chronicle.Variable.Flag` |
| Scope | `Global` |
| Default Value Type | `Bool` |
| Bool Value | `false` |
| Display Name | `Flag` |

9. Under **Localization Settings**, set:

| Field | Example |
|---|---|
| Namespace | `Project.Dialogue` |
| Target Cultures | `en`, `zh-Hans` |
| Voice Table Path | Optional |

10. Save the asset.

## 9. Create Your First Dialogue Tree

1. Open:

```text
Content/Chronicle/Dialogues
```

2. Right-click in the empty area.
3. Search for:

```text
Dialogue Tree
```

4. Create the asset.
5. Name it:

```text
DT_Intro
```

6. Double-click `DT_Intro`.

Expected result: the Chronicle Dialogue Tree editor opens. A new tree contains a Root node by default.

## 10. If the Dialogue Tree Editor Opens Too Small

If you see only a tiny editable area near the top-left corner:

1. Drag the editor tab out and maximize it, or dock it into the main editor area.
2. In the Dialogue Tree editor toolbar, use **Zoom To Fit** if available.
3. In Unreal Editor, open **Window > Load Layout > Default Editor Layout** if the surrounding editor layout is broken.
4. Close and reopen `DT_Intro`.
5. If the layout is still incorrect, close Unreal and delete only local editor layout/cache files under:

```text
Saved/
Intermediate/
```

Do not delete `Content/` or `Plugins/ChronicleEngine/Source/`.

Expected result: the custom graph editor should occupy the full asset editor area, and nodes should be editable in a normal graph workspace.

## 11. Build a Minimal Speech Graph

This section creates:

```text
Root -> Speech
```

1. Open `DT_Intro`.
2. Right-click in the graph area.
3. Choose or search:

```text
Speech
```

4. Create a Speech node to the right of Root.
5. Select the Speech node.
6. In the Details panel, add one entry to **Lines**.
7. Fill the line:

| Field | Value |
|---|---|
| Line ID | `Intro_001` |
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Text | `Welcome to Chronicle Engine.` |
| Emotion Tag | Optional |
| Voice ID | `VO_Intro_001` |
| Wait Time | `-1.0` |

8. Connect the Root node output to the Speech node input.
9. Save the asset.

Expected result: the tree has a Root node connected to a Speech node.

## 12. Add Multiple Lines to One Speech Node

1. Select the Speech node.
2. In **Lines**, add a second entry.
3. Fill:

| Field | Value |
|---|---|
| Line ID | `Intro_002` |
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Text | `This second line appears after the player advances.` |
| Voice ID | `VO_Intro_002` |

4. Save.

Runtime behavior:

1. The first line broadcasts `OnLineStarted`.
2. The player calls `Advance`.
3. The second line broadcasts `OnLineStarted`.
4. The next `Advance` follows the outgoing edge.

## 13. Add a Choice Node

This section creates:

```text
Root -> Speech -> Choice
Choice slot 0 -> Speech
Choice slot 1 -> Speech
```

1. Right-click in the graph.
2. Create a **Choice** node to the right of the first Speech node.
3. Select the Choice node.
4. In **Choices**, add two entries.
5. Fill choice 0:

| Field | Value |
|---|---|
| Text | `Tell me more.` |
| Visibility Condition | leave empty |
| Target Output Index | leave default unless you author manually |

6. Fill choice 1:

| Field | Value |
|---|---|
| Text | `Skip the details.` |
| Visibility Condition | leave empty |
| Target Output Index | leave default unless you author manually |

7. Create two Speech nodes after the Choice node.
8. Name their line IDs:

```text
Intro_More_001
Intro_Short_001
```

9. Add text to each branch.
10. Connect the first Speech node to the Choice node.
11. Connect Choice output slot 0 to the `Intro_More_001` Speech node.
12. Connect Choice output slot 1 to the `Intro_Short_001` Speech node.
13. Save.

Runtime behavior:

1. The runner enters the Choice node.
2. It evaluates each choice visibility condition.
3. It broadcasts `OnChoicesPresented`.
4. The player chooses an index.
5. `SelectChoice(0)` follows output slot 0.
6. `SelectChoice(1)` follows output slot 1.

## 14. Add a Choice Visibility Condition

Choice visibility conditions use the Chronicle condition parser.

Supported expression features:

```text
variables
parentheses
!
&&
||
==
!=
<
<=
>
>=
number literals
string literals
boolean literals
```

Example expressions:

```text
Chronicle.Variable.Score >= 10
Chronicle.Variable.Flag == true
(Chronicle.Variable.Score >= 10) && (Chronicle.Variable.Flag == true)
Chronicle.Variable.Name == "Alice"
```

To hide a choice until score is at least 10:

1. Select the Choice node.
2. Open **Choices**.
3. Pick the choice you want to gate.
4. Set **Visibility Condition**:

```text
Chronicle.Variable.Score >= 10
```

5. Save.

Expected result: that choice appears only when the runner variable `Chronicle.Variable.Score` is 10 or higher.

## 15. Set Variables at Runtime

Variables can be controlled from Blueprint or C++ through the runner.

Blueprint operation:

1. Get the Game Instance.
2. Get `Chronicle Dialogue Subsystem`.
3. Call `Get Dialogue Runner`.
4. Create an `FVariableValue`.
5. Call `Set Variable`.
6. Pass:

| Pin | Value |
|---|---|
| Tag | `Chronicle.Variable.Score` |
| Value Type | `Int32` |
| Int Value | `10` |
| Scope | `Global` |

C++ example:

```cpp
UChronicleDialogueSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->SetVariable(
    FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Variable.Score")),
    FVariableValue::MakeInt(10),
    EChronicleVariableScope::Global);
```

## 16. Add a Condition Node

A Condition node chooses an outgoing branch based on edge conditions.

Example graph:

```text
Speech -> Condition
Condition slot 0 -> HighScoreSpeech
Condition slot 1 -> LowScoreSpeech
```

1. Create a **Condition** node after a Speech node.
2. Create two Speech nodes after the Condition node.
3. Connect Speech to Condition.
4. Connect Condition output slot 0 to the high-score Speech node.
5. Set the edge condition for slot 0:

```text
Chronicle.Variable.Score >= 10
```

6. Connect Condition output slot 1 to the low-score Speech node.
7. Set the edge condition for slot 1:

```text
Chronicle.Variable.Score < 10
```

8. Save.

Expected result: the runner follows the first passing condition branch.

## 17. Add an Event Node

Event nodes send GameplayTag events and payload data to your game systems.

Example graph:

```text
Speech -> Event -> Speech
```

1. Create an **Event** node.
2. Select it.
3. Set **Event Tag**:

```text
Chronicle.Event.Quest.Start
```

4. Add payload entries:

| Key | Value |
|---|---|
| QuestTag | `Chronicle.Quest.Main` |
| Step | `IntroStarted` |

5. Leave **bEventIsAsync** disabled for a synchronous event.
6. Connect the incoming Speech node to the Event node.
7. Connect the Event node to the next Speech node.
8. Save.

Runtime behavior:

1. The runner enters the Event node.
2. It broadcasts `OnDialogueEvent`.
3. The presentation controller broadcasts `OnPresentationEvent`.
4. Synchronous events continue immediately.

## 18. Add an Async Event

Use async events when dialogue must wait for another system, such as loading a scene, playing a timeline, entering battle, or waiting for an animation.

1. Select an Event node.
2. Enable:

```text
bEventIsAsync
```

3. Set an event tag, for example:

```text
Chronicle.Event.Scene.Load
```

4. Add payload:

| Key | Value |
|---|---|
| Scene | `VillageNight` |

5. In your gameplay system, listen for `OnDialogueEvent`.
6. Start the async work.
7. When the async work finishes, call:

```cpp
Runner->NotifyEventComplete(FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Event.Scene.Load")));
```

Expected result: the runner pauses in `WaitingForEvent` until `NotifyEventComplete` is called.

## 19. Add Camera and Animation Cues

Chronicle Engine treats camera and animation cues as event-style presentation cues.

Camera cut example:

| Field | Value |
|---|---|
| Node Type | `Camera` or Event-style camera cue |
| Event Tag | `Chronicle.Camera.Cut` |
| Payload `Shot` | `IntroCloseup` |
| Payload `BlendTime` | `0.25` |

Animation example:

| Field | Value |
|---|---|
| Node Type | `Animation` or Event-style animation cue |
| Event Tag | `Chronicle.Animation.Play` |
| Payload `Actor` | `Alice` |
| Payload `Animation` | `Wave` |

Connect your own camera manager, sequencer controller, or animation system to `OnPresentationEvent` or `OnDialogueEvent`, read the tag and payload, then execute the cue.

## 20. Add Wait, Random, Jump, Sequence, and Sub-Dialogue Nodes

Chronicle supports all PRD node types in source form.

Use them as follows:

| Node Type | Typical Use |
|---|---|
| Wait | Pause before continuing. Set `WaitTime`. |
| Random | Select one outgoing branch using edge `Weight`. |
| Jump | Jump to a target entry node or target tree. |
| Sequence | Traverse ordered outgoing slots. |
| SubDialogue | Enter another Dialogue Tree, then optionally return. |
| Camera | Emit camera cue data. |
| Animation | Emit animation cue data. |

For Sub-Dialogue:

1. Create another tree, for example `DT_ShopGreeting`.
2. Add it to `DB_Chronicle`.
3. In the SubDialogue node, set **Target Tree** to `DT_ShopGreeting`.
4. Set **Target Entry Node** if you use named entries.
5. Enable **bReturnToNextNodeOnSubDialogueEnd** if dialogue should return to the parent tree.

## 21. Add the Dialogue Tree to the Database

1. Open `DB_Chronicle`.
2. Add `DT_Intro` to **Dialogue Trees**.
3. Save.

This allows database-based lookup, validation, localization gather, and runtime initialization.

## 22. Start Dialogue from Blueprint

The recommended Blueprint entry point is `UChronicleDialogueSubsystem`.

### 22.1 Level Blueprint setup

1. Open the map where you want to test dialogue.
2. Open **Blueprints > Open Level Blueprint**.
3. On `BeginPlay`, add:

```text
Get Game Instance
Get Subsystem (Chronicle Dialogue Subsystem)
Initialize Dialogue Database
Get Presentation Controller
Get Dialogue Runner
Bind Runner
Start Dialogue
```

4. Set `Initialize Dialogue Database` input to `DB_Chronicle`.
5. Set `Start Dialogue` tree input to `DT_Intro`.
6. Leave Entry Node as `None` for the root entry.
7. Save and Play.

Expected result: the dialogue starts when the level begins.

### 22.2 Add manual input

To advance dialogue manually:

1. In Level Blueprint or Player Controller Blueprint, listen for an input action.
2. Get `Chronicle Dialogue Subsystem`.
3. Get `Presentation Controller`.
4. Call:

```text
Advance
```

To select a choice:

1. Call:

```text
Select Choice
```

2. Pass `0` for the first visible choice, `1` for the second visible choice, and so on.

## 23. Start Dialogue from C++

Add module dependencies in your game module if needed:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "ChronicleEngine",
    "GameplayTags",
    "UMG"
});
```

Example C++ setup:

```cpp
#include "Runtime/ChronicleDialogueSubsystem.h"
#include "Runtime/DialogueRunner.h"
#include "Presentation/ChronicleDialoguePresentationController.h"

void AMyDialogueStarter::StartIntroDialogue()
{
    UChronicleDialogueSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UChronicleDialogueSubsystem>();
    if (!Subsystem)
    {
        return;
    }

    Subsystem->InitializeDialogueDatabase(DialogueDatabase);

    UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
    UChronicleDialoguePresentationController* Controller = Subsystem->GetPresentationController();
    Controller->BindRunner(Runner);
    Controller->StartDialogue(IntroTree);
}
```

## 24. Use the Default Widget

Chronicle Engine includes a source-built default widget class:

```text
UChronicleDialogueDefaultWidget
```

It supports:

```text
typewriter reveal
choices
backlog
auto mode
skip mode
rollback
portrait and full-body slots
continue prompt
```

### 24.1 Create a Blueprint widget subclass

1. Open:

```text
Content/Chronicle/UI
```

2. Right-click.
3. Create **User Widget Blueprint**.
4. Search parent class:

```text
Chronicle Dialogue Default Widget
```

5. Name it:

```text
WBP_ChronicleDialogue
```

6. Open the widget.
7. If you want the source-built layout, keep:

```text
bBuildDefaultLayout = true
```

8. Save.

### 24.2 Add the widget to viewport

Blueprint operation:

1. On `BeginPlay`, call `Create Widget`.
2. Set class to `WBP_ChronicleDialogue`.
3. Call `Add to Viewport`.
4. Get `Chronicle Dialogue Subsystem`.
5. Get `Presentation Controller`.
6. Call `Bind Presentation Controller` on the widget.
7. Pass the controller.

Expected result: the widget receives presentation events and displays lines, choices, backlog, auto, skip, and rollback state.

## 25. Use the Demo Actor

The fastest runtime smoke test is the demo actor.

1. Open a test map.
2. Open the Place Actors panel.
3. Search for:

```text
Chronicle Dialogue Demo Actor
```

4. Drag it into the level.
5. Select the actor.
6. In Details, keep:

| Field | Value |
|---|---|
| bStartOnBeginPlay | `true` |
| bCreateDefaultWidget | `true` |
| Dialogue Widget Class | leave default or set `WBP_ChronicleDialogue` |
| Widget Z Order | `50` |

7. Press Play.

Expected result: the actor builds a runtime demo tree and starts a playable dialogue automatically.

## 26. Bind Gameplay Systems to Dialogue Events

For quests, game state, battle, scene loading, and actor animation, bind to runner events.

Blueprint path:

1. Get `Chronicle Dialogue Subsystem`.
2. Get `Dialogue Runner`.
3. Bind to `On Dialogue Event`.
4. Break `Dialogue Event Data`.
5. Switch on `Event Tag`.
6. Read `Payload`.

C++ path:

```cpp
Runner->OnDialogueEvent.AddDynamic(this, &UMyQuestBridge::HandleDialogueEvent);
```

Handler example:

```cpp
void UMyQuestBridge::HandleDialogueEvent(const FDialogueEventData& EventData)
{
    if (EventData.EventTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Event.Quest.Start"))))
    {
        const FString* QuestTag = EventData.Payload.Find(TEXT("QuestTag"));
        if (QuestTag)
        {
            StartQuest(*QuestTag);
        }
    }
}
```

If the event is async, call `NotifyEventComplete` when your gameplay work finishes.

## 27. Save, Load, and Rollback

Runner state can be saved to `FDialogueSaveData`.

C++ save example:

```cpp
FDialogueSaveData SaveData;
Runner->SaveState(SaveData);
```

C++ load example:

```cpp
Runner->LoadState(SaveData);
```

Rollback example:

```cpp
Runner->PerformRollback(1);
```

Presentation rollback example:

```cpp
Controller->RequestRollback(1);
```

Recommended save-game approach:

1. Create a project `USaveGame` class.
2. Store `FDialogueSaveData`.
3. Store your own quest, inventory, and map state next to it.
4. On load, initialize the dialogue database first.
5. Then call `LoadState`.

## 28. Inline Tags in Dialogue Text

The text parser supports inline tag parsing and presenter callbacks.

Use inline tags for small presentation hints such as pauses, emphasis, sound cues, or custom project hooks.

Example text pattern:

```text
Hello there. [Chronicle.Audio.PlayVoice:VoiceID=VO_Intro_001] Ready to begin?
```

Presenter flow:

1. The runner presents a line.
2. The text parser finds inline tags.
3. The widget or presenter receives `HandleInlineTag`.
4. Your presenter decides what to do with the tag and params.

Keep important branching logic in nodes, conditions, and events. Use inline tags for presentation details.

## 29. Export JSON, Dialogue CSV, Localization CSV, and Audit JSON

### 29.1 Export from the Content Browser

1. In Content Browser, select `DT_Intro`.
2. Right-click the asset.
3. Choose:

```text
Export Chronicle Pipeline Artifacts...
```

4. Pick an export folder.
5. Confirm.

Expected output:

```text
DT_Intro.dialogue.json
DT_Intro.lines.csv
DT_Intro.localization.csv
DT_Intro.audit.json
```

The exact filenames depend on the asset name and export helper.

### 29.2 Export from Blueprint or Editor Utility

Use:

```text
Export Dialogue Tree Pipeline Artifacts
```

Function:

```cpp
UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts
```

This creates:

1. JSON tree data.
2. Dialogue line CSV.
3. Localization CSV.
4. Audit JSON.

## 30. Import a Script CSV

Script CSV is useful when writers work in Excel, Google Sheets, or localization tools.

### 30.1 Minimal CSV shape

Use a CSV with the columns supported by the built-in importer. Required columns are `LineID` and `Text`. The importer creates Speech nodes, connects them through `NextLineID`, and optionally inserts Event nodes after Speech nodes when `EventTag` is present.

```csv
LineID,Text,SpeakerTag,NextLineID,ConditionExpression,EventTag,EventPayload,bEventIsAsync,VoiceID,WaitTime
Intro_001,"Welcome to the village.",Chronicle.Speaker.Alice,Intro_002,,,,false,VO_Intro_001,-1
Intro_002,"The ruins are older than the kingdom.",Chronicle.Speaker.Alice,QuestStart,,Chronicle.Event.Quest.Start,"QuestTag=Chronicle.Quest.Main;Step=IntroStarted",false,VO_Intro_002,-1
QuestStart,"Come back when you are ready.",Chronicle.Speaker.Alice,,,,,false,VO_QuestStart_001,-1
```

Check `Documentation/AssetPipeline.md` for the current supported CSV contract before building a production sheet.

### 30.2 Import from Content Browser

1. Create or select a target Dialogue Tree.
2. Right-click the tree.
3. Choose:

```text
Import Chronicle Script CSV...
```

4. Select the CSV file.
5. Confirm import.
6. Open the tree.
7. Inspect nodes, edges, line text, choices, conditions, events, and payload.
8. Save.

Important: the current Content Browser action replaces the selected tree with generated content.

## 31. Localization Workflow

### 31.1 Prepare line IDs

1. Open the Dialogue Tree.
2. Make sure every dialogue line has a stable `LineID`.
3. Use IDs such as:

```text
Intro_001
Intro_002
ShopGreeting_001
QuestAccept_001
```

4. Avoid changing IDs after translation begins.

### 31.2 Export localization CSV

1. Right-click the Dialogue Tree.
2. Choose:

```text
Export Chronicle Pipeline Artifacts...
```

3. Select the export folder.
4. Send the localization CSV to translators.

### 31.3 Import translated CSV

Use the editor helper:

```text
Import Localization Csv To Tree
```

Function:

```cpp
UChronicleDialogueJsonLibrary::ImportLocalizationCsvToTree
```

Recommended production rule:

1. Lock source line IDs before translation.
2. Export localization CSV.
3. Translate only target text columns.
4. Import into a copy of the tree first.
5. Validate.
6. Merge into production content.

## 32. Validate a Dialogue Tree

Validation catches broken edges, missing root data, unreachable nodes, malformed expressions, and other authoring issues.

### 32.1 Validate inside the editor

1. Open `DT_Intro`.
2. Use the validation summary area in the Dialogue Tree editor.
3. Fix warnings and errors.
4. Save.

### 32.2 Validate through Blueprint or Editor Utility

Use:

```text
Validate Dialogue Tree
```

Function:

```cpp
UChronicleDialogueJsonLibrary::ValidateDialogueTree
```

### 32.3 Build an audit report

Use:

```text
Build Dialogue Audit Report
```

Function:

```cpp
UChronicleDialogueAuditLibrary::BuildDialogueAuditReport
```

Audit data includes:

```text
node count
edge count
speech line count
choice count
word count
broken edge count
unreachable node count
warning count
error count
speaker line stats
variable usage
validation issues
```

## 33. Run Automated Tests

Close Unreal Editor before running command-line automation.

### 33.1 Run the full validation script

From the repository root:

```powershell
.\Scripts\RunChronicleValidation.ps1
```

This script performs:

1. UE 5.3 editor build.
2. UE 5.3 Chronicle automation tests.
3. UE 5.3 plugin packaging.
4. UE 5.7 compatibility build smoke test, unless skipped.
5. UE 5.3 rebuild restore after the UE 5.7 smoke test.

### 33.2 Run only UE 5.3 build and tests

```powershell
.\Scripts\RunChronicleValidation.ps1 -SkipPackage -Skip57Smoke
```

### 33.3 Run build only

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### 33.4 Run automation directly

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

Expected result:

```text
Automation tests complete
No Chronicle test failures
```

## 34. Package the Plugin

To package Chronicle Engine for another Unreal project:

```powershell
.\Scripts\PackagePlugin.ps1
```

Default output:

```text
Artifacts\ChronicleEngine-0.12.0-UE5.3
```

Custom package name:

```powershell
.\Scripts\PackagePlugin.ps1 -PackageName "ChronicleEngine-MyBuild-UE5.3"
```

Custom engine path:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "D:\Epic\UE_5.3"
```

## 35. Install the Packaged Plugin in Another Project

1. Open the packaged plugin folder:

```text
Artifacts\ChronicleEngine-0.12.0-UE5.3
```

2. Copy the `ChronicleEngine` plugin folder.
3. Paste it into your game project:

```text
YourGameProject\Plugins\ChronicleEngine
```

4. Open your `.uproject`.
5. Enable the plugin if prompted.
6. Rebuild modules.
7. Confirm the asset types appear in the Content Browser.
8. Copy or recreate your Dialogue Database, Dialogue Trees, Speaker Profiles, and Trigger assets in the target project.

For source integration, you can also copy:

```text
Plugins\ChronicleEngine
```

directly from this repository into another project and rebuild there.

## 36. Recommended Production Workflow

For a real project, use this loop:

1. Create or update Dialogue Tree in Unreal Editor.
2. Add or revise line IDs.
3. Validate tree.
4. Export pipeline artifacts.
5. Review audit JSON for broken edges and unreachable nodes.
6. Send localization CSV to translators.
7. Import translated CSV into a copy first.
8. Run automation tests.
9. Test in PIE with the default widget.
10. Connect project UI, quest, camera, audio, and save systems.
11. Run a packaged build smoke test.
12. Commit only source, content, config, scripts, and documentation.
13. Do not commit `Binaries`, `Intermediate`, `Saved`, or generated `Artifacts`.

## 37. Troubleshooting

### Asset types do not appear

1. Confirm the plugin is enabled.
2. Rebuild `ChronicleHostEditor`.
3. Restart Unreal Editor.
4. Check that `ChronicleEngineEditor` compiled successfully.

### The module rebuild dialog appears

This is normal after switching engine versions or cleaning binaries.

1. Click **Yes**.
2. If it fails, close Unreal.
3. Run the manual build command from section 3.2.

### The graph editor opens as a tiny area

1. Dock or maximize the asset editor tab.
2. Use Zoom To Fit.
3. Reset the Unreal layout.
4. Clear local `Saved/` and `Intermediate/` cache if needed.

### A choice does not show up

1. Check its `Visibility Condition`.
2. Confirm the variable tag exists.
3. Confirm the runner variable has the expected value.
4. Temporarily clear the condition to confirm the edge works.

### A condition branch does not run

1. Check the edge condition, not only the node condition.
2. Confirm each branch has an outgoing edge slot.
3. Validate the expression with `ValidateConditionExpressionForTree`.
4. Make sure string values are quoted:

```text
Chronicle.Variable.Name == "Alice"
```

### An async event freezes dialogue

1. Confirm `bEventIsAsync` is really needed.
2. Confirm your gameplay system receives `OnDialogueEvent`.
3. Confirm your system calls `NotifyEventComplete`.
4. Confirm the same Gameplay Tag is passed back.

### No UI appears

1. Confirm a widget was created.
2. Confirm it was added to viewport.
3. Confirm it was bound with `BindPresentationController`.
4. Confirm the controller was bound to the runner with `BindRunner`.
5. Confirm dialogue actually started.

### Automation fails after a UE version smoke test

Run the UE 5.3 build again:

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

## 38. Minimum Acceptance Checklist

Use this checklist before calling a dialogue feature complete.

```text
[ ] Dialogue Tree has one Root node.
[ ] Every Speech line has a stable LineID.
[ ] Speaker tags resolve to Speaker Profiles.
[ ] Choice branches connect to valid output slots.
[ ] Conditions are validated.
[ ] Event tags exist in Gameplay Tags.
[ ] Async events call NotifyEventComplete.
[ ] Dialogue Database references the tree.
[ ] Presentation controller is bound to the runner.
[ ] Widget is bound to the presentation controller.
[ ] Save/load is tested if the dialogue can cross save boundaries.
[ ] Rollback is tested if the UI exposes rollback.
[ ] Pipeline artifacts export successfully.
[ ] Automation tests pass.
```

---

<a id="chinese-step-by-step-tutorial"></a>

# 中文超详细逐步使用教程

这份文档是 README 的手把手操作版。它默认你从源码仓库开始，目标是在 Unreal Editor 里启用 Chronicle Engine，创建对话资产，编辑对话树，运行测试，接入 UI 与事件，导出流水线文件，并最终打包插件。

## 1. 最终你会完成什么

完成本教程后，你会得到：

1. 一个能打开并编译的宿主工程。
2. 一个已启用的 Chronicle Engine 插件。
3. 一个 Dialogue Database、一个 Speaker Profile、一个 Dialogue Tree。
4. 一个包含台词、选项、条件、事件的可播放对话树。
5. 一个通过 Presentation Controller 和默认 Widget 播放的对话流程。
6. 一套 JSON、台词 CSV、本地化 CSV、审计 JSON 导出文件。
7. 一次自动化测试验证。
8. 一个可以复制到其他 UE 项目的插件打包结果。

教程使用仓库自带宿主项目：

```text
R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject
```

如果你的仓库在其他位置，把后续命令里的路径替换成你的实际路径。

## 2. 准备条件

开始前先检查这些内容。

1. 已安装 Unreal Engine 5.3。
2. 默认教程使用这个引擎路径：

```text
R:\UE\UE_5.3
```

3. 已安装 Visual Studio，并包含 C++ 游戏开发组件。
4. 仓库里存在这些文件和目录：

```text
ChronicleHost.uproject
Plugins\ChronicleEngine\ChronicleEngine.uplugin
Plugins\ChronicleEngine\Source\ChronicleEngine
Plugins\ChronicleEngine\Source\ChronicleEngineEditor
Plugins\ChronicleEngine\Source\ChronicleEngineTests
Scripts\RunChronicleValidation.ps1
Scripts\PackagePlugin.ps1
```

5. 可选兼容测试：安装 Unreal Engine 5.7 到：

```text
R:\UE\UE_5.7
```

UE 5.3 是主要基线。UE 5.7 只用于兼容冒烟测试。

## 3. 第一次打开与重建模块

### 3.1 直接打开项目

1. 双击：

```text
ChronicleHost.uproject
```

2. 如果 Unreal 弹出：

```text
The following modules are missing or built with a different engine version:
ChronicleHost
ChronicleEngine
ChronicleEngineEditor
ChronicleEngineTests
Would you like to rebuild them now?
```

3. 点击 **Yes**。
4. 等 Unreal Build Tool 编译完成。
5. 编译成功后，Unreal Editor 会自动打开。

### 3.2 如果弹窗重建失败

手动编译一次。

1. 关闭 Unreal Editor。
2. 在仓库根目录打开 PowerShell：

```text
R:\AI_Agent\Codex\JRPGtalking
```

3. 执行：

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

4. 编译成功后，再次双击 `ChronicleHost.uproject`。

### 3.3 正常结果

编辑器能打开，并且 Chronicle Engine 插件已加载。第一次打开会慢一些，因为 Unreal 需要生成工程文件、中间文件和二进制文件。

## 4. 确认插件已经启用

1. 在 Unreal Editor 中打开 **Edit > Plugins**。
2. 搜索：

```text
Chronicle Engine
```

3. 确认插件处于启用状态。
4. 如果没启用，勾选启用并重启编辑器。
5. 重启后打开 Content Browser，在空白处右键。
6. 在资产创建菜单中搜索这些类型：

```text
Dialogue Tree
Dialogue Database
Speaker Profile
Dialogue Trigger
```

如果这些资产类型能搜到，说明编辑器模块已经正常工作。

## 5. 推荐的内容目录结构

正式创建资产前，建议先建好目录。

1. 打开 Content Browser。
2. 进入项目 `Content` 根目录。
3. 创建文件夹：

```text
Content/Chronicle
```

4. 在里面继续创建：

```text
Content/Chronicle/Dialogues
Content/Chronicle/Speakers
Content/Chronicle/Databases
Content/Chronicle/Triggers
Content/Chronicle/UI
Content/Chronicle/Exports
```

推荐命名方式：

```text
DT_Intro
DB_Chronicle
SP_Alice
TR_IntroInteract
WBP_ChronicleDialogue
```

Chronicle Engine 不强制这些名字，但大项目里统一前缀会让搜索和维护轻松很多。

## 6. 检查 Gameplay Tags

Chronicle Engine 用 Gameplay Tags 表示角色、事件、变量、触发器、镜头提示、动画提示和音频提示。

宿主项目已经在这里内置了示例标签：

```text
Config/DefaultGameplayTags.ini
```

常用示例：

```text
Chronicle.Speaker.Alice
Chronicle.Variable.Score
Chronicle.Variable.Flag
Chronicle.Event.Quest.Start
Chronicle.Event.Quest.Update
Chronicle.Event.Quest.Complete
Chronicle.Camera.Cut
Chronicle.Camera.Blend
Chronicle.Animation.Play
Chronicle.Audio.PlayVoice
Chronicle.Trigger.Test
```

在编辑器中查看或新增标签：

1. 打开 **Edit > Project Settings**。
2. 搜索 **Gameplay Tags**。
3. 进入 **Gameplay Tags**。
4. 确认 `Chronicle.*` 标签存在。
5. 如果项目需要自己的标签，可以继续放在 `Chronicle.*` 根下面。

推荐标签根：

```text
Chronicle.Speaker.*
Chronicle.Variable.*
Chronicle.Event.*
Chronicle.Trigger.*
Chronicle.Camera.*
Chronicle.Animation.*
Chronicle.Audio.*
Chronicle.Quest.*
```

## 7. 创建 Speaker Profile

Speaker Profile 保存角色显示名、头像、立绘、文字颜色和默认站位。

1. 打开：

```text
Content/Chronicle/Speakers
```

2. 在空白处右键。
3. 搜索：

```text
Speaker Profile
```

4. 创建资产。
5. 命名为：

```text
SP_Alice
```

6. 双击 `SP_Alice`。
7. 填写字段：

| 字段 | 示例 |
|---|---|
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Display Name | `Alice` |
| Text Color | 白色或角色专属颜色 |
| Default Position | `Left` |
| Portrait Set | 可选，例如 `Neutral`、`Happy`、`Sad` |
| Full Body Set | 可选 |
| Voice Set | 可选 Data Table |

8. 保存资产。

正常结果：`SP_Alice` 可以被 Dialogue Database 引用，台词中使用 `Chronicle.Speaker.Alice` 时也能对应到这个角色。

## 8. 创建 Dialogue Database

Dialogue Database 是角色、全局变量、对话树、本地化设置和语音表的统一注册中心。

1. 打开：

```text
Content/Chronicle/Databases
```

2. 在空白处右键。
3. 搜索：

```text
Dialogue Database
```

4. 创建资产。
5. 命名为：

```text
DB_Chronicle
```

6. 双击 `DB_Chronicle`。
7. 在 **Speaker Profiles** 里添加 `SP_Alice`。
8. 在 **Global Variables** 里添加全局变量。

示例整数变量：

| 字段 | 值 |
|---|---|
| Variable Tag | `Chronicle.Variable.Score` |
| Scope | `Global` |
| Default Value Type | `Int32` |
| Int Value | `0` |
| Display Name | `Score` |

示例布尔变量：

| 字段 | 值 |
|---|---|
| Variable Tag | `Chronicle.Variable.Flag` |
| Scope | `Global` |
| Default Value Type | `Bool` |
| Bool Value | `false` |
| Display Name | `Flag` |

9. 在 **Localization Settings** 中填写：

| 字段 | 示例 |
|---|---|
| Namespace | `Project.Dialogue` |
| Target Cultures | `en`, `zh-Hans` |
| Voice Table Path | 可选 |

10. 保存资产。

## 9. 创建第一棵 Dialogue Tree

1. 打开：

```text
Content/Chronicle/Dialogues
```

2. 在空白处右键。
3. 搜索：

```text
Dialogue Tree
```

4. 创建资产。
5. 命名为：

```text
DT_Intro
```

6. 双击 `DT_Intro`。

正常结果：Chronicle Dialogue Tree 编辑器打开。新建树会默认带一个 Root 节点。

## 10. 如果对话树编辑器打开后特别小

如果你看到只有左上角一小块地方能编辑：

1. 把编辑器标签页拖出来并最大化，或者重新停靠到主编辑器区域。
2. 如果工具栏里有 **Zoom To Fit**，点击它。
3. 如果整个 Unreal 布局异常，打开 **Window > Load Layout > Default Editor Layout**。
4. 关闭并重新打开 `DT_Intro`。
5. 如果仍然不正常，关闭 Unreal，只清理本地缓存目录：

```text
Saved/
Intermediate/
```

不要删除 `Content/` 或 `Plugins/ChronicleEngine/Source/`。

正常结果：自定义图编辑器应该占满资产编辑器区域，节点可以在正常大小的图空间里编辑。

## 11. 搭建最小台词图

这一节会创建：

```text
Root -> Speech
```

1. 打开 `DT_Intro`。
2. 在图区域右键。
3. 选择或搜索：

```text
Speech
```

4. 在 Root 右侧创建 Speech 节点。
5. 选中 Speech 节点。
6. 在 Details 面板里给 **Lines** 添加一个元素。
7. 填写台词：

| 字段 | 值 |
|---|---|
| Line ID | `Intro_001` |
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Text | `Welcome to Chronicle Engine.` |
| Emotion Tag | 可选 |
| Voice ID | `VO_Intro_001` |
| Wait Time | `-1.0` |

8. 把 Root 节点输出连接到 Speech 节点输入。
9. 保存资产。

正常结果：树里有一个 Root 节点连接到一个 Speech 节点。

## 12. 给一个 Speech 节点添加多句台词

1. 选中 Speech 节点。
2. 在 **Lines** 中添加第二个元素。
3. 填写：

| 字段 | 值 |
|---|---|
| Line ID | `Intro_002` |
| Speaker Tag | `Chronicle.Speaker.Alice` |
| Text | `This second line appears after the player advances.` |
| Voice ID | `VO_Intro_002` |

4. 保存。

运行时行为：

1. 第一句触发 `OnLineStarted`。
2. 玩家调用 `Advance`。
3. 第二句触发 `OnLineStarted`。
4. 再次 `Advance` 后沿输出边进入下一个节点。

## 13. 添加 Choice 节点

这一节会创建：

```text
Root -> Speech -> Choice
Choice slot 0 -> Speech
Choice slot 1 -> Speech
```

1. 在图中右键。
2. 在第一段 Speech 右侧创建 **Choice** 节点。
3. 选中 Choice 节点。
4. 在 **Choices** 中添加两个元素。
5. 填写选项 0：

| 字段 | 值 |
|---|---|
| Text | `Tell me more.` |
| Visibility Condition | 留空 |
| Target Output Index | 手动编辑时可保持默认 |

6. 填写选项 1：

| 字段 | 值 |
|---|---|
| Text | `Skip the details.` |
| Visibility Condition | 留空 |
| Target Output Index | 手动编辑时可保持默认 |

7. 在 Choice 后面创建两个 Speech 节点。
8. 给它们的台词 ID 分别设置为：

```text
Intro_More_001
Intro_Short_001
```

9. 给两个分支分别添加文本。
10. 把第一段 Speech 连接到 Choice。
11. 把 Choice 的输出 slot 0 连接到 `Intro_More_001` 所在 Speech。
12. 把 Choice 的输出 slot 1 连接到 `Intro_Short_001` 所在 Speech。
13. 保存。

运行时行为：

1. Runner 进入 Choice 节点。
2. Runner 计算每个选项的可见条件。
3. Runner 广播 `OnChoicesPresented`。
4. 玩家选择一个索引。
5. `SelectChoice(0)` 沿输出 slot 0 前进。
6. `SelectChoice(1)` 沿输出 slot 1 前进。

## 14. 添加选项可见条件

选项可见条件使用 Chronicle 自研条件表达式解析器。

支持：

```text
变量引用
括号
!
&&
||
==
!=
<
<=
>
>=
数字字面量
字符串字面量
布尔字面量
```

表达式示例：

```text
Chronicle.Variable.Score >= 10
Chronicle.Variable.Flag == true
(Chronicle.Variable.Score >= 10) && (Chronicle.Variable.Flag == true)
Chronicle.Variable.Name == "Alice"
```

让一个选项在分数至少为 10 时才显示：

1. 选中 Choice 节点。
2. 展开 **Choices**。
3. 找到要限制显示的选项。
4. 设置 **Visibility Condition**：

```text
Chronicle.Variable.Score >= 10
```

5. 保存。

正常结果：只有当 Runner 变量 `Chronicle.Variable.Score` 大于等于 10 时，这个选项才会显示。

## 15. 在运行时设置变量

变量可以通过 Blueprint 或 C++ 设置。

Blueprint 操作：

1. 获取 Game Instance。
2. 获取 `Chronicle Dialogue Subsystem`。
3. 调用 `Get Dialogue Runner`。
4. 创建 `FVariableValue`。
5. 调用 `Set Variable`。
6. 填入：

| Pin | 值 |
|---|---|
| Tag | `Chronicle.Variable.Score` |
| Value Type | `Int32` |
| Int Value | `10` |
| Scope | `Global` |

C++ 示例：

```cpp
UChronicleDialogueSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->SetVariable(
    FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Variable.Score")),
    FVariableValue::MakeInt(10),
    EChronicleVariableScope::Global);
```

## 16. 添加 Condition 节点

Condition 节点根据输出边条件选择分支。

示例图：

```text
Speech -> Condition
Condition slot 0 -> HighScoreSpeech
Condition slot 1 -> LowScoreSpeech
```

1. 在 Speech 后面创建 **Condition** 节点。
2. 在 Condition 后面创建两个 Speech 节点。
3. 连接 Speech 到 Condition。
4. 连接 Condition 输出 slot 0 到高分 Speech。
5. 设置 slot 0 的边条件：

```text
Chronicle.Variable.Score >= 10
```

6. 连接 Condition 输出 slot 1 到低分 Speech。
7. 设置 slot 1 的边条件：

```text
Chronicle.Variable.Score < 10
```

8. 保存。

正常结果：Runner 会沿第一个通过条件的分支前进。

## 17. 添加 Event 节点

Event 节点把 GameplayTag 事件和 Payload 数据发送给游戏系统。

示例图：

```text
Speech -> Event -> Speech
```

1. 创建 **Event** 节点。
2. 选中它。
3. 设置 **Event Tag**：

```text
Chronicle.Event.Quest.Start
```

4. 添加 Payload：

| Key | Value |
|---|---|
| QuestTag | `Chronicle.Quest.Main` |
| Step | `IntroStarted` |

5. 如果只是同步事件，不勾选 **bEventIsAsync**。
6. 把前一个 Speech 连接到 Event。
7. 把 Event 连接到下一个 Speech。
8. 保存。

运行时行为：

1. Runner 进入 Event 节点。
2. Runner 广播 `OnDialogueEvent`。
3. Presentation Controller 广播 `OnPresentationEvent`。
4. 同步事件会立即继续。

## 18. 添加异步 Event

当对话必须等待其他系统完成时，使用异步事件，例如加载场景、播放 Timeline、进入战斗、等待动画结束。

1. 选中 Event 节点。
2. 勾选：

```text
bEventIsAsync
```

3. 设置事件标签，例如：

```text
Chronicle.Event.Scene.Load
```

4. 添加 Payload：

| Key | Value |
|---|---|
| Scene | `VillageNight` |

5. 在你的游戏系统中监听 `OnDialogueEvent`。
6. 执行异步工作。
7. 异步工作完成后调用：

```cpp
Runner->NotifyEventComplete(FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Event.Scene.Load")));
```

正常结果：Runner 会停在 `WaitingForEvent`，直到调用 `NotifyEventComplete`。

## 19. 添加镜头和动画提示

Chronicle Engine 把镜头和动画提示看作事件式表现层 Cue。

镜头切换示例：

| 字段 | 值 |
|---|---|
| Node Type | `Camera` 或事件式镜头节点 |
| Event Tag | `Chronicle.Camera.Cut` |
| Payload `Shot` | `IntroCloseup` |
| Payload `BlendTime` | `0.25` |

动画示例：

| 字段 | 值 |
|---|---|
| Node Type | `Animation` 或事件式动画节点 |
| Event Tag | `Chronicle.Animation.Play` |
| Payload `Actor` | `Alice` |
| Payload `Animation` | `Wave` |

把你自己的 Camera Manager、Sequencer Controller 或 Animation System 绑定到 `OnPresentationEvent` 或 `OnDialogueEvent`，读取标签和 Payload 后执行具体表现。

## 20. 使用 Wait、Random、Jump、Sequence、Sub-Dialogue 节点

Chronicle 已经以源码形式支持 PRD 中的全部节点类型。

常见用途：

| 节点类型 | 用途 |
|---|---|
| Wait | 暂停一段时间，设置 `WaitTime` |
| Random | 根据边的 `Weight` 随机选择输出分支 |
| Jump | 跳到目标入口节点或目标树 |
| Sequence | 按顺序遍历输出 slot |
| SubDialogue | 进入另一棵 Dialogue Tree，结束后可返回 |
| Camera | 发出镜头 Cue |
| Animation | 发出动画 Cue |

Sub-Dialogue 操作：

1. 创建另一棵树，例如 `DT_ShopGreeting`。
2. 把它加入 `DB_Chronicle`。
3. 在 SubDialogue 节点中把 **Target Tree** 设为 `DT_ShopGreeting`。
4. 如果你使用命名入口，填写 **Target Entry Node**。
5. 如果希望子对话结束后回到父树，勾选 **bReturnToNextNodeOnSubDialogueEnd**。

## 21. 把 Dialogue Tree 加入 Database

1. 打开 `DB_Chronicle`。
2. 在 **Dialogue Trees** 中添加 `DT_Intro`。
3. 保存。

这样数据库查找、校验、本地化收集和运行时初始化都能找到这棵树。

## 22. 从 Blueprint 启动对话

推荐的 Blueprint 入口是 `UChronicleDialogueSubsystem`。

### 22.1 Level Blueprint 设置

1. 打开要测试的地图。
2. 打开 **Blueprints > Open Level Blueprint**。
3. 在 `BeginPlay` 后依次添加：

```text
Get Game Instance
Get Subsystem (Chronicle Dialogue Subsystem)
Initialize Dialogue Database
Get Presentation Controller
Get Dialogue Runner
Bind Runner
Start Dialogue
```

4. 把 `Initialize Dialogue Database` 的输入设为 `DB_Chronicle`。
5. 把 `Start Dialogue` 的 Tree 输入设为 `DT_Intro`。
6. Entry Node 先保持 `None`，表示从 Root 进入。
7. 保存并 Play。

正常结果：关卡开始时对话启动。

### 22.2 添加手动输入

手动推进对话：

1. 在 Level Blueprint 或 Player Controller Blueprint 中监听输入。
2. 获取 `Chronicle Dialogue Subsystem`。
3. 获取 `Presentation Controller`。
4. 调用：

```text
Advance
```

选择选项：

1. 调用：

```text
Select Choice
```

2. 传入 `0` 表示第一个可见选项，`1` 表示第二个可见选项，依此类推。

## 23. 从 C++ 启动对话

如果你的游戏模块需要直接调用 Chronicle，先在 Build.cs 中添加依赖：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "ChronicleEngine",
    "GameplayTags",
    "UMG"
});
```

C++ 启动示例：

```cpp
#include "Runtime/ChronicleDialogueSubsystem.h"
#include "Runtime/DialogueRunner.h"
#include "Presentation/ChronicleDialoguePresentationController.h"

void AMyDialogueStarter::StartIntroDialogue()
{
    UChronicleDialogueSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UChronicleDialogueSubsystem>();
    if (!Subsystem)
    {
        return;
    }

    Subsystem->InitializeDialogueDatabase(DialogueDatabase);

    UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
    UChronicleDialoguePresentationController* Controller = Subsystem->GetPresentationController();
    Controller->BindRunner(Runner);
    Controller->StartDialogue(IntroTree);
}
```

## 24. 使用默认 Widget

Chronicle Engine 内置源码构建的默认 Widget 类：

```text
UChronicleDialogueDefaultWidget
```

它支持：

```text
打字机显示
选项
Backlog
Auto 模式
Skip 模式
Rollback
头像和立绘槽位
继续提示
```

### 24.1 创建 Widget Blueprint 子类

1. 打开：

```text
Content/Chronicle/UI
```

2. 右键。
3. 创建 **User Widget Blueprint**。
4. 搜索父类：

```text
Chronicle Dialogue Default Widget
```

5. 命名为：

```text
WBP_ChronicleDialogue
```

6. 打开 Widget。
7. 如果想直接使用源码默认布局，保持：

```text
bBuildDefaultLayout = true
```

8. 保存。

### 24.2 把 Widget 加到视口

Blueprint 操作：

1. 在 `BeginPlay` 调用 `Create Widget`。
2. Class 选择 `WBP_ChronicleDialogue`。
3. 调用 `Add to Viewport`。
4. 获取 `Chronicle Dialogue Subsystem`。
5. 获取 `Presentation Controller`。
6. 在 Widget 上调用 `Bind Presentation Controller`。
7. 传入 Controller。

正常结果：Widget 会接收表现层事件并显示台词、选项、Backlog、Auto、Skip、Rollback 状态。

## 25. 使用 Demo Actor 快速测试

最快的运行时冒烟测试是 Demo Actor。

1. 打开测试地图。
2. 打开 Place Actors 面板。
3. 搜索：

```text
Chronicle Dialogue Demo Actor
```

4. 拖入关卡。
5. 选中 Actor。
6. 在 Details 中保持：

| 字段 | 值 |
|---|---|
| bStartOnBeginPlay | `true` |
| bCreateDefaultWidget | `true` |
| Dialogue Widget Class | 保持默认或设为 `WBP_ChronicleDialogue` |
| Widget Z Order | `50` |

7. 点击 Play。

正常结果：Demo Actor 会运行时构造一棵演示对话树，并自动开始可播放对话。

## 26. 把游戏系统绑定到对话事件

任务、游戏状态、战斗、场景加载、角色动画等系统都可以监听 Runner 事件。

Blueprint 路径：

1. 获取 `Chronicle Dialogue Subsystem`。
2. 获取 `Dialogue Runner`。
3. 绑定 `On Dialogue Event`。
4. 拆开 `Dialogue Event Data`。
5. 根据 `Event Tag` 分支。
6. 读取 `Payload`。

C++ 路径：

```cpp
Runner->OnDialogueEvent.AddDynamic(this, &UMyQuestBridge::HandleDialogueEvent);
```

处理函数示例：

```cpp
void UMyQuestBridge::HandleDialogueEvent(const FDialogueEventData& EventData)
{
    if (EventData.EventTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(TEXT("Chronicle.Event.Quest.Start"))))
    {
        const FString* QuestTag = EventData.Payload.Find(TEXT("QuestTag"));
        if (QuestTag)
        {
            StartQuest(*QuestTag);
        }
    }
}
```

如果事件是异步的，玩法系统完成后记得调用 `NotifyEventComplete`。

## 27. 保存、加载和回滚

Runner 状态可以保存到 `FDialogueSaveData`。

C++ 保存：

```cpp
FDialogueSaveData SaveData;
Runner->SaveState(SaveData);
```

C++ 加载：

```cpp
Runner->LoadState(SaveData);
```

回滚：

```cpp
Runner->PerformRollback(1);
```

表现层回滚：

```cpp
Controller->RequestRollback(1);
```

推荐存档方式：

1. 创建项目自己的 `USaveGame` 类。
2. 存储 `FDialogueSaveData`。
3. 把任务、背包、地图等项目状态一起存进去。
4. 读取存档时，先初始化 Dialogue Database。
5. 再调用 `LoadState`。

## 28. 对话文本中的 Inline Tag

文本解析器支持 Inline Tag，并会转发给 Presenter。

Inline Tag 适合做小型表现提示，例如暂停、强调、音效提示或项目自定义表现。

文本示例：

```text
Hello there. [Chronicle.Audio.PlayVoice:VoiceID=VO_Intro_001] Ready to begin?
```

Presenter 流程：

1. Runner 展示一行台词。
2. 文本解析器发现 Inline Tag。
3. Widget 或 Presenter 收到 `HandleInlineTag`。
4. 你的 Presenter 根据 Tag 和参数执行具体表现。

重要分支逻辑建议放在节点、条件和事件里。Inline Tag 更适合做表现细节。

## 29. 导出 JSON、台词 CSV、本地化 CSV 和审计 JSON

### 29.1 从 Content Browser 导出

1. 在 Content Browser 选中 `DT_Intro`。
2. 右键资产。
3. 选择：

```text
Export Chronicle Pipeline Artifacts...
```

4. 选择导出文件夹。
5. 确认。

预期输出：

```text
DT_Intro.dialogue.json
DT_Intro.lines.csv
DT_Intro.localization.csv
DT_Intro.audit.json
```

实际文件名会根据资产名和导出辅助函数略有变化。

### 29.2 从 Blueprint 或 Editor Utility 导出

使用：

```text
Export Dialogue Tree Pipeline Artifacts
```

对应函数：

```cpp
UChronicleDialogueEditorLibrary::ExportDialogueTreePipelineArtifacts
```

它会创建：

1. JSON 树数据。
2. 台词 CSV。
3. 本地化 CSV。
4. 审计 JSON。

## 30. 导入 Script CSV

Script CSV 适合编剧在 Excel、Google Sheets 或本地化工具中批量编写对话。

### 30.1 最小 CSV 示例

使用内置导入器当前支持的列。必填列是 `LineID` 和 `Text`。导入器会创建 Speech 节点，用 `NextLineID` 连接节点；如果填写了 `EventTag`，会在 Speech 节点后插入 Event 节点。

```csv
LineID,Text,SpeakerTag,NextLineID,ConditionExpression,EventTag,EventPayload,bEventIsAsync,VoiceID,WaitTime
Intro_001,"Welcome to the village.",Chronicle.Speaker.Alice,Intro_002,,,,false,VO_Intro_001,-1
Intro_002,"The ruins are older than the kingdom.",Chronicle.Speaker.Alice,QuestStart,,Chronicle.Event.Quest.Start,"QuestTag=Chronicle.Quest.Main;Step=IntroStarted",false,VO_Intro_002,-1
QuestStart,"Come back when you are ready.",Chronicle.Speaker.Alice,,,,,false,VO_QuestStart_001,-1
```

生产环境正式做表前，请先查看 `Documentation/AssetPipeline.md` 中当前支持的 CSV 契约。

### 30.2 从 Content Browser 导入

1. 创建或选中目标 Dialogue Tree。
2. 右键这棵树。
3. 选择：

```text
Import Chronicle Script CSV...
```

4. 选择 CSV 文件。
5. 确认导入。
6. 打开对话树。
7. 检查节点、边、台词、选项、条件、事件和 Payload。
8. 保存。

注意：当前 Content Browser 动作会用生成内容替换选中的树。

## 31. 本地化流程

### 31.1 准备 Line ID

1. 打开 Dialogue Tree。
2. 确认每句台词都有稳定的 `LineID`。
3. 推荐格式：

```text
Intro_001
Intro_002
ShopGreeting_001
QuestAccept_001
```

4. 翻译开始后尽量不要修改 ID。

### 31.2 导出本地化 CSV

1. 右键 Dialogue Tree。
2. 选择：

```text
Export Chronicle Pipeline Artifacts...
```

3. 选择导出目录。
4. 把本地化 CSV 发给翻译。

### 31.3 导入翻译 CSV

使用编辑器辅助函数：

```text
Import Localization Csv To Tree
```

对应函数：

```cpp
UChronicleDialogueJsonLibrary::ImportLocalizationCsvToTree
```

推荐生产规则：

1. 翻译前锁定源文本 Line ID。
2. 导出本地化 CSV。
3. 只翻译目标文本列。
4. 先导入到副本树。
5. 校验。
6. 再合并进正式内容。

## 32. 校验 Dialogue Tree

校验可以发现断边、缺 Root、不可达节点、表达式错误等问题。

### 32.1 在编辑器中校验

1. 打开 `DT_Intro`。
2. 查看 Dialogue Tree 编辑器里的 validation summary 区域。
3. 修复 Warning 和 Error。
4. 保存。

### 32.2 通过 Blueprint 或 Editor Utility 校验

使用：

```text
Validate Dialogue Tree
```

对应函数：

```cpp
UChronicleDialogueJsonLibrary::ValidateDialogueTree
```

### 32.3 生成审计报告

使用：

```text
Build Dialogue Audit Report
```

对应函数：

```cpp
UChronicleDialogueAuditLibrary::BuildDialogueAuditReport
```

审计数据包括：

```text
节点数量
边数量
台词数量
选项数量
词数
断边数量
不可达节点数量
Warning 数量
Error 数量
角色台词统计
变量使用情况
校验问题列表
```

## 33. 运行自动化测试

运行命令行自动化前，请关闭 Unreal Editor。

### 33.1 运行完整验证脚本

在仓库根目录执行：

```powershell
.\Scripts\RunChronicleValidation.ps1
```

脚本会执行：

1. UE 5.3 Editor 编译。
2. UE 5.3 Chronicle 自动化测试。
3. UE 5.3 插件打包。
4. UE 5.7 兼容编译冒烟测试，除非跳过。
5. UE 5.7 冒烟后恢复 UE 5.3 编译。

### 33.2 只运行 UE 5.3 编译和测试

```powershell
.\Scripts\RunChronicleValidation.ps1 -SkipPackage -Skip57Smoke
```

### 33.3 只编译

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

### 33.4 直接运行自动化

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

正常结果：

```text
Automation tests complete
No Chronicle test failures
```

## 34. 打包插件

把 Chronicle Engine 打包给其他 Unreal 项目使用：

```powershell
.\Scripts\PackagePlugin.ps1
```

默认输出：

```text
Artifacts\ChronicleEngine-0.12.0-UE5.3
```

自定义包名：

```powershell
.\Scripts\PackagePlugin.ps1 -PackageName "ChronicleEngine-MyBuild-UE5.3"
```

自定义引擎路径：

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "D:\Epic\UE_5.3"
```

## 35. 在其他项目中安装打包插件

1. 打开打包结果：

```text
Artifacts\ChronicleEngine-0.12.0-UE5.3
```

2. 复制 `ChronicleEngine` 插件文件夹。
3. 粘贴到你的游戏项目：

```text
YourGameProject\Plugins\ChronicleEngine
```

4. 打开你的 `.uproject`。
5. 如果提示启用插件，确认启用。
6. 重建模块。
7. 确认 Content Browser 里能创建 Chronicle 资产类型。
8. 把 Dialogue Database、Dialogue Tree、Speaker Profile、Trigger 等资产复制或重新创建到目标项目。

源码接入也可以直接复制：

```text
Plugins\ChronicleEngine
```

到另一个项目，然后在那个项目里重新编译。

## 36. 推荐生产流程

真实项目建议按这个循环工作：

1. 在 Unreal Editor 中创建或修改 Dialogue Tree。
2. 添加或修订 Line ID。
3. 校验对话树。
4. 导出流水线文件。
5. 查看 Audit JSON，修复断边和不可达节点。
6. 把本地化 CSV 发给翻译。
7. 先把翻译导入到副本。
8. 运行自动化测试。
9. 用默认 Widget 在 PIE 中试玩。
10. 接入项目 UI、任务、镜头、音频、存档系统。
11. 做一次打包构建冒烟测试。
12. 只提交源码、内容、配置、脚本和文档。
13. 不提交 `Binaries`、`Intermediate`、`Saved`、生成的 `Artifacts`。

## 37. 常见问题排查

### 找不到 Chronicle 资产类型

1. 确认插件已启用。
2. 重新编译 `ChronicleHostEditor`。
3. 重启 Unreal Editor。
4. 检查 `ChronicleEngineEditor` 是否编译成功。

### 启动时出现模块重建弹窗

切换引擎版本或清理二进制后，这是正常现象。

1. 点击 **Yes**。
2. 如果失败，关闭 Unreal。
3. 执行第 3.2 节的手动编译命令。

### 图编辑器只显示左上角一小块

1. 重新停靠或最大化资产编辑器标签页。
2. 点击 Zoom To Fit。
3. 重置 Unreal 布局。
4. 必要时清理本地 `Saved/` 和 `Intermediate/` 缓存。

### 某个选项不显示

1. 检查它的 `Visibility Condition`。
2. 确认变量 Tag 存在。
3. 确认 Runner 里变量值正确。
4. 临时清空条件，确认边连接本身没问题。

### 条件分支没有执行

1. 检查边上的条件，不要只看节点字段。
2. 确认每个分支都有输出边 slot。
3. 用 `ValidateConditionExpressionForTree` 校验表达式。
4. 字符串值要加引号：

```text
Chronicle.Variable.Name == "Alice"
```

### 异步事件让对话停住

1. 确认真的需要 `bEventIsAsync`。
2. 确认玩法系统收到 `OnDialogueEvent`。
3. 确认玩法系统调用了 `NotifyEventComplete`。
4. 确认回传的是同一个 Gameplay Tag。

### UI 没显示

1. 确认 Widget 已创建。
2. 确认 Widget 已 Add to Viewport。
3. 确认 Widget 调用了 `BindPresentationController`。
4. 确认 Controller 调用了 `BindRunner`。
5. 确认对话确实已经 Start。

### UE 版本冒烟后自动化失败

重新跑一次 UE 5.3 编译：

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

## 38. 最小验收清单

每次完成一个对话功能前，用这个清单过一遍。

```text
[ ] Dialogue Tree 有且只有一个 Root 节点。
[ ] 每句 Speech 台词都有稳定 LineID。
[ ] Speaker Tag 能对应到 Speaker Profile。
[ ] Choice 分支连接到有效输出 slot。
[ ] 条件表达式已经校验。
[ ] Event Tag 已存在于 Gameplay Tags。
[ ] 异步事件会调用 NotifyEventComplete。
[ ] Dialogue Database 已引用这棵树。
[ ] Presentation Controller 已绑定 Runner。
[ ] Widget 已绑定 Presentation Controller。
[ ] 如果对话会跨存档边界，已测试 Save/Load。
[ ] 如果 UI 暴露回滚，已测试 Rollback。
[ ] 流水线文件可以成功导出。
[ ] 自动化测试通过。
```
