# UE5 商业级 JRPG 叙事系统（Chronicle Engine）项目需求文档（PRD）

**版本**：v1.0  
**日期**：2026-04-25  
**状态**：初版设计  
**目标引擎**：Unreal Engine 5.3+（向后兼容至5.7）  
**分发形式**：Engine Plugin / Project Plugin（源码）  

---

## 1. 文档概述

### 1.1 项目背景
在JRPG（日式角色扮演游戏）开发中，文本/对话/剧情管理是最核心、最复杂的内容管线之一。业内现有方案（如Articy:draft X、Ink、Yarn Spinner）各有优劣，但对于需要**深度UE集成**、**原生可视化编辑**、**商业级表现层控制**（镜头、表情、语音、UI演出）的JRPG项目，现有方案存在以下痛点：

- **Articy**：外部工具，导入工作流复杂，实时调试困难，表现层控制依赖中间件。
- **Ink/Yarn**：文本脚本优先，策划/编剧学习成本高，无原生可视化分支编辑，JRPG特有的演出标签（表情、镜头）需大量自定义封装。
- **纯蓝图/数据表**：硬编码倾向严重，分支逻辑超过二叉后维护噩梦，复用性为零。

### 1.2 项目目标
构建一套**抽离于具体项目**的UE5插件/模板系统，代号 **Chronicle Engine**，提供：
- **可视化节点图编辑器**（策划友好，零代码维护剧情）
- **数据驱动的运行时引擎**（运行时动态调整，零代码热重载）
- **JRPG专用表现层管线**（文本滚动、头像、立绘、表情、语音、镜头、场景切换）
- **企业级扩展接口**（任务系统、状态机、保存系统的松耦合联动）

### 1.3 适用范围
- **游戏类型**：JRPG、视觉小说、剧情向AVG、带重剧情的RPG/ARPG
- **复用场景**：多项目间复用，通过插件形式安装
- **用户角色**：叙事设计师/策划、技术策划、UI程序、系统程序

### 1.4 竞品对标

| 维度 | Articy:draft X | Ink + Inkpot | Yarn Spinner UE | **Chronicle Engine (本方案)** |
|---|---|---|---|---|
| 可视化编辑 | 极强（独立软件） | 弱（文本脚本） | 弱（文本脚本） | **强（UE内嵌节点图）** |
| UE原生集成 | 中等（需导入） | 良好（Inkpot封装） | 弱（Beta且归档） | **原生（C++/蓝图）** |
| 条件/变量 | 完整 | 完整（Ink语法） | 完整（Yarn语法） | **完整（蓝图+表达式）** |
| 本地化 | 完整 | 需手动集成 | 内置 | **原生FText管线** |
| JRPG表现层 | 需二次开发 | 需大量封装 | 需大量封装 | **内置专用管线** |
| 保存/回滚 | 需自行实现 | Inkpot支持序列化 | 内存变量无保存 | **内置Memento回滚** |
| 可视化调试 | 无 | Inkpot Blotter | 无 | **内置运行时调试器** |
| 任务系统联动 | 有（QuestFlow） | 无（事件派发） | 无 | **GameplayTag事件总线** |
| 成本 | 高（商业授权） | 免费（MIT） | 免费（MIT） | **自有资产** |

---

## 2. 术语表

| 术语 | 定义 |
|---|---|
| **Dialogue Database** | 叙事数据库资产，包含角色定义、变量模板、本地化表、对话树集合 |
| **Dialogue Tree** | 单段对话/剧情的节点图，类似Behavior Tree的流式结构 |
| **Node Graph Editor** | UE Slate实现的自定义节点图编辑器，用于可视化编辑Dialogue Tree |
| **Dialogue Runner** | 运行时核心，负责遍历节点图、评估条件、分发事件、驱动表现层 |
| **Variable Bank** | 全局/局部变量存储容器，支持多种数据类型与外部绑定 |
| **Presenter** | 表现层接口（IDialoguePresenter），负责将对话数据转化为屏幕呈现 |
| **Memento Stack** | 对话状态备忘录栈，用于历史回滚与保存加载 |
| **Event Dispatcher** | 事件总线，通过GameplayTag或动态委托将剧情事件广播给游戏系统 |
| **Speaker Profile** | 说话人档案，包含名称、头像、立绘、语音音色、表情映射表等 |
| **Line Segment** | 单行对话的细分单元，支持文本标签（速度、颜色、震动、插入图片等） |

---

## 3. 系统架构总览

### 3.1 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│  PRESENTATION LAYER (表现层)                                 │
│  UMG Widgets · DialogueHUD · PortraitManager · CameraDirector │
│  文本滚动器 · 选项按钮 · 头像/立绘组件 · 字幕条 · 历史日志面板      │
├─────────────────────────────────────────────────────────────┤
│  RUNTIME LAYER (运行时层)                                    │
│  UDialogueRunner · UVariableBank · UDialogueMemento        │
│  节点遍历引擎 · 条件评估器 · 事件分发器 · 序列化/反序列化          │
├─────────────────────────────────────────────────────────────┤
│  DATA LAYER (数据层)                                         │
│  UDialogueDatabase · UDialogueTree · UDialogueNode (派生)   │
│  USpeakerProfile · FDialogueText · FConditionExpression     │
├─────────────────────────────────────────────────────────────┤
│  EDITOR LAYER (编辑器层)                                     │
│  DialogueGraphSchema · SDialogueGraphEditor · AssetActions  │
│  节点图绘制 · 连线逻辑 · 属性面板 · 搜索/断点/调试面板            │
├─────────────────────────────────────────────────────────────┤
│  PIPELINE LAYER (管线层)                                     │
│  CSV/XLSX Import · JSON Export · Localization Gathering     │
│  外部表格导入 · Articy格式适配器 · 语音表同步                   │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 核心设计原则

