# Chronicle Engine

Chronicle Engine is an MIT-licensed Unreal Engine 5 plugin for JRPG-style dialogue and narrative systems. It ships as a source plugin plus a minimal host project for compiling, testing, and validating the full workflow.

- Current development version: `v0.9.0-dev`
- Latest packaged release: `v0.5.0`
- Primary engine baseline: UE 5.3
- Compatibility smoke target: UE 5.7
- Repository: `TodayYueC/ChronicleEngine`

---

# English Usage Guide

## 1. What Is Included

This repository contains:

- `ChronicleHost.uproject`: minimal Unreal host project.
- `Plugins/ChronicleEngine`: plugin source.
- `Documentation/`: roadmap, editor workflow, asset pipeline, presentation workflow, release notes, and release checklist.
- `Scripts/PackagePlugin.ps1`: local plugin packaging script.

Main capabilities:

- Dialogue Tree, Dialogue Database, Speaker Profile, and Dialogue Trigger assets.
- Runtime traversal through `UDialogueRunner`.
- Root, Speech, Choice, Condition, Event, Wait, Random, Jump, Sequence, SubDialogue, Camera, and Animation nodes.
- Global, Local, and External variable scopes.
- Recursive-descent condition expressions.
- Save/load, rollback, backlog, auto, and skip.
- GameplayTag event bus for camera, animation, audio, quest, game-state, battle, and scene events.
- `UChronicleExampleQuestAdapter` as a sample bridge from dialogue events to project quest/state systems.
- Native Slate Dialogue Tree editor with graph creation, links, Details editing, validation, search, copy/paste/duplicate/delete, breakpoints, debug snapshots, and soft locks.
- JSON tree import/export, CSV line import/export, localization gather/import, culture-specific voice-table lookup, validation, and audit reports.
- `UChronicleDialoguePresentationController`, `UChronicleDialogueWidget`, and `UChronicleDialogueDefaultWidget` for UI integration.
- Automation coverage for runtime, editor, presentation, pipeline, localization, audit, and integration flows.

## 2. Install

### Option A: Use A Release Package

1. Download `ChronicleEngine-0.5.0-UE5.3.zip` from GitHub Releases.
2. Extract it.
3. Copy the extracted plugin folder to:

```text
YourProject/Plugins/ChronicleEngine
```

4. Regenerate project files.
5. Open your UE project and enable `Chronicle Engine` in the Plugins panel.
6. Restart the editor.

Use Option B for the newest `main` branch features.

### Option B: Use The Source Repository

1. Clone this repository.
2. Open `ChronicleHost.uproject` with UE 5.3.
3. Build `ChronicleHostEditor`.
4. Test in the host project, or copy `Plugins/ChronicleEngine` into your own project.

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
- UE 5.3 `Chronicle` automation tests pass: 32/32.
- UE 5.7 build smoke passes.
- UE 5.3 BuildPlugin packaging passes for the latest packaged release.
- 100-node condition traversal budget: `0.25ms`; latest run: `0.0986ms`.

## 4. Create Assets

### Dialogue Tree

1. Right-click in the Content Browser.
2. Create a `Dialogue Tree` asset.
3. Double-click it to open the Chronicle Dialogue Tree editor.
4. Create a `Root` node, then connect it to dialogue nodes.

Common node types:

- `Root`: entry point.
- `Speech`: one or more dialogue lines.
- `Choice`: player choice list.
- `Condition`: branching by expression.
- `Event`: sends a GameplayTag event and string payload.
- `Wait`: pauses until input.
- `Random`: picks a valid outgoing edge by weight.
- `Jump`: moves to an entry in this tree or another tree.
- `SubDialogue`: enters another tree and can return to the caller.
- `Camera`: emits a camera cue.
- `Animation`: emits an animation cue.

### Speaker Profile

Speaker Profiles store speaker identity and presentation data:

- `SpeakerTag`
- display name
- portraits and full-body art
- voice table
- text color
- default position

### Dialogue Database

Dialogue Databases centralize:

- Speaker Profiles
- Dialogue Trees
- global variable definitions
- localization settings
- default and culture-specific voice tables

## 5. Edit Dialogue Trees

The custom editor supports:

- creating all PRD node types from the graph context menu
- dragging nodes and saving graph positions
- connecting pins to create edges
- breaking pins to delete edges
- copy, paste, duplicate, delete, and zoom-to-fit
- selected-node Details editing
- node search
- validation summaries
- breakpoints and debug snapshots
- Dialogue Tree / Dialogue Database soft-lock metadata

Recommended first graph:

