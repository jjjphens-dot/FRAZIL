# FRAZIL 项目开发与架构规范

> 文档状态：Draft v0.3；Development baseline accepted<br>
> 适用阶段：项目初始化 → v1.0  
> 团队规模：2 人  
> 目标产品：以“水 / 冰材质化”为核心声音特征的实时音频效果器，首要目标为 VST3，同时保留 Standalone 作为开发与测试宿主。

---

## 0. 文档目的

本文档是 FRAZIL 的**项目总纲**，用于统一：

1. 产品层到底要实现什么；
2. 工程层如何把产品语义转换为稳定、可测试的 DSP；
3. Water / Ice / Routing / Parameter / UI 等模块如何解耦；
4. 并联与串联模式下控制参数的准确语义；
5. DAW / VST3 Host 自动化应如何接入；
6. 开发阶段如何划分；
7. 哪些功能现在必须做，哪些应延后；
8. 两人协作时如何避免重复实现、知识孤岛和接口失控；
9. 如何保证实时安全、可读性、可维护性和后续扩展能力；
10. 一个功能何时才算真正完成。

本文档优先级高于临时聊天结论。若架构或产品方向发生重大变化，应通过 ADR（Architecture Decision Record）记录理由并同步修改本文档。

## Modification Policy

产品语义和已 Accepted 的架构决策属于 LOCKED 合同；候选方案、接口边界、实时规则和正式预算属于 CONTROLLED 内容；纯解释性文字可维护。`experiments/` 中的 candidate、A/B、prototype 和 listening exploration 不自动要求 ADR；只有候选方案被采纳为 production architecture/core DSP decision，或改变既有合同/边界时，才按 `docs/DOCUMENT_GOVERNANCE.md` 的 ADR trigger 补充 issue、ADR、测试与迁移/兼容性证据。

---

## 0.1 v0.2 已确认的产品决策

本版本正式固定以下控制模型：

### Parallel（并联）

```text
             ┌→ Water ─┐
Input ───────┤          ├→ Water / Ice Balance → Global Mix → Output
             └→ Ice ───┘
```

用户使用**一个横向比例控制条**调节 Water 与 Ice 的相对占比：

```text
WATER  ←────────────●────────────→  ICE
```

该参数记为：

```text
parallel.balance
```

并联模式下不使用两个独立的 Water Amount / Ice Amount 来表达两路比例。

---

### Serial（串联）

支持：

```text
Input → Water → Ice → Output
```

以及：

```text
Input → Ice → Water → Output
```

串联模式中 Water 与 Ice 使用**独立 Amount**：

```text
water.amount
ice.amount
```

它们分别控制各自处理阶段对当前信号的作用比例。

例如 Water → Ice：

```text
Input
  ↓
Water Stage [Water Amount]
  ↓
Ice Stage   [Ice Amount]
  ↓
Global Mix
  ↓
Output
```

---

### Water → Ice 的“时间变化”不增加插件内部 Morph 参数

必须区分两个概念：

#### 信号路由顺序

```text
Water → Ice
```

是 `RoutingMode`。

#### 随时间从 Water 感过渡到 Ice 感

不增加额外的：

```text
WaterToIceMorph
Transition
FreezeAutomation
```

等内部联动参数。

用户需要这种动态变化时，由 DAW / Host 分别自动化：

```text
water.amount
ice.amount
```

例如：

```text
Water Amount  100% ─────────────→ 0%
Ice Amount      0% ─────────────→ 100%
```

这样用户可以独立控制：

- 起始时间；
- 结束时间；
- automation curve；
- 两个模块是否重叠；
- 过渡是否对称；
- 是否先降 Water 再升 Ice；
- 是否同时变化。

**插件不替宿主重复实现时间线自动化系统。**

---

### Host Automation 是 v1 核心合同

所有正式用户控制参数必须：

- 在插件初始化时静态注册；
- 有稳定 Parameter ID；
- 能被 VST3 Host 枚举；
- 能加入 DAW automation lane；
- 能保存 / 恢复；
- 连续参数变化必须经过 smoothing 或等价的 click-free 处理；
- UI 隐藏某参数时，不得从宿主参数列表中动态删除它。

---

### Input / Output Gain 是 v1 核心控制

加入两个独立、可自动化的电平参数：

```text
input.gain
output.gain
```

固定信号位置：

```text
Raw Input
   ↓
Input Gain
   ↓
Post-Input Signal
   ├──────────────→ Dry Reference ──────────────┐
   │                                            │
   └→ Water / Ice / Routing → Wet Path ────────┤
                                                ↓
                                           Global Mix
                                                ↓
                                           Output Gain
                                                ↓
                                              Output
```

`Input Gain` 定义为真正的插件输入 Trim：位于所有材质处理之前，并同时影响 Global Mix 的 Dry reference 与 Wet processing。它不能在后续版本中被悄悄改成只作用于 Wet path 的 Drive。若未来需要非线性激励，应新增独立的 `drive` / `inputSensitivity` 等参数。

`Output Gain` 始终位于 `Global Mix` 之后，只负责最终 loudness compensation / output trim。

初版建议范围：

```text
Input Gain  : -24 dB ... +24 dB, default 0 dB
Output Gain : -24 dB ... +24 dB, default 0 dB
```

最终范围可在 gain-staging 与听测后调整；一旦公开，Parameter ID 必须稳定。

---

### Undo / Redo 是插件内部编辑历史

FRAZIL v1 加入：

```text
Undo
Redo
```

它们只撤销 / 重做**用户在 FRAZIL 编辑器中主动完成的参数编辑步骤**。

例如：

```text
拖动一次 Water Amount     → 1 个 transaction
拖动一次 Parallel Balance → 1 个 transaction
点击一次 Water Enable     → 1 个 transaction
切换一次 RoutingMode      → 1 个 transaction
```

不记录：

```text
DAW automation playback
Host 写入的 parameter changes
DAW project state restore
Plugin 初始化
DSP smoothing
```

Undo / Redo 本身不是音频参数，不注册为 VST3 automation parameter，也不能进入 audio thread。宿主负责 timeline / automation undo；FRAZIL 只负责插件编辑器内部的编辑历史。

---

# 1. 产品定义

## 1.1 FRAZIL 是什么

FRAZIL 是一个“声音材质化”效果器。

输入并非被简单地添加水声或冰声采样，而是经过 DSP 转换后，获得与以下对象相关的听觉质感：

- 流动的水；
- 水滴 / 液体扰动；
- 冰晶；
- 冰块摩擦；
- 冰裂 / 碎裂；
- 水与冰之间的过渡状态。

核心目标不是“自然环境拟音器”，而是：

> **让现有音乐素材在保持可辨识性的同时，获得水与冰的声音材料属性。**

---

## 1.2 v1 的核心用户能力

用户至少应能够：

- 开启 / 关闭 Water；
- 开启 / 关闭 Ice；
- 选择 Parallel / Water → Ice / Ice → Water；
- 在 Parallel 中使用一条 Water ↔ Ice Balance 控制相对比例；
- 在 Serial 中独立控制 Water Amount；
- 在 Serial 中独立控制 Ice Amount；
- 调节 Input Gain；
- 调节整体 Global Dry/Wet；
- 调节 Output Gain；
- 撤销 / 重做插件编辑器中的参数修改步骤；
- 在合理范围内控制 Water 的主要质感；
- 在合理范围内控制 Ice 的主要质感；
- 在 DAW 中识别并自动化所有正式用户参数；
- 保存并恢复插件状态；
- 在常见宿主中稳定工作。

