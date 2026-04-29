# Chronicle Engine

Chronicle Engine is an MIT-licensed Unreal Engine 5 plugin for JRPG-style dialogue and narrative systems. It provides a runtime dialogue runner, asset import/export pipeline, native editor graph workflow, UMG presentation foundation, automated tests, and release packaging support.

- Current development version: `v0.8.0-dev`
- Latest packaged release: `v0.5.0`
- Primary engine baseline: UE 5.3
- Compatibility smoke target: UE 5.7

---

# English Usage Guide

## 1. Repository Contents

This repository contains a minimal UE host project and the Chronicle Engine plugin source:

- `ChronicleHost.uproject`: minimal host project for compiling, testing, and validating the plugin.
- `Plugins/ChronicleEngine`: plugin source.
- `Documentation/`: roadmap, asset pipeline, editor workflow, presentation workflow, and release notes.
- `Scripts/PackagePlugin.ps1`: local packaging script.

Main features:

- Dialogue Tree / Dialogue Database / Speaker Profile assets.
- Runtime traversal through `UDialogueRunner`.
- Root, Speech, Choice, Condition, Event, Wait, Random, Jump, SubDialogue, Camera, and Animation nodes.
- Variables, condition expressions, save/load, and rollback.
- JSON import/export, CSV text import/export, localization gather/import, culture voice-table lookup, and structure validation.
- Native Slate graph editor, full PRD node creation, search, copy/paste/duplicate/delete, breakpoints, soft locks, and debug snapshots.
- `UChronicleDialoguePresentationController` for UI-facing flow control.
- `UChronicleDialogueWidget` as the default UMG base class.
- `UChronicleDialogueDefaultWidget` as a ready-to-use source-built dialogue HUD.
- Auto, Skip, Backlog, Rollback, and Choice forwarding.
- Camera / Animation / Audio presentation cues.
- Dialogue Trigger assets and `UDialogueTriggerManager` activation flow.
- Automation tests and BuildPlugin packaging.

## 2. Installation

### Option A: Use the GitHub Release Package

1. Download `ChronicleEngine-0.5.0-UE5.3.zip` from GitHub Releases. This is the latest packaged release; use Option B for the newest `main` branch features.
2. Extract it and copy the `ChronicleEngine` folder into your project:

```text
YourProject/Plugins/ChronicleEngine
```

3. Regenerate project files.
4. Open the UE project and confirm `Chronicle Engine` is enabled in the Plugins panel.
5. Restart the editor.

### Option B: Use the Source Repository

1. Clone this repository.
2. Open `ChronicleHost.uproject` with UE 5.3.
3. Build `ChronicleHostEditor`.
4. Test the plugin in the host project, or copy `Plugins/ChronicleEngine` into another UE project.
5. Use this option for the latest development version, currently `v0.8.0-dev`.

## 3. Build And Test

Build with UE 5.3:

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

Run automation tests:

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

Current verification status:

- UE 5.3 build passes.
- UE 5.3 `Chronicle` automation tests pass: 30/30.
- UE 5.7 build smoke passes.
- UE 5.3 BuildPlugin packaging passes.
- 100-node condition traversal budget: `0.25ms`; latest warm-path run: `0.0557ms`.

## 4. Create Core Assets

### Create A Dialogue Tree

1. Right-click in the Content Browser.
2. Select the Chronicle asset type.
3. Create a `Dialogue Tree`.
4. Double-click it to open the custom Dialogue Tree editor.

A Dialogue Tree stores dialogue nodes and edges.

Common node types:

- `Root`: dialogue entry point.
- `Speech`: dialogue lines.
- `Choice`: player choices.
- `Condition`: branching logic.
- `Event`: sends events to external systems.
- `Wait`: pauses the flow or waits for input.
- `Random`: chooses a valid outgoing edge by weight.
- `Jump`: moves to a target entry in the current or target tree.
- `SubDialogue`: enters another tree and can return to the caller.
- `Camera`: broadcasts a camera presentation cue.
- `Animation`: broadcasts an animation presentation cue.