1. **数据与表现严格分离**：运行时引擎只输出"当前该显示什么文本、提供什么选项、触发什么事件"，绝不直接操作UMG或摄像机。
2. **策零代码原则**：所有剧情内容（分支、条件、事件参数）应在编辑器内配置完成，无需C++或蓝图编译。
3. **热重载友好**：对话树资产在编辑器内修改后，PIE（Play In Editor）无需重启即可生效（通过UAsset重载监听）。
4. **松耦合联动**：通过GameplayTag事件总线与任务、战斗、动画系统通信，不直接引用具体系统类。
5. **多项目复用**：所有类型定义、基类、接口在插件内完成，项目层仅做子类化或蓝图实现。

---

## 4. 功能需求详述

### 4.1 数据层（Data Layer）

#### FR-DL-01 叙事数据库资产（UDialogueDatabase）
- **描述**：每个项目可拥有多个Database（例如按章节、按城镇分库），也可使用单一主库。
- **必须包含**：
  - `SpeakerProfiles`（角色档案数组）
  - `GlobalVariables`（全局变量定义模板）
  - `DialogueTrees`（对话树资产引用数组）
  - `LocalizationSettings`（本地化配置：目标语言、Namespace规则、语音表路径）
  - `VoiceTable`（语音映射表：LineID → SoundCue/AKEvent路径）
- **接口**：支持通过GameplayTag快速查询SpeakerProfile和Variable。

#### FR-DL-02 对话树资产（UDialogueTree）
- **描述**：独立的UAsset，内部存储节点图数据（非UObject节点，而是纯数据结构，避免UObject数量爆炸）。
- **数据结构**：
  - `Nodes: TArray<FDialogueNode>`，每个节点含唯一GUID、节点类型枚举、位置坐标、属性Payload（TSharedPtr<FJsonObject>或自定义UStruct）、输入/输出插槽定义。
  - `Edges: TArray<FDialogueEdge>`，含FromGUID、ToGUID、FromSlotIndex、ToSlotIndex、条件表达式字符串（可选）。
  - `RootNodeGUID`：入口节点。
  - `Variables: TArray<FVariableDefinition>`：本对话树局部变量。
- **版本控制友好**：资产使用UE标准UAsset序列化，支持Diff；同时提供导出为JSON/YAML的右键菜单，便于Git文本Diff。

#### FR-DL-03 角色档案（USpeakerProfile）
- **描述**：定义游戏中的说话人。
- **属性**：
  - `SpeakerTag: FGameplayTag`（唯一标识，如 `Speaker.Hero.Alice`）
  - `DisplayName: FText`（显示名称，支持本地化）
  - `PortraitSet: TMap<FName, TSoftObjectPtr<UTexture2D>>`（头像/表情映射，如 Normal, Happy, Angry）
  - `FullBodySet: TMap<FName, TSoftObjectPtr<UTexture2D>>`（立绘/全身表情映射）
  - `VoiceSet: TSoftObjectPtr<UDataTable>`（音色/语音事件表）
  - `TextColor: FLinearColor`（对话文本颜色标识）
  - `DefaultPosition: ESpeakerPosition`（屏幕位置：Left, Center, Right, OffScreen）

#### FR-DL-04 文本行数据结构（FDialogueLine）
- **描述**：对话节点的核心内容单元。
- **属性**：
  - `LineID: FName`（唯一标识，自动生成 `TreeName_NodeGUID_LineIndex`）
  - `SpeakerTag: FGameplayTag`
  - `Text: FText`（UE原生FText，自动接入Localization系统）
  - `EmotionTag: FGameplayTag`（驱动Portrait/FullBody切换与动画）
  - `VoiceID: FName`（关联VoiceTable）
  - `MetaTags: TArray<FGameplayTag>`（额外标签，如 `Meta.Whisper`, `Meta.Shout`）
  - `CameraDirective: FCameraDirective`（镜头指令，可选）
  - `WaitTime: float`（自动推进等待时间，-1表示等待输入）
  - `Segments: TArray<FLineSegment>`（富文本分段，支持内嵌标签）

#### FR-DL-05 富文本标签系统（FLineSegment / Inline Tags）
- **描述**：策划可在文本中插入标签以控制表现，无需程序介入。
- **支持标签语法**（类似BBCode，在编辑器内提供自动补全）：
  - `[speed=0.05]` 调整后续文字出现速度
  - `[color=#FF0000]` 局部变色
  - `[shake]` 文本震动效果
  - `[wait=1.0]` 停顿
  - `[portrait=Angry]` 切换当前说话人表情
  - `[camera=CloseUp]` 触发镜头指令
  - `[sfx=HeartBeat]` 播放音效
  - `[event=AddItem.Potion]` 触发内联事件（慎用，主要用于简单反馈）
- **解析器**：运行时由 `UDialogueTextParser` 将原始字符串解析为 `TArray<FLineSegment>`，每个Segment含纯文本+属性集合。