Water 与 Ice 的内部 DSP 参数可以很多，但**不应全部直接暴露给用户**。

---

## 1.3 产品层参数与 DSP 参数必须分离

FRAZIL 应区分：

### 用户参数（User Parameters）

体现用户意图：

```text
WaterEnable
IceEnable
RoutingMode

ParallelBalance

WaterAmount
IceAmount

InputGain
GlobalMix
OutputGain

WaterCharacter
WaterMotion
IceCharacter
IceFracture
...
```

### 引擎参数（Engine Parameters）

体现 DSP 和 routing 实际需要：

```text
routing
parallelBalance

waterStageAmount
iceStageAmount

waterParameters
iceParameters

inputGain
globalMix
outputGain
...
```

其中：

```text
ParallelBalance
```

与：

```text
WaterAmount / IceAmount
```

是**不同的产品语义**，不能继续抽象成同一个 MaterialBalance。

---

## 1.4 Amount 与算法内部参数分离

推荐把：

```text
water.amount
ice.amount
```

理解成**串联阶段处理比例（stage mix）**，而不是 WaterProcessor / IceProcessor 内部算法本身的一部分。

例如：

```text
WaterProcessor
```

负责生成 Water 处理后的结果：

```text
W(x)
```

而：

```text
water.amount
```

由 RoutingEngine / stage mixer 决定当前阶段采用多少：

```text
mix(x, W(x), waterAmount)
```

这样可以避免：

```text
WaterProcessor
```

为了 Parallel / Serial 两种 UI 逻辑承担额外职责。

如果未来 Water 算法确实需要内部“强度”参数，应另设清晰参数，例如：

```text
water.character
water.motion
water.intensity
```

不能无意中把“算法特征强度”和“串联阶段 Dry/Wet”混为一谈。

---

# 2. 总体架构原则

FRAZIL 不采用“大而全”的多层企业架构，而采用适合两人音频项目的四层结构：

```text
┌───────────────────────────────────────┐
│  1. Presentation / Product Layer      │
│  GUI、用户参数、交互语义               │
└───────────────────┬───────────────────┘
                    ↓
┌───────────────────────────────────────┐
│  2. Application / Orchestration Layer │
│  参数映射、状态、AudioEngine 编排       │
└───────────────────┬───────────────────┘
                    ↓
┌───────────────────────────────────────┐
│  3. DSP Execution Layer               │
│  Water / Ice / Routing / DSP primitives│
└───────────────────┬───────────────────┘
                    ↓
┌───────────────────────────────────────┐
│  4. Platform / Infrastructure Layer   │
│  JUCE AudioProcessor、VST3、构建、宿主   │
└───────────────────────────────────────┘
```

依赖必须总体保持**单向**。

职责分层图用于说明 ownership，不等同于 C++ include 顺序。实际允许的依赖边界是：

```text
ui -> narrow plugin parameter interface
ui -> narrow app edit/history command interface
plugin -> app -> dsp
tests -> 被测模块
```

`ParameterLayout` 属于 `src/plugin/` 的 Platform / Host Adapter Layer，负责 JUCE-facing 静态参数注册；`StateModel` 属于 `src/app/`，只依赖 application value types。Plugin Host State Adapter 调用 StateModel，禁止 `app -> plugin` 反向依赖。

禁止出现：

```text
WaterProcessor → PluginEditor
IceProcessor   → APVTS
DSP primitive  → PluginProcessor
```

---

# 3. 应用层与执行层的职责划分

## 3.1 Presentation / Product 层

目录：

```text
src/ui/
```

负责：

- 主界面；
- Knob / Button / Slider / Meter；
- RoutingMode 的显示；
- Parallel / Serial 对应控件的显示切换；
- 参数 attachment；
- tooltip；
- 可视化。

不负责：

- 具体 DSP 算法；
- buffer 分配策略；
- Water / Ice 算法选择；
- 实际 routing；
- stage mix 运算；
- Host 参数注册。

UI 的任务是：

> **表达和修改产品状态。**

不是：

> **执行声音算法。**

UI 只通过 narrow plugin parameter interface 表达 Host 参数，并通过 narrow app edit/history command interface 发起 begin/end gesture、discrete edit、undo 和 redo。UI 不直接持有 AudioEngine 或任何 DSP object。

---

## 3.2 Application / Orchestration 层

目录：

```text
src/app/
```

核心模块：

```text
AudioEngine
ParameterMapper
ParameterSnapshot
StateModel
EditHistoryManager
```

职责：

- 将宿主 / UI 参数转换为引擎参数；
- 管理 Water / Ice 启用状态；
- 管理 RoutingMode；
- 根据 RoutingMode 决定哪些参数当前具有 DSP 意义；
- 管理插件 UI 产生的 Undo / Redo transaction；
- 在每个 audio block 开始时生成一致参数快照；
- 驱动 DSP 模块；
- 将状态保存 / 恢复与 DSP 解耦。

原则：

> Application 知道“Water 和 Ice 是什么、当前如何组合”，但不知道它们内部具体如何制造声音。

---

## 3.3 DSP Execution 层

目录：

```text
src/dsp/
```

负责真正的实时声音处理。

包含：

```text
WaterProcessor
IceProcessor
RoutingEngine
StageMixer
DryWetMixer
DSP primitives
```

该层：

- 不读取 UI；
- 不访问 PluginEditor；
- 不直接读取 APVTS；
- 不进行文件 I/O；
- 不进行 console logging；
- process 中不得依赖阻塞锁；
- process 中不得进行不可控内存分配。

---

## 3.4 Platform / Infrastructure 层

目录：

```text
src/plugin/
```

负责：

```text
PluginProcessor
VST3 / Standalone glue
host bus layout
JUCE callback
ParameterLayout（static Host/JUCE parameter declaration）
state serialization adapter
```

这是 FRAZIL 和 DAW / VST3 Host 之间的适配器。

核心目标：

> 即使未来加入 AU 或其他格式，WaterProcessor 与 IceProcessor 也不需要知道 Host automation 的存在。

---

# 4. 推荐目录结构