### Create A Speaker Profile

Speaker Profile stores speaker data:

- `SpeakerTag`
- display name
- portrait set
- full body set
- voice table
- text color
- default position

### Create A Dialogue Database

Dialogue Database centralizes:

- Speaker Profiles
- Dialogue Trees
- global variable definitions
- localization settings
- voice table

Runtime systems can use it to initialize variables and look up dialogue resources.

## 5. Edit A Dialogue Tree

The custom editor supports:

- Creating all PRD node types from the context menu.
- Dragging nodes.
- Connecting pins to create edges.
- Breaking pins to remove edges.
- Copying, pasting, duplicating, and deleting selected nodes from the toolbar or graph shortcuts.
- Zooming the graph to fit the current tree or selected nodes.
- Editing selected nodes in Details.
- Searching for nodes.
- Validating broken edges, missing roots, unreachable nodes, and other structure issues.
- Setting breakpoint metadata.

Recommended first tree:

1. Create a `Root` node.
2. Connect `Root` to a `Speech` node.
3. Add one or more `FDialogueLine` entries.
4. Add a `Choice` node if branching is needed.
5. Add multiple `FDialogueChoice` entries.
6. Connect each choice slot to a different next node.
7. Run validation.

## 6. Condition Expressions

Condition nodes, edge conditions, and Choice visibility use Chronicle condition expressions.

Supported syntax:

- variable references: `Chronicle.Variable.Score`
- boolean literals: `true`, `false`
- numbers: `10`, `3.14`
- strings: `"Alice"`
- comparisons: `==`, `!=`, `>`, `>=`, `<`, `<=`
- boolean operators: `AND`, `OR`, `NOT`
- symbolic operators: `&&`, `||`, `!`
- parentheses: `( ... )`

Examples:

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true)
```

```text
Chronicle.Variable.Name == "Alice" OR Chronicle.Variable.Score > 80
```

Variables are managed by `UVariableBank`. Runtime scopes are `Global`, `Local`, and `External`.

## 7. Runtime Integration

Blueprint and C++ users should access the active runner through `UChronicleDialogueSubsystem`.

C++ example:

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

Trigger Manager access:

```cpp
UDialogueTriggerManager* TriggerManager = Subsystem->GetTriggerManager();
TriggerManager->TryActivateTriggerByTag(TriggerTag);
```

Common API:

- `Initialize`
- `StartDialogue`
- `Advance`
- `SelectChoice`
- `EndDialogue`
- `NotifyEventComplete`
- `SetVariable`
- `GetVariable`
- `SaveState`
- `LoadState`
- `PerformRollback`
- `GetTriggerManager`
- `TryActivateTrigger`
- `TryActivateTriggerByTag`
- `TryActivateBestTrigger`
- `EvaluateAutoTriggers`

Common events:

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`
- `OnTriggerActivated`
- `OnTriggerRejected`

### Dialogue Triggers

Create a `Dialogue Trigger` asset when the same Dialogue Tree needs world or event-based activation. A trigger stores:

- `TriggerTag`
- `TargetTree`
- `EntryNode`
- `TriggerType`: `Proximity`, `Interact`, `Auto`, or `Event`
- `ActivationConditions`
- `CooldownTime`
- `bOneShot`
- `Priority`

Register triggers with `UDialogueTriggerManager`, or activate a known trigger directly by Gameplay Tag. The manager evaluates conditions against the runner variable bank, blocks cooldown and consumed one-shot triggers, broadcasts `OnTriggerActivated`, then starts the target dialogue through the presentation controller when available.

## 8. Variables

Set a variable:

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

Read a variable:

```cpp
bool bFound = false;
FVariableValue Value = Runner->GetVariable(ScoreTag, bFound);
```

The same flow is available through BlueprintCallable methods.

## 9. Event Nodes

Event nodes send story events to your own systems, such as:

- camera cuts
- voice playback
- quest updates
- animation triggers
- async waits

Event payload:

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

If `bAsync` is true, the runner enters `WaitingForEvent`. Resume it by calling:

```cpp
Runner->NotifyEventComplete(EventTag);
```

## 10. UMG Presentation Layer

After M4, the recommended UI bridge is `UChronicleDialoguePresentationController`.

Get the controller:

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

The controller handles:

- current line forwarding
- visible choice forwarding
- UI-facing backlog
- Auto mode
- Skip mode
- Rollback
- Camera / Animation / Audio event forwarding

### Use The Default UMG Widget

For a source-only default HUD, create a Widget Blueprint with parent class `UChronicleDialogueDefaultWidget`, or instantiate that class directly from C++. The default widget can build its own runtime layout and already includes:

- speaker name text
- dialogue text with typewriter reveal state
- choice buttons
- Backlog panel
- Advance, Auto, Skip, Backlog, and Back buttons
- portrait and full-body image slots for project-specific speaker art

Bind it to the presentation controller:

```cpp
DefaultWidget->BindPresentationController(Presentation);
```

### Create A Custom UMG Widget

1. Create a Widget Blueprint.
2. Set its parent class to `UChronicleDialogueWidget`.
3. Call `BindPresentationController` after creating the widget.
4. Implement these Blueprint events:

- `On Dialogue Started`
- `On Dialogue Ended`
- `On Line Started`
- `On Line Completed`
- `On Choices Presented`
- `On Choice Selected`
- `On Waiting For Input`
- `On Rollback`
- `On Dialogue Event`
- `Handle Inline Tag`

UI buttons can call:

- `AdvanceDialogue`
- `SelectDialogueChoice`
- `SetAutoAdvanceEnabled`
- `SetSkipModeEnabled`
- `RequestRollback`

## 11. Auto, Skip, Backlog, Rollback

### Auto

Auto only advances while the runner is `WaitingForInput`. It does not auto-select choices.

```cpp
Presentation->SetAutoAdvanceEnabled(true, 1.5f);
```

### Skip

Skip fast-forwards through `WaitingForInput` lines until it reaches:

- Choice
- Async Event
- Dialogue End
- `MaxSkipStepsPerTick`

Skipped lines are presented with `ETextRevealMode::Instant`.

### Backlog

The Presentation Controller stores a UI-facing backlog:

```cpp
TArray<FDialogueHistoryEntry> Backlog = Presentation->GetBacklog();
```

### Rollback

Rollback through the Presentation Controller:

```cpp
Presentation->RequestRollback(1);
```

The runner stores rollback mementos only at player-visible pause points, not every internal condition hop.

## 12. Camera / Animation / Audio Cues

Default cue tags:

- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

Projects can listen to `OnDialogueEvent` or `OnPresentationEvent`, then forward payloads to their own camera, audio, quest, or animation systems.

Voice playback can also use:

```cpp
FDialogueLine::VoiceID
```

## 13. JSON / CSV Workflow

JSON is used for complete Dialogue Tree structure exchange. CSV is used for writing, text handoff, and localization.

Typical uses:

- Export JSON for review or external tooling.
- Import JSON back into a Dialogue Tree.
- Export CSV for writers or localization.
- Import CSV to update dialogue text.

See:

- `Documentation/AssetPipeline.md`

### Localization Gather

`UChronicleDialogueJsonLibrary` now exposes source-control-friendly localization helpers:

- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`

Gathered entries use `LineID` as the stable localization key. Missing or duplicate `LineID` values are repaired from `TreeGuid + NodeGuid + LineIndex`, so text edits do not change localization keys. `UDialogueDatabase` also supports `CultureVoiceTables` and `ResolveVoiceTableForCulture` for language-specific voice-table lookup.

## 14. Demo Actor

`AChronicleDialogueDemoActor` builds a small runtime demo tree that shows:

- Camera cue
- VoiceID
- multi-line speech
- Choice
- Presentation Controller startup

Usage:

1. Place `AChronicleDialogueDemoActor` in a level.
2. Keep `bStartOnBeginPlay = true`.
3. Run PIE.
4. Bind your `UChronicleDialogueWidget` to show UI.

## 15. Package The Plugin

Default UE 5.3 package:

```powershell
.\Scripts\PackagePlugin.ps1
```

Custom engine root:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

Output directory:

```text
Artifacts/
```

This directory is ignored by git.

## 16. Troubleshooting

### The plugin fails to compile

Confirm the project is using UE 5.3 or UE 5.7, then regenerate project files.

### Gameplay Tags are missing

Check `Config/DefaultGameplayTags.ini`, or add the required tags in Project Settings.

### Choices do not appear

Check `VisibilityCondition`. Empty conditions are visible; failed expressions or missing variables hide the choice.

### Dialogue does not continue after an async event

External systems must call:

```cpp
Runner->NotifyEventComplete(EventTag);
```

### UMG does not receive events

Confirm the widget called:

```cpp
BindPresentationController(PresentationController);
```

### Backlog is inconsistent after rollback

Call `RequestRollback` on the Presentation Controller instead of bypassing it and calling the Runner directly.

---

# 中文使用教程

## 1. 项目内容

本仓库包含一个最小 UE 宿主项目和 Chronicle Engine 插件源码：

- `ChronicleHost.uproject`：用于编译、测试和演示插件的最小宿主项目。
- `Plugins/ChronicleEngine`：插件源码。
- `Documentation/`：路线图、资产管线、编辑器、表现层和发布说明。
- `Scripts/PackagePlugin.ps1`：本地打包脚本。

插件主要能力：

- Dialogue Tree / Dialogue Database / Speaker Profile 数据资产。
- `UDialogueRunner` 运行时对话遍历。
- Root、Speech、Choice、Condition、Event、Wait、Random、Jump、SubDialogue、Camera、Animation 节点。
- 条件表达式、变量系统、保存加载、回滚。
- JSON 导入导出、CSV 文本导入导出、本地化 gather/import、按语言查找 VoiceTable、结构校验。
- 原生 Slate 图编辑器、完整 PRD 节点创建、节点搜索、复制/粘贴/复制副本/删除、断点、软锁、调试快照。
- `UChronicleDialoguePresentationController` 表现层控制器。
- `UChronicleDialogueWidget` UMG 基类。
- `UChronicleDialogueDefaultWidget` 可直接使用的源码 Dialogue HUD。
- Auto、Skip、Backlog、Rollback、Choice 转发。
- Camera / Animation / Audio 表现 cue。
- Dialogue Trigger 资产和 `UDialogueTriggerManager` 激活流程。
- 自动化测试和 BuildPlugin 打包。

## 2. 安装方式

### 方式 A：使用 GitHub Release 包

1. 从 GitHub Releases 下载 `ChronicleEngine-0.5.0-UE5.3.zip`。这是最新打包版本；如果需要 `main` 分支最新功能，请使用方式 B。
2. 解压后将 `ChronicleEngine` 文件夹复制到你的项目：

```text
YourProject/Plugins/ChronicleEngine
```

3. 重新生成项目文件。
4. 打开 UE 项目，在 Plugins 面板确认 `Chronicle Engine` 已启用。
5. 重启编辑器。

### 方式 B：从源码仓库使用

1. 克隆仓库。
2. 用 UE 5.3 打开 `ChronicleHost.uproject`。
3. 编译 `ChronicleHostEditor`。
4. 在宿主项目中测试插件或把 `Plugins/ChronicleEngine` 复制到其他 UE 项目。
5. 如需最新开发版，请使用此方式；当前开发版为 `v0.8.0-dev`。

## 3. 编译与测试

UE 5.3 编译：

```powershell
R:\UE\UE_5.3\Engine\Build\BatchFiles\Build.bat ChronicleHostEditor Win64 Development -Project="R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -WaitMutex
```

运行自动化测试：

```powershell
R:\UE\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "R:\AI_Agent\Codex\JRPGtalking\ChronicleHost.uproject" -Unattended -NullRHI -NoSplash -NoSound -ExecCmds="Automation RunTests Chronicle; Quit"
```

当前验证状态：

- UE 5.3 编译通过。
- UE 5.3 `Chronicle` 自动化测试 30/30 通过。
- UE 5.7 编译冒烟通过。
- UE 5.3 BuildPlugin 打包通过。
- 100 节点条件遍历测试预算：`0.25ms`；最新热路径实测 `0.0557ms`。

## 4. 创建基础数据资产

### 创建 Dialogue Tree

1. 在 Content Browser 中右键。
2. 选择 Chronicle 相关资产类型。
3. 创建 `Dialogue Tree`。
4. 双击打开自定义 Dialogue Tree 编辑器。

Dialogue Tree 用来保存具体对话节点和连接关系。

常用节点：

- `Root`：对话入口。
- `Speech`：对白行。
- `Choice`：玩家选项。
- `Condition`：条件分支。
- `Event`：向外部系统发送事件。
- `Wait`：等待输入或流程暂停。
- `Random`：按权重选择一条可用输出边。
- `Jump`：跳转到当前树或目标树中的入口节点。
- `SubDialogue`：进入子对话树，并可在结束后返回调用节点的后续分支。
- `Camera`：发送镜头表现 cue。
- `Animation`：发送动画表现 cue。

### 创建 Speaker Profile

Speaker Profile 用来描述说话人信息：

- `SpeakerTag`
- 显示名
- 头像集合
- 立绘集合
- 语音表
- 文本颜色
- 默认位置

### 创建 Dialogue Database

Dialogue Database 用来集中管理：

- Speaker Profiles
- Dialogue Trees
- 全局变量定义
- 本地化设置
- 语音表

运行时可以通过 Dialogue Database 初始化变量和查找对话资源。

## 5. 编辑 Dialogue Tree

打开 Dialogue Tree 后，可以使用自定义图编辑器：

- 右键创建全部 PRD 节点类型。
- 拖拽节点调整位置。
- 连接 Pin 创建边。
- 删除连接移除边。
- 通过工具栏或图快捷键复制、粘贴、复制副本和删除选中节点。
- 将图缩放到适合当前树或当前选区的视图。
- 在 Details 面板编辑选中节点。
- 使用搜索定位节点。
- 使用校验面板检查坏边、缺失 Root、不可达节点等问题。
- 给节点设置断点元数据。

推荐基本流程：

1. 创建 `Root` 节点。
2. 从 `Root` 连接到第一个 `Speech`。
3. 在 `Speech` 中添加一行或多行 `FDialogueLine`。
4. 如果需要分支，连接到 `Choice`。
5. 在 `Choice` 中添加多个 `FDialogueChoice`。
6. 每个 Choice slot 连接到不同后续节点。
7. 使用 Validation 检查结构。

## 6. 条件表达式

Condition 节点、边条件和 Choice 可见性都使用 Chronicle 条件表达式。

支持内容：

- 变量引用：`Chronicle.Variable.Score`
- 布尔字面量：`true`、`false`
- 数字：`10`、`3.14`
- 字符串：`"Alice"`
- 比较：`==`、`!=`、`>`、`>=`、`<`、`<=`
- 布尔运算：`AND`、`OR`、`NOT`
- 符号运算：`&&`、`||`、`!`
- 括号：`( ... )`

示例：

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true)
```