#### FR-DL-06 对话触发器资产（UDialogueTrigger）
- **描述**：定义对话在游戏世界中的激活条件，与对话树解耦。同一对话树可被多个触发器引用，触发器负责检测"何时何地由谁触发"。
- **属性**：
  - `TriggerTag: FGameplayTag`（唯一标识，如 `Trigger.Town.AliceMorning`）
  - `TargetTree: TSoftObjectPtr<UDialogueTree>`（触发后执行的对话树）
  - `EntryNode: FName`（可选：进入对话树的指定入口节点，默认Root）
  - `TriggerType: EDialogueTriggerType`
    - `Proximity`：玩家进入碰撞体（Sphere/Box）触发
    - `Interact`：玩家面对NPC按交互键（E键）触发
    - `Auto`：关卡加载/任务到达某阶段后自动触发
    - `Event`：外部事件触发（如战斗胜利后、动画播完后）
  - `ActivationConditions: TArray<FConditionExpression>`：额外条件（如仅白天、需持有某物品）
  - `CooldownTime: float`：触发冷却（防止重复触发）
  - `bOneShot: bool`：是否仅触发一次（自动记录到SaveGame）
  - `Priority: int32`：当多个触发器同时满足时，高优先级优先执行
- **运行时接口**：`UDialogueTriggerManager` 管理世界内所有激活的触发器，每帧评估条件，通过委托通知Runner启动对话。

---

### 4.2 编辑器层（Editor Layer）

#### FR-ED-01 自定义节点图编辑器
- **描述**：在UE内容浏览器中双击 `UDialogueTree` 资产后，打开自定义Slate节点图Tab。
- **UI参考**：Behavior Tree编辑器 + Material Editor的混合体验。
- **必须功能**：
  - 画布平移/缩放（支持鼠标中键拖拽、滚轮缩放）
  - 右键菜单创建节点（节点类型分类树）
  - 节点拖拽移动，自动吸附网格
  - 插槽连线（输出→输入），支持多输出（条件分支、选择分支）
  - 连线上显示条件标签（当Edge附加条件时）
  - 框选/多选/复制/粘贴/删除（支持Ctrl+C/V）
  - 缩略图导航面板（Minimap）
  - 搜索面板（按节点内容、Speaker、GUID搜索并定位）
  - 断点标记（在节点上右键Toggle Breakpoint，PIE时在此节点暂停）

#### FR-ED-02 节点类型体系
所有节点在编辑器中以不同颜色和图标区分。

| 节点类型 | 颜色 | 核心功能 | 输入/输出 |
|---|---|---|---|
| **Root** | 绿色 | 对话树入口 | 0输入 / 1输出 |
| **Speech** | 蓝色 | 说话人发言（可含多行） | 1输入 / 1输出 |
| **Choice** | 橙色 | 玩家选择分支 | 1输入 / N输出（每选项一个） |
| **Condition** | 紫色 | 条件判断，多路分支 | 1输入 / N输出（每条件一个，含Default） |
| **Event** | 红色 | 触发游戏事件 | 1输入 / 1输出（事件后可继续） |
| **Wait** | 灰色 | 等待时间或输入 | 1输入 / 1输出 |
| **Random** | 青色 | 随机选择分支 | 1输入 / N输出（带权重） |
| **Jump** | 黄色 | 跳转到本树或其他树的指定节点 | 1输入 / 0输出（逻辑跳转） |
| **Sequence** | 白色 | 线性组合多个子节点（语法糖） | 1输入 / 1输出 |
| **SubDialogue** | 靛蓝 | 嵌套引用另一段对话树 | 1输入 / 1输出（子树返回后） |
| **Camera** | 粉色 | 纯镜头指令节点 | 1输入 / 1输出 |
| **Animation** | 棕色 | 角色动画/表情指令节点 | 1输入 / 1输出 |

#### FR-ED-03 属性细节面板（Details Customization）
- **描述**：选中节点后，Details面板显示该节点的可配置属性。
- **特殊控件**：
  - `FText` 属性使用多行文本框，带 `[Tag]` 插入辅助按钮。
  - `FGameplayTag` 属性使用UE原生GameplayTag选择器。
  - `FConditionExpression` 使用自定义表达式编辑器（支持变量名自动补全、运算符高亮）。
  - `TSoftObjectPtr` 使用资产选择器，支持过滤类型。
  - 数组属性支持拖拽排序、复制条目。

#### FR-ED-04 条件表达式编辑器
- **描述**：Condition节点和Choice节点的可见性条件需要可视化编辑。
- **语法示例**：
  ```
  Global.Quest.MainStage >= 3 AND Speaker.Likeness.Alice > 50
  ```
  ```
  Inventory.HasItem("Potion") OR (Party.MemberCount == 1)
  ```
- **编辑器功能**：
  - 左侧变量浏览器（列出Global/Local/External可用变量）
  - 中间表达式输入框（带语法高亮：变量名蓝色、运算符红色、字符串绿色）
  - 右侧实时验证（显示当前表达式的返回值类型与错误提示）
  - 简易模式：下拉框选择变量+运算符+常量（面向非技术策划）
  - 高级模式：自由输入表达式（面向技术策划）

#### FR-ED-05 断点与调试面板
- **描述**：在编辑器内提供Dialogue Debugger窗口（类似Blueprint Debugger）。
- **功能**：
  - PIE时实时显示当前激活节点（高亮+脉冲动画）
  - 显示当前Variable Bank的全部变量值（只读/可写）
  - 显示最近N条执行历史（节点路径）
  - 支持Step Into / Step Over / Continue
  - 支持运行时修改变量值并重新评估当前分支
  - 支持强制跳转到任意节点（Goto Node）

#### FR-ED-06 资产管道与导入导出
- **CSV/Excel导入**：策划可在Excel中撰写对话文本，按固定列格式（LineID, Speaker, Text, Emotion, NextLineID, Condition...）编写，通过右键资产→Import CSV覆盖或追加到现有对话树。
- **JSON导出**：支持将对话树导出为JSON，供外部工具（如语音录制管理工具、翻译公司）使用。
- **Articy适配器（可选Phase 2）**：提供Articy JSON/XML导入器，将Articy导出的数据转换为UDialogueTree。