```text
FRAZIL/
│
├─ CMakeLists.txt
├─ README.md
├─ CHANGELOG.md
├─ LICENSE
│
├─ cmake/
│
├─ src/
│  │
│  ├─ plugin/
│  │  ├─ PluginProcessor.cpp
│  │  ├─ PluginProcessor.h
│  │  ├─ ParameterLayout.cpp
│  │  └─ ParameterLayout.h
│  │
│  ├─ app/
│  │  ├─ AudioEngine.cpp
│  │  ├─ AudioEngine.h
│  │  ├─ ParameterMapper.cpp
│  │  ├─ ParameterMapper.h
│  │  ├─ ParameterSnapshot.h
│  │  ├─ StateModel.*
│  │  ├─ EditHistoryManager.cpp
│  │  └─ EditHistoryManager.h
│  │
│  ├─ dsp/
│  │  ├─ water/
│  │  │  ├─ WaterProcessor.cpp
│  │  │  ├─ WaterProcessor.h
│  │  │  └─ ...
│  │  ├─ ice/
│  │  │  ├─ IceProcessor.cpp
│  │  │  ├─ IceProcessor.h
│  │  │  └─ ...
│  │  ├─ routing/
│  │  │  ├─ RoutingEngine.cpp
│  │  │  ├─ RoutingEngine.h
│  │  │  ├─ StageMixer.cpp
│  │  │  └─ StageMixer.h
│  │  └─ primitives/
│  │     ├─ SmoothedValue.*
│  │     ├─ RandomSource.*
│  │     ├─ Envelope.*
│  │     ├─ Filter.*
│  │     └─ ...
│  └─ ui/
│     ├─ PluginEditor.*
│     └─ components/
│        ├─ RoutingSelector.*
│        ├─ ParallelBalanceSlider.*
│        ├─ AmountKnob.*
│        ├─ GainKnob.*
│        ├─ UndoRedoControls.*
│        └─ ...
│
├─ experiments/
│  ├─ water/
│  ├─ ice/
│  └─ routing/
│
├─ tests/
│  ├─ unit/
│  ├─ dsp/
│  ├─ render/
│  └─ integration/
│
├─ testdata/
│  ├─ input/
│  ├─ reference/
│  └─ rendered/
│
├─ docs/
│  ├─ PROJECT_SPEC.md
│  ├─ PRODUCT.md
│  ├─ PARAMETERS.md
│  ├─ TESTING.md
│  ├─ PERFORMANCE.md
│  └─ adr/
│
└─ .github/
   ├─ workflows/
   ├─ ISSUE_TEMPLATE/
   └─ pull_request_template.md
```

---

# 5. 核心模块设计

## 5.1 PluginProcessor

职责：

- 接收 JUCE audio callback；
- 读取稳定注册的宿主参数；
- 生成 / 获取 ParameterSnapshot；
- 调用 AudioEngine；
- 与宿主交换插件状态；
- 处理 channel / bus layout。

不负责：

- Water 算法；
- Ice 算法；
- UI 逻辑；
- routing 运算；
- 复杂参数映射。

理想状态：

```cpp
void PluginProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const auto snapshot = parameterSnapshot.capture();
    const auto engineParameters = parameterMapper.map(snapshot);
    audioEngine.process(buffer, engineParameters);
}
```

---

## 5.2 AudioEngine

这是**应用层执行编排器**。

它组合：

```text
Input Gain
WaterProcessor
IceProcessor
RoutingEngine
Global Dry/Wet
Output Gain
```

建议接口：

```cpp
struct ProcessSpec
{
    double sampleRate {};
    int maximumBlockSize {};
    int numChannels {};
};

class AudioEngine
{
public:
    void prepare(const ProcessSpec&);
    void reset() noexcept;

    void process(
        juce::AudioBuffer<float>&,
        const EngineParameters&) noexcept;
};
```

AudioEngine 不包含 Water / Ice 的具体算法。

---

## 5.3 WaterProcessor

公开接口保持小：

```cpp
struct WaterParameters
{
    float character {};
    float motion {};
};

class WaterProcessor
{
public:
    void prepare(const ProcessSpec&);
    void reset() noexcept;

    void process(
        juce::AudioBuffer<float>&,
        const WaterParameters&) noexcept;
};
```

候选内部机制：

```text
FlowModulator
DropletExciter
LiquidResonator
SpectralShaper
MicroDelayNetwork
```

这些名称在算法未验证前只是候选。

原则：

> **先用 experiments 证明算法值得留下，再进入生产 DSP。**

---

## 5.4 IceProcessor

生命周期尽量与 Water 一致：

```cpp
struct IceParameters
{
    float character {};
    float fracture {};
};

class IceProcessor
{
public:
    void prepare(const ProcessSpec&);
    void reset() noexcept;

    void process(
        juce::AudioBuffer<float>&,
        const IceParameters&) noexcept;
};
```

候选内部机制：

```text
FrictionTexture
CrackTransientGenerator
ModalResonator
CrystalExciter
SpectralShaper
```

Water 与 Ice 可以共享生命周期和参数传递习惯，但不要求内部 DSP 对称。

---

# 5.5 RoutingEngine

路由是独立职责，不写死在 Water 或 Ice 中。

```cpp
enum class RoutingMode
{
    parallel,
    waterIntoIce,
    iceIntoWater
};
```

建议：

```cpp
class RoutingEngine
{
public:
    void prepare(const ProcessSpec&);
    void reset() noexcept;

    void process(
        AudioBuffer<float>& buffer,
        WaterProcessor& water,
        IceProcessor& ice,
        const EngineParameters& params) noexcept;
};
```

---

## 5.5.1 Parallel

信号流：

```text
             ┌→ Water ─┐
Input ───────┤          ├→ Balance → Wet
             └→ Ice ───┘
```

设：

```text
x = input
w = W(x)
i = I(x)
b = normalized ParallelBalance, 0...1
```

第一版可采用直接线性比例：

```text
wet = (1 - b) * w + b * i
```

对应：

```text
b = 0.0 → 100% Water
b = 0.5 → 50% Water + 50% Ice
b = 1.0 → 100% Ice
```

后续可以通过 loudness-matched listening test 判断是否需要修改 crossfade law，但必须保持：

- 左端明确为 Water；
- 右端明确为 Ice；
- 中间单调连续；
- 不发生不可解释的增益跳变。

### Parallel 中的 Amount

并联模式 UI 不使用独立：

```text
Water Amount
Ice Amount
```

来表示两条支路比例。

`water.amount` 与 `ice.amount` 参数仍然存在于宿主参数列表中，但在 Parallel 模式下不参与当前 routing 计算。

---

## 5.5.2 Water → Ice Serial

信号流：

```text
Input
  ↓
Water
  ↓
Water Stage Mix
  ↓
Ice
  ↓
Ice Stage Mix
  ↓
Wet
```

设：

```text
x = input
aW = water.amount
aI = ice.amount
```

概念行为：

```text
waterProcessed = W(x)
stage1 = mix(x, waterProcessed, aW)

iceProcessed = I(stage1)
wet = mix(stage1, iceProcessed, aI)
```

因此：

```text
Water Amount = 0%
```

表示 Water stage 对当前信号不产生处理贡献。

```text
Water Amount = 100%
```

表示下一阶段收到完整 Water 处理结果。

Ice 同理。

---

## 5.5.3 Ice → Water Serial

概念行为：

```text
iceProcessed = I(x)
stage1 = mix(x, iceProcessed, aI)

waterProcessed = W(stage1)
wet = mix(stage1, waterProcessed, aW)
```

因此两个 Amount 的语义在两种串联方向中保持一致：

> **Amount 永远表示“该模块在其所在 stage 中处理多少”。**

不要让同一个参数在不同串联方向中改变含义。

---

## 5.5.4 Enable 行为

### Parallel

若 Water 与 Ice 都开启：

```text
parallel.balance
```

正常控制比例。

若只开启 Water：

```text
wet = Water branch
```

Balance 暂时不影响输出，但值必须保留。

若只开启 Ice：

```text
wet = Ice branch
```

同理。

若都关闭：

```text
wet path = pass-through
```

---

### Serial

若 Water 关闭：

```text
Water stage = pass-through
```

`water.amount` 数值保留，但当前无效果。

若 Ice 关闭：

```text
Ice stage = pass-through
```

同理。