```text
Chronicle.Variable.Name == "Alice" OR Chronicle.Variable.Score > 80
```

变量默认由 `UVariableBank` 管理。Runtime 支持 `Global`、`Local`、`External` 三种作用域。

## 7. Runtime 接入

Blueprint 和 C++ 都建议通过 `UChronicleDialogueSubsystem` 获取当前 Runner。

C++ 示例：

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

Trigger Manager 接入：

```cpp
UDialogueTriggerManager* TriggerManager = Subsystem->GetTriggerManager();
TriggerManager->TryActivateTriggerByTag(TriggerTag);
```

常用 API：

- `Initialize`
- `StartDialogue`
- `Advance`
- `SelectChoice`
- `EndDialogue`
- `NotifyEventComplete`
- `SetVariable`
- `GetVariable`
- `SaveState`
- `LoadState`
- `PerformRollback`
- `GetTriggerManager`
- `TryActivateTrigger`
- `TryActivateTriggerByTag`
- `TryActivateBestTrigger`
- `EvaluateAutoTriggers`

常用事件：

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`
- `OnTriggerActivated`
- `OnTriggerRejected`

### Dialogue Trigger

当同一个 Dialogue Tree 需要由世界交互、任务事件或自动流程激活时，可以创建 `Dialogue Trigger` 资产。Trigger 保存：

- `TriggerTag`
- `TargetTree`
- `EntryNode`
- `TriggerType`：`Proximity`、`Interact`、`Auto` 或 `Event`
- `ActivationConditions`
- `CooldownTime`
- `bOneShot`
- `Priority`

把 Trigger 注册到 `UDialogueTriggerManager`，也可以用 Gameplay Tag 直接激活指定 Trigger。Manager 会用 Runner 的变量库评估条件，拦截冷却和已消耗的一次性 Trigger，广播 `OnTriggerActivated`，并在可用时通过表现层控制器启动目标对话。

## 8. 变量读写

设置变量：

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

读取变量：

```cpp
bool bFound = false;
FVariableValue Value = Runner->GetVariable(ScoreTag, bFound);
```

蓝图中可以通过对应 BlueprintCallable 方法完成同样操作。

## 9. Event 节点

Event 节点用于把剧情事件传递给项目自己的系统，例如：

- 镜头切换
- 播放语音
- 开启任务
- 触发动画
- 等待外部系统完成

事件载荷结构：

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

如果 `bAsync` 为 true，Runner 会进入 `WaitingForEvent`，外部系统完成后调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

## 10. UMG 表现层

M4 之后推荐使用 `UChronicleDialoguePresentationController` 作为 UI 和 Runner 之间的中间层。

获取控制器：

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

控制器负责：

- 转发当前对白行。
- 转发可见选项。
- 保存 UI backlog。
- 控制 Auto。
- 控制 Skip。
- 执行 Rollback。
- 转发 Camera / Animation / Audio 事件。

### 使用默认 UMG Widget

如果需要源码默认 HUD，可以创建一个父类为 `UChronicleDialogueDefaultWidget` 的 Widget Blueprint，也可以直接从 C++ 实例化这个类。默认 Widget 会自动构建基础布局，并包含：

- 说话人文本
- 带打字机状态的对白文本
- 选项按钮
- Backlog 面板
- Advance、Auto、Skip、Backlog、Back 按钮
- 头像和立绘图片槽，供项目接入自己的角色美术

绑定到表现层控制器：

```cpp
DefaultWidget->BindPresentationController(Presentation);
```

### 创建自定义 UMG Widget

1. 创建 Widget Blueprint。
2. 父类选择 `UChronicleDialogueWidget`。
3. 在 Begin Play 或创建后调用 `BindPresentationController`。
4. 实现这些 Blueprint 事件：

- `On Dialogue Started`
- `On Dialogue Ended`
- `On Line Started`
- `On Line Completed`
- `On Choices Presented`
- `On Choice Selected`
- `On Waiting For Input`
- `On Rollback`
- `On Dialogue Event`
- `Handle Inline Tag`

按钮可以绑定：

- `AdvanceDialogue`
- `SelectDialogueChoice`
- `SetAutoAdvanceEnabled`
- `SetSkipModeEnabled`
- `RequestRollback`

## 11. Auto、Skip、Backlog、Rollback

### Auto

Auto 模式只会在 Runner 处于 `WaitingForInput` 时自动推进，不会自动选择 Choice。

```cpp
Presentation->SetAutoAdvanceEnabled(true, 1.5f);
```

### Skip

Skip 模式会快速跳过 `WaitingForInput` 的对白，直到遇到：

- Choice
- Async Event
- Dialogue End
- `MaxSkipStepsPerTick`

Skip 中显示的文本会使用 `ETextRevealMode::Instant`。

### Backlog

Presentation Controller 会保存 UI-facing backlog：

```cpp
TArray<FDialogueHistoryEntry> Backlog = Presentation->GetBacklog();
```

### Rollback

回滚调用：

```cpp
Presentation->RequestRollback(1);
```

Runner 只在玩家可见暂停点保存回滚快照，避免内部条件跳转造成大量无意义 memento。

## 12. Camera / Animation / Audio Cue

默认 cue tag：

- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

项目可以监听 `OnDialogueEvent` 或 Presentation Controller 的 `OnPresentationEvent`，再把 payload 转发给自己的镜头、音频、任务或动画系统。

对白语音也可以直接使用：

```cpp
FDialogueLine::VoiceID
```

## 13. JSON / CSV 工作流

JSON 用于完整 Dialogue Tree 结构交换。CSV 用于文本、本地化和写作协作。

典型用途：

- 导出 JSON 做版本审查或外部工具处理。
- 从 JSON 重新导入 Dialogue Tree。
- 导出 CSV 给编剧或本地化人员。
- 导入 CSV 更新对白文本。

更多细节见：

- `Documentation/AssetPipeline.md`

### 本地化 Gather

`UChronicleDialogueJsonLibrary` 现在提供适合源码管理的本地化辅助 API：

- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`