#### FR-ED-07 多人协作与内容锁定机制
- **描述**：商业项目通常有多个策划/编剧同时编辑剧情内容。UAsset是二进制格式，Git合并冲突难以解决，需提供工作流支持。
- **功能设计**：
  - **Database级锁定**：双击打开Database时，在临时文件（如 `.chronicle/locks/`）中写入Lock文件，记录编辑者姓名与机器名；其他用户打开时显示只读模式（类似Perforce Checkout）。
  - **Tree级粒度锁定**：在Database编辑器内，可对单个DialogueTree标记"正在编辑"，允许多人同时编辑同一Database下的不同Tree。
  - **只读模式**：锁定状态下可查看、可预览、可Debugger调试，但禁止保存。
  - **变更提示**：若底层UAsset被版本控制更新（如Git Pull），编辑器内提示"底层资产已变更，建议关闭重载"。
  - **JSON/YAML并行工作流（推荐）**：核心对话数据支持导出为可读文本格式（JSON），策划可在文本编辑器中修改后重新导入。文本格式更利于Git Diff和多人并行编辑不同Tree。
- **注**：UE原生不支持文件级版本控制锁定（除非搭配Perforce/SVN），本机制为**软性锁定**（基于文件/共享目录的约定），用于提醒而非强制阻断。

---

### 4.3 运行时层（Runtime Layer）

#### FR-RT-01 对话执行引擎（UDialogueRunner）
- **描述**：单例（ per-world 或 per-GameInstance ）运行器，驱动对话流程。
- **生命周期**：
  1. `Initialize(UDialogueDatabase*)`：加载数据库，初始化Variable Bank。
  2. `StartDialogue(UDialogueTree*, FName EntryNode = NAME_None)`：开始一段对话，压入新Memento。
  3. `Tick(float DeltaTime)`：每帧驱动（处理等待时间、输入检测）。
  4. `Advance()`：推进到下一个逻辑节点（内部自动处理Speech/Wait/Event等）。
  5. `SelectChoice(int32 ChoiceIndex)`：玩家选择后跳转。
  6. `EndDialogue()`：结束对话，弹出Memento，触发OnDialogueEnded。
- **状态机**：
  - `Idle`：未运行
  - `Running`：正常遍历
  - `WaitingForInput`：等待玩家按键/点击推进
  - `WaitingForChoice`：等待玩家选择选项
  - `WaitingForEvent`：等待外部事件完成（如场景切换、动画播完）
  - `Paused`：被系统暂停（如进入战斗）

#### FR-RT-02 变量与条件系统（UVariableBank）
- **描述**：支持运行时动态创建、读取、修改变量，所有修改记录到Memento以便回滚。
- **数据类型**：
  - `Bool`, `Int32`, `Float`, `String`, `Name`, `GameplayTag`
  - `Vector`（用于位置/颜色类逻辑）
  - `List`（Ink风格列表，如 `Inventory.Potions = [Red, Blue]`）
- **作用域**：
  - **Global**：绑定到SaveGame，跨对话持久。
  - **Local**：当前DialogueTree内有效，对话结束自动销毁。
  - **External**：不直接存储值，而是通过委托实时从外部系统（如PlayerState、QuestManager）读取/写入。
- **外部绑定接口**：
  ```cpp
  DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FOnEvaluateExternalBool, 
      FGameplayTag, VariableTag, bool, OutValue, bool&, bFound, UObject*, Context);
  // 项目层通过UDialogueRunner::BindExternalVariable(...) 注册回调
  ```

#### FR-RT-03 分支逻辑执行
- **Choice节点**：输出N个选项，每个选项可附加可见性条件（`VisibilityCondition`）。运行时只将条件为真的选项呈现给Presenter层。若只有一个选项且配置`AutoSelectIfSingle = true`，则自动选择。
- **Condition节点**：按输出Edge的顺序评估条件，进入第一个条件为真的分支；若无，进入Default分支（最后一个输出）。
- **Random节点**：按输出Edge的`Weight`值做加权随机。支持设置`SeedSource`（全局/局部/固定Seed）以保证可复现。
- **Jump节点**：支持跳转到同一Tree内的节点，或跨Tree跳转（自动结束当前树并启动目标树）。Jump可附带`ReturnBookmark`，用于SubDialogue返回。

#### FR-RT-04 事件触发系统
- **描述**：Event节点和Speech节点内嵌的`[event]`标签通过事件总线广播。
- **事件格式**：
  - `GameplayTag` 作为主标识，如 `Event.Quest.Update`, `Event.Battle.Start`, `Event.Camera.PlayShot`
  - `FJsonObject` 或 `TMap<FName, FString>` 作为参数载荷。
- **分发方式**：
  - **同步事件**：立即执行，节点在事件回调返回后继续（适用于简单状态修改）。
  - **异步事件**：节点进入 `WaitingForEvent` 状态，直到外部系统调用 `UDialogueRunner::NotifyEventComplete(FGameplayTag EventTag)` 后才继续（适用于场景切换、长动画、战斗）。
- **蓝图暴露**：所有事件通过Dynamic Multicast Delegate广播，蓝图可直接Bind。

#### FR-RT-05 保存、加载与历史回滚（Memento系统）
- **Memento数据结构**：
  ```cpp
  USTRUCT()
  struct FDialogueMemento {
      GENERATED_BODY()
      UPROPERTY() FDateTime Timestamp;
      UPROPERTY() TMap<FName, FVariableSnapshot> VariableSnapshots;
      UPROPERTY() FGuid CurrentNodeGUID;
      UPROPERTY() FGuid CurrentTreeGUID;
      UPROPERTY() int32 CurrentLineIndex;
      UPROPERTY() TArray<FDialogueHistoryEntry> History; // 已播放的文本记录
  };
  ```