模块重新开启时恢复此前 Amount，不自动重置。

---

# 5.6 Gain Staging

## 5.6.1 Input Gain

`input.gain` 在所有 Water / Ice 处理之前执行：

```text
rawInput → Input Gain → processedInput
```

`processedInput` 同时作为 Global Mix 的 Dry reference 和 RoutingEngine 的输入，因此 Input Gain 始终表示“进入 FRAZIL 整个信号路径前的输入电平”。推荐 UI / 参数层使用 dB，DSP 内转换为 linear gain；连续变化必须 smoothing。

## 5.6.2 Output Gain

最终路径：

```text
mixed = GlobalMix(dry, wet)
output = mixed * outputGain
```

Output Gain 不改变 Water / Ice 的输入、Parallel Balance、Serial Amount 或 Global Mix，只负责最终输出 trim；连续变化同样必须 smoothing。

## 5.6.3 Gain 语义稳定性

禁止把 `input.gain` 在未来改成 Wet-only drive。若需要 drive，应新增参数。这样可以保护 preset、Host automation 与 gain-staging 的长期语义。

---

# 5.7 EditHistoryManager

Undo / Redo 是 Application 层职责，推荐建立独立 `EditHistoryManager`。底层可以使用 JUCE `UndoManager` / `UndoableAction`，但 FRAZIL 必须自行定义 transaction 边界。

建议接口方向：

```cpp
class EditHistoryManager
{
public:
    void beginParameterGesture(const juce::String& parameterID, float startValue);
    void endParameterGesture(const juce::String& parameterID, float endValue);
    void pushDiscreteParameterEdit(const juce::String& parameterID,
                                   float oldValue,
                                   float newValue);

    bool canUndo() const noexcept;
    bool canRedo() const noexcept;
    void undo();
    void redo();
    void clear();
};
```

## 5.7.1 连续控制按 Gesture 合并

一次旋钮 / Slider 拖动只能形成一个 transaction：

```text
mouseDown / beginGesture
→ 记录 startValue
→ 拖动过程中正常 setValueNotifyingHost
→ mouseUp / endGesture
→ 记录 endValue
→ push 1 transaction
```

不能把拖动中的每一个中间采样点都记成一步。

## 5.7.2 离散操作

Water/Ice Enable、RoutingMode、reset-to-default 等每次用户操作通常各算一步。未来若 `Reset All` / `Load Preset` 一次修改多个参数，应把这些修改组合成一个 transaction。

## 5.7.3 Host Automation 必须与 History 隔离

历史来源必须是：

```text
Plugin UI user gesture / command
```

而不是通用的 `parameterChanged()` callback。否则 DAW automation playback 会不断污染 Undo 栈。

Undo / Redo 恢复值时仍应走标准参数通知路径，使 UI / DSP / Host 同步；但 Undo / Redo 本身不能成为 Host 参数。

## 5.7.4 State Restore

宿主调用 `setStateInformation(...)` 恢复工程状态后，应清空插件内部 Undo / Redo history，避免历史动作指向已经失效的旧状态。

未来插件自己的 preset browser 可以把“用户主动 Load Preset”作为一个可撤销 transaction；Host project restore 不属于插件编辑历史。

## 5.7.5 线程与容量

EditHistory 只允许在 Message / UI thread 操作，不进入 audio callback，不持有会阻塞 audio thread 的锁。历史必须有上限，例如约 100 个 transaction，避免无限增长。

---

# 5.8 ParameterMapper

这是保持 UI / Host 与 DSP 解耦的重要模块。

```cpp
struct UserParameters
{
    bool waterEnabled {};
    bool iceEnabled {};

    RoutingMode routing {};

    float parallelBalance {};

    float waterAmount {};
    float iceAmount {};

    float inputGain {};
    float globalMix {};
    float outputGain {};
};

struct EngineParameters
{
    bool waterEnabled {};
    bool iceEnabled {};

    RoutingMode routing {};

    float parallelBalance {};

    float waterStageAmount {};
    float iceStageAmount {};

    WaterParameters water {};
    IceParameters ice {};

    float inputGain {};
    float globalMix {};
    float outputGain {};
};
```

映射：

```cpp
class ParameterMapper
{
public:
    EngineParameters map(
        const UserParameters&) const noexcept;
};
```

ParameterMapper 负责：

- 参数范围归一化；
- UI 语义 → engine 语义；
- mode-dependent 参数选择；
- 默认值；
- 必要的非线性映射。

ParameterMapper 不负责：

- buffer；
- DSP；
- UI 显隐；
- 宿主 timeline。

---

# 6. 参数与 Host Automation 系统

## 6.1 Parameter ID 是兼容性接口

建议 v1 之前锁定以下核心 ID：

```text
water.enabled
ice.enabled

routing.mode

parallel.balance

water.amount
ice.amount

input.gain
global.mix
output.gain
```

后续核心 macro 示例：

```text
water.character
water.motion

ice.character
ice.fracture
```

一旦进入公开版本并被 DAW automation / preset / session 使用，应把这些 ID 视为稳定 API。

UI 显示名称可以变化，但底层 ID 不应轻易改变。

---

## 6.2 参数必须静态注册

禁止：

```text
进入 Parallel
→ 动态创建 parallel.balance
→ 删除 water.amount / ice.amount
```

也禁止：

```text
进入 Serial
→ 删除 parallel.balance
→ 动态创建两个 Amount
```

正确方式：

```text
Plugin 初始化
    ↓
一次性注册所有正式参数
    ↓
Host 始终看到稳定参数集合
```

UI 根据 RoutingMode 只改变：

```text
visible / enabled
```

而不是改变宿主参数结构。

---

## 6.3 模式切换时的参数有效性

### Parallel

当前 DSP 使用：

```text
parallel.balance
```

保留但暂不使用：

```text
water.amount
ice.amount
```

### Water → Ice / Ice → Water

当前 DSP 使用：

```text
water.amount
ice.amount
```

保留但暂不使用：

```text
parallel.balance
```

切换模式不能重置这些值。

---

## 6.4 所有旋钮 / Slider 必须可被宿主自动化

至少以下连续参数必须可 automation：

```text
parallel.balance
water.amount
ice.amount
input.gain
global.mix
output.gain
Water / Ice 用户 macro
```

同时建议：

```text
water.enabled
ice.enabled
routing.mode
```

也作为标准宿主参数暴露。

---

## 6.5 不实现插件内部 Water → Ice 时间线

v1 不增加：

```text
Morph Time
Freeze Time
Transition Envelope
Internal Sequencer
Internal Automation Recorder
```

用户需要自动过渡时，由宿主 automation 完成：

```text
water.amount
ice.amount
```

这是明确的产品边界：

> **FRAZIL 负责实时声音变换；DAW 负责时间线编排。**

---

## 6.6 APVTS / Host 参数读取

禁止 DSP 每 sample 直接读取 APVTS。

正确结构：

```text
Host Parameter Atomics
        ↓
ParameterSnapshot
        ↓
 ParameterMapper
        ↓
 EngineParameters
        ↓
   AudioEngine
        ↓
  Routing / DSP
```

每个 audio block 建立一致快照。

连续参数在 DSP / stage mixer 中做 smoothing。

---

## 6.7 参数平滑

至少需要处理：

```text
parallel.balance
water.amount
ice.amount
input.gain
global.mix
output.gain

continuous Water macros
continuous Ice macros
```