Gather 结果使用 `LineID` 作为稳定本地化 Key。缺失或重复的 `LineID` 会根据 `TreeGuid + NodeGuid + LineIndex` 自动修复，因此修改对白正文不会改变本地化 Key。`UDialogueDatabase` 也新增了 `CultureVoiceTables` 和 `ResolveVoiceTableForCulture`，用于按语言查找语音表。

## 14. 示例 Actor

`AChronicleDialogueDemoActor` 会在运行时创建一个小型 demo tree，展示：

- Camera cue
- VoiceID
- 多行对白
- Choice
- Presentation Controller 启动流程

使用方式：

1. 在关卡中放置 `AChronicleDialogueDemoActor`。
2. 保持 `bStartOnBeginPlay = true`。
3. 运行 PIE。
4. 绑定自己的 `UChronicleDialogueWidget` 后即可显示 UI。

## 15. 打包插件

默认使用 UE 5.3：

```powershell
.\Scripts\PackagePlugin.ps1
```

指定 UE 版本：

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

输出目录：

```text
Artifacts/
```

该目录被 git 忽略。

## 16. 常见问题

### 插件启用后编译失败

确认 UE 版本为 5.3 或 5.7，并重新生成项目文件。

### 找不到 Gameplay Tag

确认 `Config/DefaultGameplayTags.ini` 中包含所需 tag，或在项目设置中添加对应 Gameplay Tags。

### Choice 没显示

检查 `VisibilityCondition`。空条件表示可见；表达式失败或变量不存在会导致不可见。

### Async Event 后对话不继续

外部系统必须调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

### UMG 没收到事件

确认 Widget 已调用：

```cpp
BindPresentationController(PresentationController);
```

### Backlog 回滚后不一致

通过 Presentation Controller 调用 `RequestRollback`，不要直接绕过表现层调用 Runner 回滚。

---

## Additional Documentation

- `Documentation/AssetPipeline.md`
- `Documentation/EditorWorkflow.md`
- `Documentation/PresentationWorkflow.md`
- `Documentation/ReleaseNotes.md`
- `Documentation/ReleaseChecklist.md`

## License

Chronicle Engine is open source under the MIT License. See `LICENSE`.