- **保存/加载**：
  - `SaveToSlot(FString SlotName)`：将当前完整的Variable Bank + Memento Stack序列化为SaveGame对象。
  - `LoadFromSlot(FString SlotName)`：反序列化并恢复到指定状态。
- **历史回滚**：
  - 运行时自动维护一个 `HistoryStack`（上限可配置，如50条）。
  - 玩家按下"历史/回滚"键（或点击UI按钮）时，弹出当前Memento，恢复到上一个Memento状态。
  - 回滚后，Presenter层需重新渲染历史文本（类似视觉小说的Backlog+Rollback）。
  - 支持配置`bAllowRollbackDuringChoice`：在选择分支界面是否允许回滚。
- **历史日志（Backlog）**：单独维护只读历史数组，用于UI的回溯查看，不回滚时清空。

#### FR-RT-06 Skip（快进已读）与 Auto（自动播放）系统
- **描述**：JRPG必备的玩家体验功能。系统需追踪哪些对话文本已被玩家阅读过，从而允许快进。
- **Seen Text 追踪**：
  - 以 `TreeGUID + NodeGUID + LineID` 组合生成唯一Hash，存储于SaveGame的 `SeenDialogueHashes` 集合中。
  - 每次Line播放完毕且非Skip状态下，标记为已读。
- **Skip模式**：
  - 玩家按住Skip键（如Ctrl / R1 / 鼠标右键）时，Runner进入 `Skipping` 状态。
  - 若当前Line的Hash在Seen集合中：瞬显当前Line，并自动Advance到下一节点；持续快进直到遇到未读文本或Choice节点。
  - 若当前Line未读：Skip键无效（或仅加速打字机），防止首次游玩剧透。
  - Skip过程中可随时释放按键停止。
- **Auto模式**：
  - 玩家开启Auto后，每条Line播放完毕后自动等待 `AutoWaitTime`（可按Line配置，如 `Line.AutoWaitTime = 2.0f`，默认按字数计算）后自动Advance。
  - Choice节点处Auto暂停，等待玩家选择（或配置 `bAutoSelectFirstChoice = false`）。
- **配置项**：
  - `bSkipReadTextOnly: bool`（默认true，仅快进已读）
  - `SkipSpeedMultiplier: float`（快进时打字机倍速，如10x）
  - `AutoWaitBaseTime: float`（Auto模式基础等待秒数）
  - `AutoWaitPerChar: float`（Auto模式每字符额外等待，模拟阅读速度）

---

### 4.4 表现层框架（Presentation Layer）

#### FR-PL-01 表现层接口（IDialoguePresenter）
- **描述**：所有UI/演出逻辑通过实现此接口与运行时解耦。
- **核心方法**：
  ```cpp
  UINTERFACE()
  class IDialoguePresenter {
      // 开始/结束对话
      virtual void OnDialogueStarted(UDialogueTree* Tree) = 0;
      virtual void OnDialogueEnded(UDialogueTree* Tree) = 0;
      
      // 显示/隐藏说话人
      virtual void OnSpeakerShown(const FSpeakerDisplayInfo& Info) = 0;
      virtual void OnSpeakerHidden(FGameplayTag SpeakerTag) = 0;
      
      // 文本推进（支持逐字/逐词/瞬显）
      virtual void OnLineStarted(const FDialogueLine& Line, ETextRevealMode RevealMode) = 0;
      virtual void OnLineCompleted(const FDialogueLine& Line) = 0;
      
      // 选项呈现
      virtual void OnChoicesPresented(const TArray<FDialogueChoice>& Choices) = 0;
      virtual void OnChoiceSelected(int32 ChoiceIndex) = 0;
      
      // 等待状态
      virtual void OnWaitingForInput() = 0;
      
      // 历史回滚
      virtual void OnRollback(const TArray<FDialogueHistoryEntry>& HistorySnapshot) = 0;
      
      // 内联标签处理（由Presenter自行实现视觉效果）
      virtual void HandleInlineTag(const FGameplayTag& Tag, const FString& Params) = 0;
  };
  ```

#### FR-PL-02 UMG组件套件
插件提供一套**默认UMG实现**，项目可直接使用或作为子类化基础。

- **WBP_DialogueHUD**：主容器，管理布局状态（Normal / ChoiceOnly / Cutscene）。
- **WBP_TextWindow**：文本显示窗口。
  - 支持Typewriter效果（速度可调，支持 `[speed]` 标签）。
  - 支持富文本块（`FRichTextBlock` + 自定义装饰器实现 `[shake]`, `[color]` 等）。
  - 支持自动换行与动态文本框高度调整。
  - 支持说话人名称标签与头像并排显示。
- **WBP_ChoiceButton**：选项按钮，支持鼠标悬停/键盘导航（上下键+回车/空格）。
  - 可配置`MaxVisibleChoices`（如5个），超出时滚动。
  - 支持选项条件预览（如显示"需要好感度>50"的灰色不可选选项，可配置是否隐藏）。
- **WBP_PortraitPanel**：立绘/头像面板。
  - 支持左右双侧同时显示多个角色。
  - 支持表情切换时的淡入淡出/缩放/位移过渡动画。
  - 支持高亮当前说话人（非当前说话人变暗）。
- **WBP_HistoryPanel**：历史日志面板。
  - 按时间倒序显示已播放对话。
  - 支持滚动查看、点击单条重新播放语音（如果有）。
- **WBP_SkipIndicator**：快进/跳过指示器（按住Ctrl/TextSkip键时显示进度条）。