目标：

- 不出现 zipper noise；
- 不因快速 DAW automation 产生明显 click；
- automation 曲线听感连续。

RoutingMode / Enable 等离散改变不得简单硬切导致爆音。

可以采用：

- 短 crossfade；
- 状态平滑；
- click-free bypass；
- 必要时双路径短暂并行。

具体时长通过听测和 CPU profiling 决定。

---

## 6.8 Undo / Redo 与 Host 参数合同

Undo / Redo 是 editor command，不是 parameter。禁止创建 `history.undo` / `history.redo` 等 VST3 automation parameter。

插件 history 只记录 FRAZIL UI 发起并完成的编辑 transaction，不记录 Host automation、Host project restore、参数初始化同步或 DSP smoothing。

当 Undo / Redo 恢复参数时，值仍应通过标准参数通知机制写回。如果宿主正在播放 automation，后续 automation point 再次覆盖该参数属于正常行为：**DAW timeline 对 automation 参数的实时控制优先于插件内部编辑历史。**

---

# 7. UI / UX 控制合同

## 7.1 Parallel UI

核心区域：

```text
[WATER]  ←────────────●────────────→  [ICE]
```

建议显示：

```text
Water        Balance        Ice
```

或更加材质化的视觉表达。

核心要求：

- 单一比例条；
- 中心位置明确；
- 两端语义明确；
- 可直接 automation；
- 拖动时反馈连续。

---

## 7.2 Serial UI

Water → Ice：

```text
[ Water Amount ]  →  [ Ice Amount ]
```

Ice → Water：

```text
[ Ice Amount ]  →  [ Water Amount ]
```

建议视觉顺序与信号流顺序一致，但：

```text
water.amount
ice.amount
```

参数 ID 不能因控件位置变化而变化。

---

## 7.3 Global Mix

无论 routing：

```text
Global Mix
```

都保留。

语义始终为：

```text
Original Input ↔ Complete FRAZIL Wet Path
```

不能根据 RoutingMode 改变含义。

---

## 7.4 UI 隐藏不等于参数不存在

例如 Parallel 模式下可以隐藏：

```text
Water Amount
Ice Amount
```

但：

- Host automation lane 仍然存在；
- 参数值仍保存在 state 中；
- 切回 Serial 后恢复此前值。

Serial 模式下 Parallel Balance 同理。

---

## 7.5 Input / Output Gain UI

主界面始终提供 `Input` 与 `Output` 两个 gain control：

- Input 靠近信号入口；
- Output 靠近信号出口；
- 不随 RoutingMode 隐藏；
- 0 dB 有明确视觉基准；
- double-click 可 reset 至 0 dB；
- 显示单位为 dB；
- 两者均可 Host automation。

Input / Output meter 可作为 P1，不是 gain control 的前置条件。

## 7.6 Undo / Redo UI

建议在 Header 提供：

```text
↶ Undo    ↷ Redo
```

要求：

- 无可撤销项时 Undo disabled；
- 无可重做项时 Redo disabled；
- 新用户编辑发生后清除 Redo branch；
- 一次连续拖动只产生一步；
- 可选 tooltip 显示下一步动作名；
- Editor 有键盘焦点时，可在 P1 支持 Cmd/Ctrl+Z 及平台常见 Redo 快捷键。

---

# 8. 实时线程规则

所有进入 audio callback 路径的代码默认按实时系统处理。

## process() 中禁止

```text
new / delete
不可控 vector resize
磁盘 I/O
网络 I/O
sleep
阻塞 mutex
等待其他线程
GUI 操作
UndoManager / EditHistory 操作
console logging
异常作为正常控制流
```

必要内存应在：

```text
prepare()
```

阶段预分配。

---

## 8.1 随机 DSP

Water / Ice 很可能需要随机事件。

禁止生产代码依赖不可复现的全局随机状态。

建议：

```cpp
class RandomSource
{
public:
    void setSeed(uint64_t seed);
    float nextFloat() noexcept;
};
```

测试：

```text
fixed seed
```

正式运行：

```text
runtime seed
```

这样 render regression 可以重复。

---

# 9. 高内聚、低耦合设计规则

## 9.1 一个类只有一个主要变化原因

例如 `WaterProcessor` 不应该同时负责：

```text
Water DSP
GUI layout
state serialization
VST3 automation
routing
```

---

## 9.2 Routing 控制不进入 Water / Ice 内部

推荐：

```text
WaterProcessor:
Input → Full Water Transform

IceProcessor:
Input → Full Ice Transform

RoutingEngine:
决定怎样组合这些结果
```

避免：

```text
WaterProcessor::setParallelMode(...)
IceProcessor::setSerialIndex(...)
```

Water / Ice 不应知道自己处于 Parallel 还是 Serial。

---

## 9.3 模块通过数据接口沟通

推荐：

```cpp
water.process(buffer, waterParams);
```

避免：

```cpp
water.setPluginProcessor(this);
water.readAPVTS();
```

---

## 9.4 优先组合，不为未来大量继承

不建议早期建立：

```text
IDspModule
  ├ WaterModule
  ├ IceModule
  ├ FilterModule
  ├ ResonatorModule
  └ ...
```

再把所有东西 virtual。

两人项目更推荐：

> **统一接口习惯 + concrete types + composition。**

---

## 9.5 DSP primitive 真正复用后再下沉

只有 Water 使用的工具：

```text
dsp/water/...
```

Water / Ice 都真实依赖后，再考虑：

```text
dsp/primitives/...
```

避免为了 DRY 提前抽象。

---

# 10. 代码风格

建议：

```text
Type / Class           PascalCase
function               camelCase
local variable         camelCase
member variable        camelCase_
constant               kPascalCase
enum class             PascalCase
namespace              lowercase
file                   与主类型一致
```

统一 clang-format。

Header 应：

- 小；
- 自包含；
- 少 include；
- 不泄露无关实现。

注释主要解释：

```text
为什么
```

而不是重复代码已经表达的：

```text
做什么
```

禁止无解释魔法数字。

禁止全局可变状态。

---

# 11. 开发阶段

# M0 — Product Contract & Repository Bootstrap

## 目标

建立能够让两个人安全开始开发的最小工程制度。

## P0

- [ ] README
- [ ] CMake
- [ ] JUCE 接入
- [ ] VST3 target
- [ ] Standalone target
- [ ] Debug / Release build
- [ ] GitHub Actions 基础 CI
- [ ] main branch protection
- [ ] Issue template
- [ ] PR template
- [ ] PROJECT_SPEC
- [ ] PARAMETERS
- [ ] TESTING
- [ ] clang-format
- [ ] 基础单元测试框架

## Exit Criteria

```text
fresh clone
→ configure
→ build
→ run Standalone
→ load VST3
→ CI PASS
```

---

# M1 — Audio Skeleton & Parameter Contract

## 目标

先证明架构、参数和宿主通信成立。

## 必须

- [ ] AudioEngine
- [ ] ProcessSpec
- [ ] static ParameterLayout
- [ ] ParameterMapper
- [ ] ParameterSnapshot
- [ ] pass-through processing
- [ ] input.gain / output.gain 参数合同
- [ ] Input / Output gain processing skeleton
- [ ] EditHistoryManager skeleton
- [ ] state save / restore
- [ ] Host 能枚举核心参数
- [ ] 基础 automation test
- [ ] offline WAV render harness
- [ ] deterministic random source
- [ ] no-NaN / no-Inf test
- [ ] basic pluginval

