# Chronicle Engine

Chronicle Engine is a UE5 JRPG narrative-system plugin planned from `01_PRD_项目需求文档.md`.

This repository contains:

- `ChronicleHost.uproject`: a minimal C++ host project for compiling and testing the plugin.
- `Plugins/ChronicleEngine`: the plugin source.
- `Documentation/Roadmap.md`: staged implementation notes and acceptance criteria.

## Current Milestone

M1 focuses on a runnable runtime vertical slice:

- dialogue data assets
- variable storage
- condition-expression evaluation
- dialogue runner traversal for Root, Speech, Choice, Condition, Event, and Wait nodes
- presenter-facing delegates
- inline tag parsing
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

