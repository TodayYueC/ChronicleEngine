# Chronicle Engine

Chronicle Engine is an MIT-licensed UE5 plugin for JRPG-style dialogue and narrative systems. It is being built as a production-oriented Runtime + Editor toolchain with a minimal host project for compilation, automated tests, and vertical-slice validation.

This repository contains:

- `ChronicleHost.uproject`: a minimal C++ host project for compiling and testing the plugin.
- `Plugins/ChronicleEngine`: the plugin source.
- `Documentation/Roadmap.md`: staged implementation notes and acceptance criteria.

## Current Milestone

M4 presentation workflow is complete on top of the runnable runtime, asset-pipeline, and editor foundation:

- dialogue data assets
- variable storage
- condition-expression evaluation
- dialogue runner traversal for Root, Speech, Choice, Condition, Event, and Wait nodes
- presenter-facing delegates
- presentation controller for UI-facing state and flow control
- abstract UMG widget base with Blueprint presentation events
- auto, skip, backlog, rollback, and choice forwarding flows
- camera and voice presentation cue tags
- source-generated demo dialogue actor
- inline tag parsing
- stable JSON export/import for `UDialogueTree`
- CSV export/import for dialogue-line text handoff
- structural validation for broken edges and unreachable nodes
- custom Dialogue Tree asset editor foundation
- native `SGraphEditor` dialogue graph with search selection, validation summary, toolbar and context-menu node creation, position persistence, Details editing, breakpoints, soft locks, and edge create/delete controls
- automation tests

## Build Baseline

- Primary: Unreal Engine 5.3
- Compatibility smoke target: Unreal Engine 5.7

## Local Commands

Use the installed UE 5.3 build:

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

Run automation tests:

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

See:

- `Documentation/AssetPipeline.md` for the current JSON, CSV, and validation workflow.
- `Documentation/EditorWorkflow.md` for the current Dialogue Tree editor workflow.
- `Documentation/PresentationWorkflow.md` for the current UMG and presentation-controller workflow.

## License

Chronicle Engine is open source under the MIT License. See `LICENSE`.