## 重点

这一阶段不追求声音功能。

优先证明：

```text
architecture works
parameters work
automation works
state works
render works
plugin works
```

---

# M2 — Water Vertical Slice

## P0

- [ ] WaterProcessor 生命周期
- [ ] 第一版真正可听的 Water 算法
- [ ] Water enable
- [ ] Water 用户 macro
- [ ] parameter smoothing
- [ ] click-free bypass
- [ ] state restore
- [ ] render tests
- [ ] Listening Review
- [ ] pluginval

注意：

串联用的：

```text
water.amount
```

属于 routing / stage mix 参数，不应迫使 WaterProcessor 内部绑定该语义。

---

# M3 — Ice Vertical Slice

## P0

- [ ] IceProcessor
- [ ] 第一版摩擦 / 冰晶 / 裂纹质感机制
- [ ] Ice enable
- [ ] Ice 用户 macro
- [ ] smoothing
- [ ] bypass
- [ ] state restore
- [ ] render test
- [ ] Listening Review

原则：

Water 与 Ice 共享：

```text
生命周期
参数传递方式
测试方法
```

而不是 DSP 算法。

---

# M4 — Routing & Material Engine Integration

## 目标

实现已经确认的控制合同，而不是继续讨论一种统一比例模型。

## P0

- [ ] Parallel signal path
- [ ] Water → Ice
- [ ] Ice → Water
- [ ] parallel.balance
- [ ] water.amount
- [ ] ice.amount
- [ ] Input Gain
- [ ] Global Mix
- [ ] Output Gain
- [ ] Enable combinations
- [ ] routing state restore
- [ ] mode switch state retention
- [ ] automation smoothing
- [ ] loudness-matched routing A/B
- [ ] click-free routing changes
- [ ] render tests

## 已确认行为

### Parallel

```text
一条 Balance 控制 Water / Ice 比例
```

### Serial

```text
Water Amount + Ice Amount 独立控制
```

### 时间上的 Water → Ice 变化

```text
交给 DAW automation
```

插件不增加内部 morph timeline。

---

# M5 — Product UX / GUI

## P0

- [ ] 主布局
- [ ] Water / Ice 独立 Enable
- [ ] Routing selector
- [ ] Parallel Balance bar
- [ ] Serial Water Amount
- [ ] Serial Ice Amount
- [ ] Input Gain
- [ ] Global Mix
- [ ] Output Gain
- [ ] Undo button
- [ ] Redo button
- [ ] 核心 Water macro
- [ ] 核心 Ice macro
- [ ] mode-dependent visibility
- [ ] automation 操作一致
- [ ] parameter tooltip
- [ ] Resize 策略

## P1

- [ ] 简单视觉反馈
- [ ] reset / default behavior
- [ ] accessibility 基本检查
- [ ] routing flow visual hint
- [ ] Undo / Redo keyboard shortcuts
- [ ] Undo action-name tooltip

## P2 暂缓

- 粒子动画
- GPU 特效
- 复杂皮肤系统
- preset browser
- online preset cloud
- 多主题

原则：

> GUI 服务于声音和操作速度，而不是成为独立技术项目。

---

# M6 — Hardening / Beta

## 必须

- [ ] pluginval 高严格度
- [ ] Debug / Release
- [ ] AddressSanitizer
- [ ] 长时间运行
- [ ] sample-rate matrix
- [ ] block-size matrix
- [ ] mono / stereo
- [ ] parameter fuzz
- [ ] input/output gain automation stress
- [ ] automation stress test
- [ ] undo/redo transaction stress test
- [ ] state stress test
- [ ] mode-switch automation test
- [ ] 多实例测试
- [ ] DAW compatibility matrix
- [ ] CPU benchmark
- [ ] memory benchmark
- [ ] listening regression
- [ ] crash / bug triage

建议基础矩阵：

```text
44.1 kHz
48 kHz
96 kHz

32
64
128
256
512
1024 samples
```

---

# M7 — v1.0 Release

Feature Freeze 后只允许：

- bug fix；
- compatibility fix；
- performance fix；
- documentation；
- release packaging。

Release Gate：

```text
CI PASS
pluginval PASS
DAW automation PASS
state compatibility PASS
DAW matrix acceptable
performance budget PASS
listening regression PASS
known blockers = 0
```

---

# 12. 功能优先级

## P0

- Water；
- Ice；
- Water / Ice enable；
- RoutingMode；
- Parallel Balance；
- Serial Water Amount；
- Serial Ice Amount；
- Input Gain；
- Global Mix；
- Output Gain；
- Undo / Redo；
- stable parameter IDs；
- Host automation；
- AudioEngine；
- ParameterMapper；
- RoutingEngine；
- state save / restore；
- realtime safety；
- offline render；
- CI；
- pluginval；
- listening test。

---

## P1

- Water / Ice 额外 macro；
- Input / Output meters；
- Undo / Redo keyboard shortcut polish；
- 更好的 GUI feedback；
- presets；
- 更完善 DAW matrix；
- 性能优化。

---

## P2

暂缓：

- 内部 Water → Ice 自动 Morph；
- 内部 automation timeline；
- AAX；
- preset cloud；
- 用户账户；
- modulation matrix；
- 多频段；
- MIDI modulation；
- skin system；
- GPU 粒子动画；
- generic DSP node graph；
- 插件内部脚本语言。

原则：

> **由真实声音问题触发复杂度，而不是由“以后也许会用”触发复杂度。**

---

# 13. DSP 实验代码与生产代码

```text
experiments/
```

允许：

- 快速；
- hard-code；
- Python 原型；
- 临时参数；
- 失败。

```text
src/dsp/
```

要求：

- 实时安全；
- 生命周期明确；
- 参数明确；
- 可测试；
- 可读；
- 可复现；
- 性能受控。

流程：

```text
Idea
 ↓
Experiment
 ↓
Listening A/B
 ↓
Accepted
 ↓
Production DSP
 ↓
Tests
 ↓
PR
```

---

# 14. 测试体系

## L1 — Unit Test

测试：

```text
smoother
mapping
balance curve
stage mix
gain mapping
edit-history transaction grouping
random
filter
math
```

重点新增：

```text
ParallelBalance mapping
WaterAmount stage mix
IceAmount stage mix
InputGain / OutputGain mapping
mode-dependent parameter mapping
Undo / Redo transaction boundaries
```

---

## L2 — Processor Property Test

保证：

```text
finite input → finite output
parameter extremes → no NaN / Inf
sample-rate change → no crash
block-size change → no crash
```

---

## L3 — Routing Render Regression

固定：

```text
input WAV
parameters
routing mode
seed
sample rate
block size
```

至少保存：

```text
Parallel Water endpoint
Parallel center
Parallel Ice endpoint

Water→Ice 0/0
Water→Ice 100/0
Water→Ice 0/100
Water→Ice 100/100

Ice→Water 0/0
Ice→Water 100/0
Ice→Water 0/100
Ice→Water 100/100
```

---

## L4 — Host / Plugin Validation

验证：

- plugin lifecycle；
- state；
- editor；
- bus；
- parameter fuzz；
- automation；
- parameter enumeration；
- VST3 validation。