#### FR-PL-03 镜头与场景集成
- **CameraDirective 数据结构**：
  ```cpp
  USTRUCT()
  struct FCameraDirective {
      GENERATED_BODY()
      UPROPERTY() ECameraDirectiveType Type; // Cut, Blend, Shake, FocusOnSpeaker
      UPROPERTY() TSoftObjectPtr<ACameraActor> CameraActor; // 预设机位
      UPROPERTY() FName CameraShotName; // 对应Sequencer中的Shot名称
      UPROPERTY() float BlendTime = 1.0f;
      UPROPERTY() EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut;
      UPROPERTY() ULevelSequence* SequenceToPlay; // 可附加播放一个Sequencer片段
  };
  ```
- **默认实现**：`ADialogueCameraDirector`（AActor），绑定到DialogueRunner的Camera事件，自动调用`PlayerController->SetViewTargetWithBlend`或触发Sequencer。
- **场景切换支持**：Event节点可触发`Event.Scene.Transition`异步事件，由项目层的Level Streaming系统处理，完成后NotifyEventComplete。

#### FR-PL-04 语音与音频
- **语音播放**：
  - TextWindow在 `OnLineStarted` 时根据 `Line.VoiceID` 查询数据库的VoiceTable，获取 `USoundBase*` 或Wwise/AudioKinetic事件。
  - 支持语音与打字机同步：每个字/词按时间戳高亮（Karaoke效果），需VoiceTable提供 `Phoneme/Timing` 数据（可选功能）。
  - 支持点击跳过当前语音（停止音频并瞬显文本）。
- **音效标签**：`[sfx=...]` 由Presenter层调用 `UGameplayStatics::PlaySound2D`。

#### FR-PL-05 LipSync（口型同步）与动态立绘接口
- **描述**：商业级JRPG通常需要角色说话时的口型动画，以及动态立绘（Spine/Live2D）支持。插件不直接依赖这些第三方方案，但提供标准化数据接口。
- **口型同步（LipSync）**：
  - `FVoiceData` 扩展字段：可选包含 `PhonemeSequence: TArray<FPhonemeTimeStamp>`（音素+时间戳）。
  - `IDialoguePresenter` 新增接口：`OnVoicePhoneme(FName Phoneme, float Duration)`。
  - 默认UMG实现中，若SpeakerProfile配置了口型映射表（`TMap<FName, TSoftObjectPtr<UTexture2D>> LipFlapFrames`），则按时间戳切换口型帧图片。
  - 项目层若使用SkeletalMesh角色，可接收Phoneme事件驱动BlendShape或骨骼动画。
- **动态立绘接口**：
  - `USpeakerProfile` 增加 `DynamicPortraitType: EDynamicPortraitType`（None / Spine / Live2D / Custom）。
  - 增加 `DynamicPortraitAsset: TSoftObjectPtr<UObject>`（泛型资产引用，项目层按类型Cast）。
  - `IDialoguePresenter` 增加 `OnSpeakerDynamicEmotion(FGameplayTag EmotionTag)`，由项目层的Spine/Live2D运行时接收并播放对应动画。
  - 插件提供示例蓝图：`WBP_SpinePortrait`（使用Spine插件的 `USpineWidget`）作为参考实现。

#### FR-PL-06 对话触发与表现层激活
- **描述**：Trigger资产（FR-DL-06）与表现层的衔接逻辑。
- **激活流程**：
  1. `UDialogueTriggerManager` 检测到触发条件满足 → 发送 `OnTriggerActivated` 委托。
  2. 项目层（或默认控制器）接收委托，可执行前置表现（如玩家转向NPC、镜头拉近）。
  3. 前置表现完成后，调用 `UDialogueRunner::StartDialogue`。
  4. `Presenter::OnDialogueStarted` 被调用，UMG HUD淡入，输入模式切换为 `UIOnly`。
- **去激活流程**：
  - 对话正常结束（Runner->EndDialogue）或强制中断（如进入战斗）时：
    - UMG HUD淡出，输入模式恢复为 `GameOnly` 或 `GameAndUI`。
    - 触发器若配置 `bOneShot`，则永久禁用。
    - 若触发器为 `Interact` 类型且未配置OneShot，冷却后重新进入可触发状态。
- **冲突处理**：当多个触发器同时满足时，按 `Priority` 排序，仅执行最高优先级；低优先级触发器进入等待队列（若高优先级对话结束后仍满足条件，则自动触发）。

---

### 4.5 本地化系统（Localization）

#### FR-LO-01 FText原生集成
- **描述**：所有 `FText` 字段自动接入UE的Localization Pipeline。
- **收集方式**：
  - 编辑器提供 `Gather Texts from Dialogue Databases` 菜单命令，扫描所有Database中的FText字段，生成/更新 `.manifest` 和 `.archive` 文件。
  - 文本Key规则：`Namespace = Project.Dialogue`，`Key = LineID`（确保稳定，不因文本内容变化而失效）。
- **运行时切换**：调用UE标准 `FInternationalization::SetCurrentCulture`，UI自动刷新（通过FText的动态绑定）。
- **语音本地化**：VoiceTable按语言分表（如 `VoiceTable_EN`, `VoiceTable_JP`），切换语言时重新绑定。

#### FR-LO-02 文本导出/导入工作流
- **CSV导出**：为翻译人员导出CSV，列包括 `Key`, `SourceText`, `TranslatedText`, `ContextComment`（可提取节点的Speaker和备注作为上下文）。
- **CSV导入**：翻译完成后，通过编辑器工具导入并覆盖对应Culture的Archive。

---

### 4.6 任务系统与状态机联动