1. Create `Root`.
2. Connect `Root` to `Speech`.
3. Add one or more `FDialogueLine` entries.
4. Add a `Choice` node for branching.
5. Add `FDialogueChoice` entries.
6. Connect each choice slot to a result node.
7. Run validation.

## 6. Condition Expressions

Conditions are used by Condition nodes, edge conditions, and Choice visibility.

Supported syntax:

- variables: `Chronicle.Variable.Score`
- booleans: `true`, `false`
- numbers: `10`, `3.14`
- strings: `"Alice"`
- comparisons: `==`, `!=`, `>`, `>=`, `<`, `<=`
- boolean operators: `AND`, `OR`, `NOT`, `&&`, `||`, `!`
- parentheses: `( ... )`

Examples:

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true)
```

```text
Chronicle.Variable.Name == "Alice" OR Chronicle.Variable.Score > 80
```

## 7. Runtime Integration

Blueprint and C++ callers should use `UChronicleDialogueSubsystem`.

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

Common runner API:

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

Common runner events:

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`

## 8. Dialogue Triggers

Create a `Dialogue Trigger` asset when dialogue should be activated by world interaction or events.

Trigger data:

- `TriggerTag`
- `TargetTree`
- `EntryNode`
- `TriggerType`: `Proximity`, `Interact`, `Auto`, or `Event`
- `ActivationConditions`
- `CooldownTime`
- `bOneShot`
- `Priority`

Use the subsystem trigger manager:

```cpp
UDialogueTriggerManager* TriggerManager = Subsystem->GetTriggerManager();
TriggerManager->TryActivateTriggerByTag(TriggerTag);
```

## 9. Variables

Set a variable:

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

Read a variable:

```cpp
bool bFound = false;
FVariableValue Value = Runner->GetVariable(ScoreTag, bFound);
```

`External` variables are resolved through native bindings on `UVariableBank`. A project system can also push values into the runner with `Global` or `Local` scope.

## 10. Event Bus And Project Adapters

Event nodes send:

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

If `bAsync` is true, the runner enters `WaitingForEvent`. Resume it with:

```cpp
Runner->NotifyEventComplete(EventTag);
```

Standard event tags include:

- `Chronicle.Event.Quest.Start`
- `Chronicle.Event.Quest.Update`
- `Chronicle.Event.Quest.Complete`
- `Chronicle.Event.GameState.Change`
- `Chronicle.Event.Actor.Animate`
- `Chronicle.Event.Battle.Encounter`
- `Chronicle.Event.Scene.Load`
- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

`UChronicleExampleQuestAdapter` shows how to bind to `OnDialogueEvent` and rebroadcast project-facing quest/state delegates.

```cpp
UChronicleExampleQuestAdapter* Adapter = NewObject<UChronicleExampleQuestAdapter>();
Adapter->BindToRunner(Runner);
```

The adapter recognizes quest, game-state, actor-animation, battle, and scene-load event tags. It also exposes `PushExternalVariable` so outside systems can feed runner variables.

## 11. UMG Presentation

Use `UChronicleDialoguePresentationController` as the bridge between runner and UI:

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

The controller handles:

- line forwarding
- choice forwarding
- UI-facing backlog
- Auto mode
- Skip mode
- Rollback
- presentation event forwarding

### Default Widget

Create a Widget Blueprint derived from `UChronicleDialogueDefaultWidget`, or instantiate it from C++.

```cpp
DefaultWidget->BindPresentationController(Presentation);
```

It provides:

- speaker text
- typewriter reveal state
- dialogue text
- choice buttons
- Backlog panel
- Advance, Auto, Skip, Backlog, and Back controls
- portrait and full-body image slots

### Custom Widget

1. Create a Widget Blueprint.
2. Set parent class to `UChronicleDialogueWidget`.
3. Call `BindPresentationController`.
4. Implement the Blueprint events you need:
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

## 12. JSON, CSV, Localization, And Audit

`UChronicleDialogueJsonLibrary` provides:

- `ExportDialogueTreeToJsonString`
- `ImportDialogueTreeFromJsonString`
- `ExportDialogueLinesToCsvString`
- `ImportDialogueLinesFromCsvString`
- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`
- `ValidateDialogueTree`

`UChronicleDialogueAuditLibrary` provides:

- `BuildDialogueAuditReport`
- `ExportDialogueAuditReportToJsonString`
- `ExportDialogueAuditReportForTreeToJsonString`

Audit reports include node/edge counts, speech line counts, choice counts, word counts, speaker line stats, variable usage, broken edges, unreachable nodes, and validation issue totals.

## 13. Demo Actor

`AChronicleDialogueDemoActor` builds a small runtime demo tree in source.

It demonstrates:

- camera cue
- voice ID
- multi-line speech
- choice
- presentation-controller startup

Usage:

1. Place `AChronicleDialogueDemoActor` in a level.
2. Keep `bStartOnBeginPlay = true`.
3. Run PIE.
4. Bind a Chronicle dialogue widget to the presentation controller.

## 14. Package The Plugin

Default UE 5.3 package:

```powershell
.\Scripts\PackagePlugin.ps1
```

Custom engine root:

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

Packages are written to `Artifacts/`, which is ignored by git.

## 15. Troubleshooting

### Missing modules or different engine version

Build `ChronicleHostEditor` with the engine version you are opening. This repository currently keeps UE 5.3 as the baseline, then smoke-tests UE 5.7.

### Gameplay Tags are missing

Check `Config/DefaultGameplayTags.ini`, or add equivalent tags in Project Settings.

### Choices do not appear

Check `VisibilityCondition`. Empty conditions are visible. Failed expressions or missing variables hide the choice.

### Dialogue does not continue after an async event

Call:

```cpp
Runner->NotifyEventComplete(EventTag);
```

### UMG does not receive updates

Confirm the widget called:

```cpp
BindPresentationController(PresentationController);
```

---

# 中文使用教程

## 1. 项目内容

本仓库包含一个最小 UE 宿主项目和 Chronicle Engine 插件源码。

- `ChronicleHost.uproject`：用于编译、测试和验证插件。
- `Plugins/ChronicleEngine`：插件源码。
- `Documentation/`：路线图、编辑器流程、资产管线、表现层流程、发布说明和发布检查表。
- `Scripts/PackagePlugin.ps1`：本地插件打包脚本。

主要能力：

- Dialogue Tree、Dialogue Database、Speaker Profile、Dialogue Trigger 资产。
- 通过 `UDialogueRunner` 运行对话。
- 支持 Root、Speech、Choice、Condition、Event、Wait、Random、Jump、Sequence、SubDialogue、Camera、Animation 节点。
- 支持 Global、Local、External 变量作用域。
- 自研递归下降条件表达式。
- 保存、加载、回滚、Backlog、Auto、Skip。
- 基于 GameplayTag 的事件总线，可接入镜头、动画、音频、任务、游戏状态、战斗和场景系统。
- `UChronicleExampleQuestAdapter` 作为任务/状态系统接入示例。
- 原生 Slate 对话树编辑器，支持节点创建、连线、Details 编辑、验证、搜索、复制、粘贴、复制副本、删除、断点、调试快照和软锁。
- JSON 导入导出、CSV 文本导入导出、本地化 gather/import、按语言查找语音表、结构验证和审计报告。
- `UChronicleDialoguePresentationController`、`UChronicleDialogueWidget`、`UChronicleDialogueDefaultWidget` 用于 UI 接入。
- 覆盖 Runtime、Editor、Presentation、Pipeline、Localization、Audit、Integration 的自动化测试。

## 2. 安装方式

### 方式 A：使用 Release 包

1. 从 GitHub Releases 下载 `ChronicleEngine-0.5.0-UE5.3.zip`。
2. 解压。
3. 把 `ChronicleEngine` 文件夹复制到：

```text
YourProject/Plugins/ChronicleEngine
```

4. 重新生成项目文件。
5. 打开 UE 项目，在 Plugins 面板启用 `Chronicle Engine`。
6. 重启编辑器。

如果需要 `main` 分支的最新功能，请使用方式 B。

### 方式 B：使用源码仓库

1. 克隆本仓库。
2. 使用 UE 5.3 打开 `ChronicleHost.uproject`。
3. 编译 `ChronicleHostEditor`。
4. 在宿主项目中测试，或把 `Plugins/ChronicleEngine` 复制到你的 UE 项目。

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
- UE 5.3 `Chronicle` 自动化测试通过：32/32。
- UE 5.7 编译冒烟通过。
- 最新打包版本的 UE 5.3 BuildPlugin 流程通过。
- 100 节点条件遍历预算：`0.25ms`；最新记录：`0.0986ms`。

## 4. 创建资产

### Dialogue Tree

1. 在 Content Browser 中右键。
2. 创建 `Dialogue Tree` 资产。
3. 双击打开 Chronicle 对话树编辑器。
4. 创建 `Root` 节点，再连接到其他对话节点。

常用节点：

- `Root`：对话入口。
- `Speech`：一行或多行对白。
- `Choice`：玩家选项。
- `Condition`：按表达式分支。
- `Event`：发送 GameplayTag 事件和字符串载荷。
- `Wait`：等待输入。
- `Random`：按权重选择可用输出边。
- `Jump`：跳转到当前树或其他树的入口。
- `SubDialogue`：进入另一个树，并可返回调用方。
- `Camera`：发送镜头 cue。
- `Animation`：发送动画 cue。

### Speaker Profile

Speaker Profile 保存角色表现信息：

- `SpeakerTag`
- 显示名
- 头像和立绘资源
- 语音表
- 文本颜色
- 默认位置

### Dialogue Database

Dialogue Database 集中管理：

- Speaker Profiles
- Dialogue Trees
- 全局变量定义
- 本地化设置
- 默认和分语言语音表

## 5. 编辑 Dialogue Tree

自定义编辑器支持：

- 从图右键菜单创建全部 PRD 节点类型
- 拖动节点并保存位置
- 连接 Pin 创建边
- 断开 Pin 删除边
- 复制、粘贴、复制副本、删除和缩放到合适视图
- 选中节点 Details 编辑
- 节点搜索
- 验证摘要
- 断点和调试快照
- Dialogue Tree / Dialogue Database 软锁元数据

推荐的第一棵树：

1. 创建 `Root`。
2. 连接 `Root` 到 `Speech`。
3. 添加一行或多行 `FDialogueLine`。
4. 如果需要分支，添加 `Choice`。
5. 添加 `FDialogueChoice`。
6. 把每个 choice slot 连接到结果节点。
7. 运行验证。

## 6. 条件表达式

Condition 节点、边条件和 Choice 可见性都使用 Chronicle 条件表达式。

支持语法：

- 变量：`Chronicle.Variable.Score`
- 布尔值：`true`、`false`
- 数字：`10`、`3.14`
- 字符串：`"Alice"`
- 比较：`==`、`!=`、`>`、`>=`、`<`、`<=`
- 布尔运算：`AND`、`OR`、`NOT`、`&&`、`||`、`!`
- 括号：`( ... )`

示例：

```text
(Chronicle.Variable.Score >= 50 AND Chronicle.Variable.Flag == true)
```

```text
Chronicle.Variable.Name == "Alice" OR Chronicle.Variable.Score > 80
```

## 7. Runtime 接入

Blueprint 和 C++ 都建议通过 `UChronicleDialogueSubsystem` 获取 Runner。

```cpp
UChronicleDialogueSubsystem* Subsystem = GameInstance->GetSubsystem<UChronicleDialogueSubsystem>();
UDialogueRunner* Runner = Subsystem->GetDialogueRunner();
Runner->StartDialogue(DialogueTree);
```

常用 Runner API：

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

常用事件：

- `OnDialogueStarted`
- `OnDialogueEnded`
- `OnLineStarted`
- `OnChoicesPresented`
- `OnDialogueEvent`
- `OnRunnerStateChanged`

## 8. Dialogue Trigger

当对话需要由世界交互或事件激活时，可以创建 `Dialogue Trigger` 资产。

Trigger 数据：

- `TriggerTag`
- `TargetTree`
- `EntryNode`
- `TriggerType`：`Proximity`、`Interact`、`Auto` 或 `Event`
- `ActivationConditions`
- `CooldownTime`
- `bOneShot`
- `Priority`

通过子系统获取 Trigger Manager：

```cpp
UDialogueTriggerManager* TriggerManager = Subsystem->GetTriggerManager();
TriggerManager->TryActivateTriggerByTag(TriggerTag);
```

## 9. 变量

设置变量：

```cpp
Runner->SetVariable(ScoreTag, FVariableValue::MakeInt(75));
```

读取变量：

```cpp
bool bFound = false;
FVariableValue Value = Runner->GetVariable(ScoreTag, bFound);
```

`External` 变量通过 `UVariableBank` 的原生绑定解析。项目系统也可以把值以 `Global` 或 `Local` 作用域推送到 Runner。

## 10. 事件总线与项目适配

Event 节点发送：

```cpp
FGameplayTag EventTag;
TMap<FName, FString> Payload;
bool bAsync;
```

如果 `bAsync` 为 true，Runner 会进入 `WaitingForEvent`。完成后调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

标准事件标签：

- `Chronicle.Event.Quest.Start`
- `Chronicle.Event.Quest.Update`
- `Chronicle.Event.Quest.Complete`
- `Chronicle.Event.GameState.Change`
- `Chronicle.Event.Actor.Animate`
- `Chronicle.Event.Battle.Encounter`
- `Chronicle.Event.Scene.Load`
- `Chronicle.Camera.Cut`
- `Chronicle.Camera.Blend`
- `Chronicle.Animation.Play`
- `Chronicle.Audio.PlayVoice`
- `Chronicle.Audio.StopVoice`

`UChronicleExampleQuestAdapter` 展示了如何绑定 `OnDialogueEvent`，并把事件转成项目侧的任务/状态委托。

```cpp
UChronicleExampleQuestAdapter* Adapter = NewObject<UChronicleExampleQuestAdapter>();
Adapter->BindToRunner(Runner);
```

适配器识别任务、游戏状态、角色动画、战斗和场景加载事件，也提供 `PushExternalVariable`，方便外部系统把变量写回 Runner。

## 11. UMG 表现层

使用 `UChronicleDialoguePresentationController` 作为 Runner 和 UI 的中间层：

```cpp
UChronicleDialoguePresentationController* Presentation = Subsystem->GetPresentationController();
```

控制器负责：

- 转发当前对白
- 转发可见选项
- 保存 UI Backlog
- Auto 模式
- Skip 模式
- 回滚
- 转发表现层事件

### 默认 Widget

创建父类为 `UChronicleDialogueDefaultWidget` 的 Widget Blueprint，或从 C++ 实例化。

```cpp
DefaultWidget->BindPresentationController(Presentation);
```

它提供：

- 说话人文本
- 打字机显示状态
- 对白文本
- 选项按钮
- Backlog 面板
- Advance、Auto、Skip、Backlog、Back 控件
- 头像和立绘槽位

### 自定义 Widget

1. 创建 Widget Blueprint。
2. 父类选择 `UChronicleDialogueWidget`。
3. 调用 `BindPresentationController`。
4. 按需实现 Blueprint 事件：
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

## 12. JSON、CSV、本地化与审计

`UChronicleDialogueJsonLibrary` 提供：

- `ExportDialogueTreeToJsonString`
- `ImportDialogueTreeFromJsonString`
- `ExportDialogueLinesToCsvString`
- `ImportDialogueLinesFromCsvString`
- `EnsureStableLineIds`
- `GatherDialogueTextsFromTree`
- `GatherDialogueTextsFromDatabase`
- `ExportLocalizationCsvFromTree`
- `ImportLocalizationCsvToTree`
- `ValidateDialogueTree`

`UChronicleDialogueAuditLibrary` 提供：

- `BuildDialogueAuditReport`
- `ExportDialogueAuditReportToJsonString`
- `ExportDialogueAuditReportForTreeToJsonString`

审计报告包含节点数、边数、对白行数、选项数、词数、说话人统计、变量使用情况、坏边、不可达节点和验证问题统计。

## 13. 示例 Actor

`AChronicleDialogueDemoActor` 会在源码中构建一个小型运行时 demo tree。

它展示：

- 镜头 cue
- VoiceID
- 多行对白
- 选项
- Presentation Controller 启动流程

使用方式：

1. 把 `AChronicleDialogueDemoActor` 放入关卡。
2. 保持 `bStartOnBeginPlay = true`。
3. 运行 PIE。
4. 将 Chronicle 对话 Widget 绑定到表现层控制器。

## 14. 打包插件

默认使用 UE 5.3：

```powershell
.\Scripts\PackagePlugin.ps1
```

指定引擎路径：

```powershell
.\Scripts\PackagePlugin.ps1 -EngineRoot "R:\UE\UE_5.7" -PackageName "ChronicleEngine-0.5.0-UE5.7"
```

输出目录为 `Artifacts/`，该目录已被 git 忽略。

## 15. 常见问题

### 启动时提示模块缺失或引擎版本不同

请使用当前要打开项目的 UE 版本编译 `ChronicleHostEditor`。本仓库当前以 UE 5.3 为基线，并用 UE 5.7 做兼容冒烟。

### 找不到 Gameplay Tags

检查 `Config/DefaultGameplayTags.ini`，或在项目设置中添加等价标签。

### Choice 没有显示

检查 `VisibilityCondition`。空条件表示可见；表达式失败或变量不存在会让选项隐藏。

### Async Event 之后对话不继续

调用：

```cpp
Runner->NotifyEventComplete(EventTag);
```

### UMG 没有收到更新

确认 Widget 已调用：

```cpp
BindPresentationController(PresentationController);
```

---

## Additional Documentation

- `Documentation/AssetPipeline.md`
- `Documentation/EditorWorkflow.md`
- `Documentation/IntegrationWorkflow.md`
- `Documentation/PresentationWorkflow.md`
- `Documentation/ReleaseNotes.md`
- `Documentation/ReleaseChecklist.md`

## License

Chronicle Engine is open source under the MIT License. See `LICENSE`.