---

## L5 — Automation Acceptance Test

至少在目标 DAW 中验证：

### Parallel

```text
parallel.balance
```

能：

- 被宿主识别；
- 添加 automation lane；
- 录制 automation；
- 回放 automation；
- 快速变化无明显 click。

### Serial

```text
water.amount
ice.amount
```

都能独立：

- 被宿主识别；
- 添加 automation lane；
- 录制；
- 编辑；
- 回放。

必须专门测试：

```text
Water Amount 100 → 0
Ice Amount     0 → 100
```

并允许非对称 automation。

### Gain

`input.gain` 与 `output.gain` 都必须：

- 被宿主识别；
- 可录制 / 编辑 / 回放 automation；
- 快速变化无明显 click / zipper noise；
- default 0 dB；
- state restore 后数值一致。

---

## L6 — State / Mode Retention Test

测试：

1. Parallel 设置 Balance；
2. 切换 Serial；
3. 设置 Water Amount / Ice Amount；
4. 再切回 Parallel；
5. Balance 必须恢复原值；
6. 再切回 Serial；
7. 两个 Amount 必须恢复原值；
8. 保存 DAW project；
9. 重启宿主；
10. 所有值和 routing 恢复一致。

---

## L7 — Undo / Redo Acceptance Test

至少验证：

```text
一次 knob drag
→ 一次 Undo 回到 drag 前
→ 一次 Redo 回到 drag 后
```

并测试多个离散操作、RoutingMode 切换与 reset-to-default 的 transaction 边界。

以下内容不得污染插件 history：

```text
DAW automation playback
Host project restore
初始化参数同步
DSP smoothing
```

Host state restore 后：

```text
Undo history = cleared
Redo history = cleared
```

Undo / Redo 不能在 audio thread 执行，也不能破坏 Host parameter synchronization。

---

## L8 — Listening Test

固定素材：

```text
drums
vocal
piano
guitar
pad
bass
noise
impulse
```

每个核心 DSP / routing PR 输出：

```text
before
candidate A
candidate B
```

尽量 loudness match。

---

# 15. 性能设计

Beta 前建立：

```text
Reference Machine
Reference DAW
48 kHz / 128 samples baseline
```

记录：

```text
Water only
Ice only
Parallel
Water → Ice
Ice → Water
routing transition
automation stress
idle UI
animated UI
```

重点监控：

```text
audio thread time
peak callback time
allocation
memory
denormal
CPU spike
```

---

# 16. Git 与两人协作

采用轻量 GitHub Flow：

```text
main
 ├ feat/water-processor
 ├ feat/ice-fracture
 ├ feat/parallel-balance
 ├ feat/serial-stage-mix
 ├ fix/bypass-click
 └ test/automation
```

main 必须保持：

```text
buildable
testable
reviewed
```

推荐：

- Pull Request required；
- 1 approval；
- CI required；
- conversations resolved；
- force push disabled。

---

# 17. 两人职责

可以设置 primary ownership：

```text
Developer A
DSP / sound research primary

Developer B
plugin / architecture / tooling primary
```

但接口、routing、参数 ID、声音方向必须双人理解。

声音方向的重要 PR 两人共同听测。

---

# 18. Issue 工作流

```text
Backlog
  ↓
Ready
  ↓
In Progress
  ↓
Code Review
  ↓
Listening / Automation Test
  ↓
Done
```

推荐：

```text
每人最多 1 个 In Progress
团队 WIP ≤ 2
```

---

# 19. Definition of Ready

Issue 进入 Ready 前：

- [ ] 问题明确；
- [ ] 用户行为明确；
- [ ] Acceptance Criteria 明确；
- [ ] 自动测试方法明确；
- [ ] 是否需要 Listening Test 明确；
- [ ] 是否涉及参数 ID 明确；
- [ ] 是否影响 Host automation 明确；
- [ ] dependency 明确。

---

# 20. Definition of Done

- [ ] implementation complete
- [ ] Debug build
- [ ] Release build
- [ ] unit / DSP tests
- [ ] CI PASS
- [ ] no realtime-safety violation
- [ ] parameter smoothing checked
- [ ] Host automation checked
- [ ] Input / Output gain checked
- [ ] Undo / Redo transaction behavior checked
- [ ] bypass checked
- [ ] state save / restore checked
- [ ] mode value retention checked
- [ ] reference render checked
- [ ] DAW check where relevant
- [ ] listening test where relevant
- [ ] documentation updated
- [ ] other developer approved
- [ ] merged into main

---

# 21. PR 规范

标题示例：

```text
[DSP] Add first Water processor
[ROUTING] Add Parallel balance crossfade
[ROUTING] Add serial Water/Ice stage amount
[HOST] Expose routing parameters for automation
[FIX] Remove click during amount automation
[TEST] Add automation state retention test
```

PR 描述：

```text
## Problem

## Solution

## Architectural Impact

## Parameter / Automation Impact

## Test Plan

## Audio Evaluation

## Screenshots / Renders

## Checklist
```

若修改：

- parameter ID；
- parameter range；
- routing；
- state format；
- DSP 核心算法；

必须明确说明 Architectural / Automation Impact。

---

# 22. ADR

建议：

```text
docs/adr/
0001-framework-and-build.md
0002-parameter-model.md
0003-water-algorithm.md
0004-ice-algorithm.md
0005-routing-and-control-model.md
0006-host-automation-contract.md
0007-gain-staging.md
0008-plugin-edit-history.md
```

`0005-routing-and-control-model.md` 应记录本次已经确认的决定：

```text
Parallel
→ one Water/Ice Balance bar

Serial
→ independent Water Amount + Ice Amount

Temporal Water→Ice transition
→ DAW automation, no internal morph timeline
```

---

# 23. 可读性和维护性检查表

每次 Review 至少问：

### 架构

```text
这个类是否知道了不该知道的东西？
```

### Routing

```text
Water / Ice 是否开始知道自己处于 Parallel 或 Serial？
```

如果是，优先重构。

### 参数

```text
这个参数 ID 是否可能破坏已有 DAW automation？
```

### UI

```text
UI 隐藏控件是否错误地改变了 Host 参数结构？
```

### 实时安全

```text
automation 快速变化时是否可能 allocation / lock / click？
```

### 状态

```text
切换 routing 后，暂时无效的参数值是否仍然保留？
```

### Gain Staging

```text
Input Gain 是否仍位于所有处理之前？
Output Gain 是否仍只作用于最终输出？
```

### Undo / Redo

```text
这次变化是否真的来自用户 UI 编辑？
一次连续手势是否只生成一个 transaction？
Host automation 是否被错误记录？
```

### 抽象

```text
这个 abstraction 是解决真实问题，还是想象未来？
```

---

# 24. FRAZIL 特别需要避免的反模式

## God PluginProcessor

错误：

```text
PluginProcessor
├ Water
├ Ice
├ routing
├ stage mix
├ parameters
├ state
└ random
```

---

## DSP 直接读取 APVTS

错误：

```cpp
water.process(
    buffer,
    *apvts.getRawParameterValue("water.amount"));
```

ParameterMapper / Snapshot 应集中处理。

---

## 动态 Host 参数集合

禁止：

```text
Parallel → Host 只有 Balance
Serial   → Host 突然变成两个 Amount
```

Host 参数集合必须稳定。