#### FR-QS-01 GameplayTag事件总线
- **核心原则**：Chronicle Engine 不直接依赖任何任务系统插件，只负责**发送事件**和**暴露变量**。
- **标准事件Tag**：
  - `Event.Quest.Start` (Payload: QuestTag)
  - `Event.Quest.Update` (Payload: QuestTag, ObjectiveIndex)
  - `Event.Quest.Complete` (Payload: QuestTag)
  - `Event.GameState.Change` (Payload: StateTag)
  - `Event.Actor.Animate` (Payload: ActorTag, AnimName)
  - `Event.Battle.Encounter` (Payload: EnemyGroupTag, BGM)
  - `Event.Scene.Load` (Payload: LevelName, TransformTag)
- **接收外部事件**：任务系统或状态机可通过 `UDialogueRunner::SetVariable(FGameplayTag VariableTag, FVariableValue Value)` 反向修改变量，从而触发Condition节点评估。

#### FR-QS-02 示例适配器（项目层参考）
插件内提供示例蓝图/C++类 `UExampleQuestAdapter`，展示如何：
- Bind到DialogueRunner的事件委托，转发给项目任务系统。
- 监听任务系统回调，更新DialogueRunner的External变量。
- 在对话开始前检查任务状态，决定是否阻断或跳转对话。

---

### 4.7 调试与测试工具

#### FR-DB-01 编辑器运行时调试器（Dialogue Debugger）
- 已在前文 FR-ED-05 描述。

#### FR-DB-02 单元测试与自动化
- **Inkpot式单元测试**：提供 `UDialogueTest` 基类，支持在编辑器内运行自动化测试。
- **测试类型**：
  - 条件表达式求值测试（输入变量+表达式，断言结果）
  - 对话树遍历测试（给定变量初始值，断言最终到达节点/历史记录）
  - 序列化/反序列化一致性测试
  - 本地化文本收集完整性测试
- **CI/CD友好**：通过UE的 `FAutomationTestFramework` 实现，可在命令行运行。

#### FR-DB-03 统计与审计工具
- **字数统计**：按Database/Tree统计总字数、各语言字数、说话人台词量分布。
- **孤立节点检测**：检测不可达节点（无入口路径）。
- **断链检测**：检测Jump节点指向已删除GUID的情况。
- **变量使用报告**：列出所有定义变量和实际使用位置，标记未使用变量。

---

## 5. 非功能需求

### 5.1 性能需求
- **运行时内存**：单段对话树加载后常驻内存不超过2MB（不含纹理/音频资源）。
- **遍历性能**：单帧内完成100个节点的条件评估+跳转，耗时<0.1ms。
- **UMG渲染**：文本打字机效果不得导致帧率波动（使用Tick+累计时间，而非每字一帧）。
- **资产加载**：使用 `TSoftObjectPtr` 延迟加载Speaker肖像和语音，对话开始时异步预加载接下来5个节点所需资源。

### 5.2 兼容性
- **引擎版本**：主要支持UE 5.3-5.7，向后兼容策略通过预处理器宏隔离破坏性API变更。
- **构建目标**：Win64（首要）、Linux、macOS、PS5/Xbox Series X（运行时逻辑纯C++，无平台相关代码，主机兼容性好）。
- **项目类型**：C++项目与纯蓝图项目均支持（插件含C++源码，蓝图项目需至少一个空C++类触发编译）。

### 5.3 扩展性
- **节点扩展**：项目层可通过继承 `UDialogueNode`（C++）并在C++中注册到 `FDialogueNodeRegistry`，即可在编辑器右键菜单中出现新节点类型。
- **Presenter扩展**：通过 `IDialoguePresenter` 接口，项目可完全自定义UI（如3D世界内气泡对话、VR对话面板）。
- **变量类型扩展**：通过 `FVariableValue` 的自定义序列化Union，未来可扩展新类型。
- **导入器扩展**：`UDialogueImporterBase` 抽象类，支持项目层添加自定义格式导入（如Twine、Ren'Py脚本）。

### 5.4 稳定性与错误处理
- **数据损坏防护**：加载对话树时验证所有Edge的GUID有效性，无效Edge记录Warning并自动断开。
- **循环检测**：编辑器内检测Jump/SubDialogue循环引用，标红并阻止PIE。
- **表达式安全**：条件表达式运行时求值若抛出异常（如除以零、访问不存在的变量），返回Default值并记录Error，**不得Crash**。
- **Graceful Degradation**：语音文件缺失时静默跳过并输出Log；Portrait缺失时显示默认占位图。

---

## 6. 接口与API需求

### 6.1 C++核心API

```cpp
// 运行时核心
UCLASS()
class CHRONICLEENGINE_API UDialogueRunner : public UObject {
    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void StartDialogue(UDialogueTree* Tree, FName EntryNode = NAME_None);
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void Advance();
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void SelectChoice(int32 ChoiceIndex);
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Dialogue")
    void EndDialogue();
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    void SetVariable(FGameplayTag Tag, const FVariableValue& Value);
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Variables")
    FVariableValue GetVariable(FGameplayTag Tag) const;
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Save")
    void SaveState(FDialogueSaveData& OutData);
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Save")
    void LoadState(const FDialogueSaveData& InData);
    
    UFUNCTION(BlueprintCallable, Category="Chronicle|Rollback")
    void PerformRollback(int32 Steps = 1);
    
    // 事件
    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnDialogueEvent OnDialogueEvent;
    
    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnLineStarted OnLineStarted;
    
    UPROPERTY(BlueprintAssignable, Category="Chronicle|Events")
    FOnChoicesPresented OnChoicesPresented;
};
```

### 6.2 蓝图节点暴露
- 所有UDialogueRunner的UFUNCTION均为BlueprintCallable。
- 提供纯蓝图工具节点：
  - `Get Dialogue Runner`（从World/GameInstance获取）
  - `Bind External Variable`（绑定蓝图事件到变量读写）
  - `Is Dialogue Playing`（判断是否正在播放对话）
  - `Get Current Speaker`（获取当前说话人Tag）
  - `Skip Current Line`（跳过当前打字机效果）
  - `Set Skip Enabled`（启用/禁用Skip快进模式）
  - `Set Auto Play Enabled`（启用/禁用Auto自动播放模式）
  - `Is Current Line Seen`（查询当前文本是否已读过）
  - `Open History Panel` / `Close History Panel`

---

## 7. 数据需求与示例

### 7.1 示例：简单分支对话

**场景**：玩家与NPC Alice对话，如果已完成主线任务Stage 3且Alice好感度>50，则进入隐藏剧情。

**节点图结构**：
```
[Root]
  └── [Speech: Alice] "你好，旅行者。"
        └── [Condition: Global.Quest.MainStage >= 3]
              ├── [True] [Condition: Global.Likeness.Alice > 50]
              │             ├── [True] [Speech: Alice] "你来了...我一直在等你。"
              │             │             └── [Event: Event.Quest.HiddenStart]
              │             └── [False] [Speech: Alice] "有什么事吗？"
              └── [False] [Speech: Alice] "请去完成主线任务再来找我。"
```

### 7.2 示例：复杂选择（多条件隐藏选项）

**Choice节点配置**：
- 选项1: "能给我那把剑吗？" → VisibilityCondition: `Inventory.HasSpace AND Global.Quest.BlacksmithComplete`
- 选项2: "告诉我关于那座塔的事。" → 始终可见
- 选项3: "（威胁）不交出来就死。" → VisibilityCondition: `Player.Stat.Strength > 80`；选中后 → Event: `Event.Reputation.Down`
- 选项4: "再见。" → 始终可见，选中后 → EndDialogue

---

## 8. 风险分析

| 风险 | 影响 | 缓解措施 |
|---|---|---|
| Slate编辑器开发周期超预期 | 高 | **核心降级方案**：如6周内无法完成可用节点图，先交付基于Editor Utility Widget的树形编辑器；Slate节点图作为后续大版本更新。开发期采用双轨并行 |
| 大型对话树性能瓶颈 | 中 | 节点使用纯Struct存储，非UObject；编辑器内视口裁剪与分页加载 |
| 纯蓝图项目编译障碍 | 中 | 文档明确指导添加空C++类；提供示例空类模板文件，用户复制即可 |
| 与项目现有任务系统冲突 | 低 | 严格通过GameplayTag事件解耦，不强制依赖任何具体系统；提供无依赖示例适配器 |
| 多语言文本Key冲突 | 低 | 强制使用TreeGUID+NodeGUID生成Key，确保全局唯一；不依赖文本内容Hash |
| 多人编辑UAsset二进制冲突 | 高 | **缓解**：支持JSON/YAML文本导出作为策划主工作流，定期导入回UAsset；Database级+Tree级软性锁定提醒 |
| Skip/Auto系统与回滚系统逻辑冲突 | 中 | Skip不写入Seen标记（仅正常播放才标记）；回滚时同步回退Seen标记快照，确保回滚后可重新Skip |
| Spine/Live2D第三方插件缺失 | 低 | 接口预留但不强依赖；无插件时DynamicPortrait自动降级为静态Texture2D |

---

## 9. 附录

### 9.1 目录结构（插件）
```
ChronicleEngine/
├── ChronicleEngine.uplugin
├── Config/
│   └── FilterPlugin.ini
├── Content/
│   ├── Widgets/               # 默认UMG资产
│   ├── Materials/             # 默认材质（如打字机遮罩）
│   └── Maps/                  # 示例地图
├── Resources/
│   └── Icon128.png
├── Source/
│   ├── ChronicleEngine/
│   │   ├── Public/
│   │   │   ├── Core/          # 类型定义、接口、GameplayTags
│   │   │   ├── Data/          # UDialogueDatabase, UDialogueTree, USpeakerProfile
│   │   │   ├── Runtime/       # UDialogueRunner, UVariableBank, UDialogueMemento
│   │   │   └── Presentation/  # IDialoguePresenter, ADialogueCameraDirector
│   │   └── Private/
│   ├── ChronicleEngineEditor/
│   │   ├── Public/
│   │   │   ├── Asset/         # 资产工厂、资产TypeActions
│   │   │   ├── Graph/         # Slate节点图、Schema、连接策略
│   │   │   ├── Customization/ # Details面板自定义
│   │   │   └── Debugging/     # 调试器窗口、断点管理
│   │   └── Private/
│   └── ChronicleEngineTests/
│       └── ...                # 自动化测试
└── Documentation/
    ├── QuickStart.md
    ├── BlueprintAPI.md
    └── SampleProjectSetup.md
```

### 9.2 依赖清单
- **UE5核心模块**：Core, CoreUObject, Engine, InputCore, Slate, SlateCore, EditorStyle, UnrealEd, AssetTools, Kismet, Projects, GameplayTags
- **可选增强**：UMG, LevelSequence, MovieScene（用于镜头集成）
- **第三方**：无强制第三方库。若需表达式解析，可轻量集成 `exprtk` 头文件库或手写递归下降解析器。

### 9.3 命名规范
- 类前缀：`U`（UObject）、`A`（AActor）、`S`（SlateWidget）、`F`（Struct）、`I`（Interface）
- 模块名：`ChronicleEngine`（Runtime）、`ChronicleEngineEditor`（Editor-only）
- GameplayTag根节点：`Chronicle.Speaker.*`, `Chronicle.Event.*`, `Chronicle.Variable.*`