---

## 用一个参数同时表示 Parallel Balance 与 Serial Amount

禁止把：

```text
MaterialBalance
```

在不同 routing 下解释成完全不同的含义。

应使用独立 ID：

```text
parallel.balance
water.amount
ice.amount
```

---

## 内部 Morph 系统重复 DAW 能力

v1 不应为了 Water → Ice 时间变化额外实现：

```text
timeline
sequencer
automation recorder
transition duration
```

除非后续用户验证证明宿主 automation 无法满足产品需求。

---

## 把 Host Automation 塞进 Undo 栈

禁止：

```text
parameterChanged(any source) → push Undo
```

历史必须由明确的 UI gesture / command boundary 产生。

## Input Gain 语义漂移

禁止在后续版本把 `input.gain` 从整个插件的输入 trim 改成 Wet-only drive；若需要 drive，应新增独立参数。

---

# 25. 推荐近期执行顺序

```text
1. 初始化 repository
2. CMake + JUCE
3. VST3 + Standalone pass-through
4. app / dsp / plugin / ui 边界
5. static ParameterLayout
6. AudioEngine
7. ParameterMapper / Snapshot
8. input/output gain skeleton
9. EditHistoryManager / Undo-Redo transaction skeleton
10. state save / restore
11. Host automation smoke test
12. Test framework
13. Offline render harness
14. CI
15. pluginval
16. Reference Pack
17. Water experiment
18. Water vertical slice
19. Ice experiment
20. Ice vertical slice
21. Parallel routing
22. Serial stage mix
23. Water→Ice / Ice→Water
24. automation stress test
25. Undo / Redo acceptance test
26. 参数和 UX 收敛
27. Final GUI
28. Beta hardening
29. v1.0
```

---

# 26. 第一批建议 Issues

## Infrastructure

```text
INFRA-001 Bootstrap JUCE/CMake project
INFRA-002 Add VST3 and Standalone targets
INFRA-003 Add CI
INFRA-004 Add branch / PR workflow
INFRA-005 Add formatting rules
```

## Architecture

```text
ARCH-001 Introduce AudioEngine
ARCH-002 Define static ParameterLayout
ARCH-003 Add ParameterMapper and Snapshot
ARCH-004 Define DSP lifecycle contract
ARCH-005 Add deterministic RandomSource
ARCH-006 Define Input / Output gain staging
ARCH-007 Add EditHistoryManager
```

## Host / Parameters

```text
HOST-001 Register stable core parameter IDs
HOST-002 Verify VST3 parameter enumeration
HOST-003 Add state save/restore
HOST-004 Add automation smoke test
HOST-005 Test inactive-mode parameter retention
HOST-006 Register input.gain / output.gain
HOST-007 Verify gain automation
```

## Testing

```text
TEST-001 Add unit test target
TEST-002 Add offline render harness
TEST-003 Create reference input pack
TEST-004 Integrate pluginval
TEST-005 Add routing render matrix
TEST-006 Add automation acceptance tests
```

## Water

```text
EXP-WATER-001 Research candidate Water algorithms
DSP-WATER-001 Implement Water vertical slice
```

## Ice

```text
EXP-ICE-001 Research candidate Ice algorithms
DSP-ICE-001 Implement Ice vertical slice
```

## Routing

```text
ROUTING-001 Implement Parallel dual branch
ROUTING-002 Implement parallel.balance
ROUTING-003 Implement serial StageMixer
ROUTING-004 Implement Water → Ice
ROUTING-005 Implement Ice → Water
ROUTING-006 Add click-free routing transition
```

## UI

```text
UI-001 Add Routing selector
UI-002 Add Parallel Water/Ice Balance bar
UI-003 Add Serial Water Amount control
UI-004 Add Serial Ice Amount control
UI-005 Add mode-dependent control visibility
UI-006 Verify all controls map to host parameters
UI-007 Add Input / Output gain controls
UI-008 Add Undo / Redo controls
UI-009 Group continuous edits into one history transaction
```

## State / History

```text
STATE-001 Implement bounded edit history
STATE-002 Exclude Host automation from plugin history
STATE-003 Clear history on Host state restore
STATE-004 Add Undo / Redo acceptance tests
```

---

# 27. 最终架构目标

最终调用关系：

```text
DAW / VST3 Host
      │
      │ automation / state
      ↓
PluginProcessor
      │
      ├── setup: ParameterLayout -> APVTS / Host Parameter Registry
      ├── state: Host State Adapter -> StateModel
      └── audio runtime: Host Parameter Atomics
                                  ↓
                           ParameterSnapshot
                                  ↓
                           ParameterMapper
                                  ↓
                           EngineParameters
                                  ↓
                             AudioEngine
      │
      ├→ Input Gain
      │
      ↓
RoutingEngine
 ┌────┴────┐
 ↓         ↓
Water     Ice
 │         │
 └────┬────┘
      ↓
Stage / Parallel Mix
      ↓
Global Mix
      ↓
Output Gain
      │
      ↓
DAW
```

UI 控制路径：

```text
PluginEditor
     │
     ├→ EditHistoryManager → Undo / Redo
     │
     ↓
Host-visible User Parameters
     │
     ↓
ParameterMapper
```

不是：

```text
PluginEditor → WaterProcessor
```

也不是：

```text
DAW Automation → WaterProcessor
```

---

# 28. 架构是否成功的判断标准

### 修改 Water 算法

主要影响：

```text
dsp/water/
```

### 修改 Ice 算法

主要影响：

```text
dsp/ice/
```

### 修改 Parallel Balance 曲线

主要影响：

```text
RoutingEngine / StageMixer
ParameterMapper
相关 tests
```

Water / Ice 算法本身基本不变。

### 修改 Serial Amount UI

主要影响：

```text
ui/
```

若参数 ID 和语义不变，DSP 不应因此重写。

### Host 自动化问题

主要影响：

```text
plugin parameter layer
ParameterMapper
smoothing / routing mix
integration tests
```

### 修改 Input / Output Gain

主要影响 `ParameterLayout`、AudioEngine gain stages、Gain UI 与 automation / smoothing tests；Water / Ice 算法不应改变。

### 修改 Undo / Redo

主要影响 `EditHistoryManager`、UI gesture boundaries 与 state/history integration tests；audio callback 与 Water / Ice DSP 不应依赖 history。

### 加 AU

主要影响：

```text
build / plugin configuration
```

核心 DSP 基本不变。

如果能够长期保持这种变化局部性，说明项目真正实现：

> **高内聚、低耦合。**

---

# 29. 项目最终原则

FRAZIL 的工程判断顺序：

```text
声音价值
    ↓
产品语义清晰
    ↓
正确性
    ↓
实时安全
    ↓
Host / automation 兼容性
    ↓
可测试性
    ↓
可维护性
    ↓
扩展性
```

两人开发最需要保护：

```text
共同理解
清晰边界
稳定参数合同
可预测的编辑历史
小型接口
可验证行为
可重复声音实验
稳定 main
```

---

# 30. 一句话开发准则

> **Experiment freely, integrate deliberately, keep the audio path simple, keep edit history local, and let the host own the timeline.**

中文：

> **实验可以大胆，集成必须克制；音频路径保持简单，撤销历史只管理插件内部编辑，时间线交给宿主，并把每个声音控制稳定地暴露出来。**
