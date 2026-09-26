# FRAZIL 参数与状态合同

## Latency policy revision

[ADR-0007](adr/0007-minimum-practical-processing-latency.md) proposes minimum
practical declared processing latency; independent Joint Gate is pending. Current
plugin/accepted M1 zero-sample evidence remains unchanged. The authorized offline
[D1 study](../experiments/water/EXP-W-FD-002.md) is separate from production activation.


> 状态：M1 core contract draft；v1 Host API freeze 由 `PARAM-FREEZE-001` 完成<br>
> 适用：M1 至 v1.0  
> 变更规则：任何 ID、范围、默认值、单位、routing 语义或 state schema 变更必须同步测试与 ADR。

## Modification Policy

参数注册表、参数语义和已冻结的 Host 兼容性规则属于 LOCKED/CONTROLLED 合同：已冻结内容只能通过明确 issue、必要 ADR、迁移策略和兼容性测试变更。解释性文字可维护，但不得借此改变参数事实；详细分级规则见 `docs/DOCUMENT_GOVERNANCE.md`。

## 1. Host 参数注册表

所有参数在插件构造时一次性静态注册。`enabled` 与 `routing.mode` 也作为可自动化离散参数暴露；Undo/Redo 不注册。

| ID | Host 名称 | 类型/范围 | 默认 | DSP 语义 | 当前模式相关性 | 平滑/切换 |
|---|---|---|---:|---|---|---|
| `water.enabled` | Water Enabled | bool | on | Water stage/branch 是否参与处理 | 全部 | click-free bypass |
| `ice.enabled` | Ice Enabled | bool | on | Ice stage/branch 是否参与处理 | 全部 | click-free bypass |
| `routing.mode` | Routing Mode | choice: Parallel, Water -> Ice, Ice -> Water | Parallel | wet path 拓扑 | 全部 | 短交叉淡化 |
| `parallel.balance` | Parallel Balance | 0..1 | 0.5 | 0=Water，1=Ice | Parallel | block snapshot -> sample-aware DSP smoother |
| `water.amount` | Water Amount | 0..1 | 1.0 | Serial Water stage dry/wet | Serial | block snapshot -> sample-aware DSP smoother |
| `ice.amount` | Ice Amount | 0..1 | 1.0 | Serial Ice stage dry/wet | Serial | block snapshot -> sample-aware DSP smoother |
| `input.gain` | Input Gain | -24..+24 dB | 0 dB | 完整插件输入 trim | 全部 | block target -> sample-aware linear smoother |
| `global.mix` | Global Mix | 0..1 | 1.0 | post-input dry reference 与完整 wet path 混合 | 全部 | block snapshot -> sample-aware DSP smoother |
| `output.gain` | Output Gain | -24..+24 dB | 0 dB | Global Mix 后最终 trim | 全部 | block target -> sample-aware linear smoother |

计划中的声音 macro 仅在实验验证后加入。Water 的 M2 candidate controls 见下节；Ice candidate
仍保持 `ice.character`、`ice.fracture`，且本次 Water-focused revision 不改变 Ice。任何 candidate
加入正式 registry 前必须定义用户听感语义、范围、默认值、mapping、smoothing、automation test 和
state behavior。

### 1.1 M2 Water candidate controls（未注册、未冻结）

Water 的产品方向已确定为双模式 `Fluid` / `Resonant`，但下列 ID、类型和语义仍是 M2 candidate
contract，不是当前九参数生产 registry、`ParameterLayout`、`schemaVersion=1` 或已冻结 v1 Host API
的一部分：

| Candidate ID | Candidate status / expected type | User semantic | Mode relevance | Planned automation / transition | Adoption prerequisite |
|---|---|---|---|---|---|
| `water.model` | M2 candidate；离散 choice，working order `Fluid`, `Resonant` | 选择两种有意区分的 Water material behavior；不是“real/fake”或质量档位 | 两模式选择器 | 若正式采用，需静态注册、确定 choice order、block snapshot、click-free bounded transition、rapid automation 与最终值测试 | Water ADR、transition/state ownership、range/default/choice freeze、save/restore 和 compatibility fixtures |
| `water.size` | M2 candidate；normalized continuous product macro | `Fine / Small / Bright <-> Large / Deep`；回答“Water material 的尺度是什么”，不表示 loudness、Amount、density 或 energy | 两模式共享同一高层语义 | 连续 Host automation；sample-aware smoothing；映射在合适处保持单调、可感知一致 | 两模式 mapping、范围/default、非线性曲线、listening/property evidence 和 state evolution review |
| `water.motion` | M2 candidate；normalized continuous product macro | `Calm / Stable <-> Active / Flowing`；回答“Water material 的时间活动度是什么” | 两模式共享语义；Resonant 的变化应刻意比 Fluid 更 subtle | 连续 Host automation；sample-aware smoothing；快速 automation 不得 click/zipper，能量变化须有界 | mode-specific mapping、loudness/energy strategy、范围/default、listening/property evidence 和 state evolution review |
| `water.decay` | M2 candidate；experiment-only normalized `[0, 1]`，未注册、未冻结 | Water response persistence；`Short / Tight <-> Long / Lingering`；回答“一次输入激发的响应持续多久” | Fluid Bubble/Droplet response；Resonant modal response；Flow 默认无直接 destination | 若正式采用，需确定 live damping / event-latched policy、existing-state behavior、smoothing 和快速 automation 安全性 | EXP-W-001..003、Joint Gate、ADR-W-001、mode-specific curves、tail/overlap evidence 和 parameter/state compatibility |

这些 candidate 的完整责任链必须在 M2 evidence 中逐项闭环：

```text
user perceptual intention
  -> normalized product parameter
  -> ParameterMapper: Host/application values -> normalized WaterProductValues
  -> WaterMacroMapper: Water product values -> mode-specific bounded DSP targets
  -> bounded DSP quantities
  -> expected audible consequence
  -> automation/smoothing or transition requirements
  -> listening/property/state validation
```

未来 Water mapping 的唯一职责链为：

```text
Host / Developer Control -> ParameterSnapshot -> ParameterMapper
  -> WaterProductValues { model, size, motion, decay } (Water-domain value type)
  -> WaterMacroMapper (Water domain) -> FluidTargets / ResonantTargets -> DSP components
```

这是 planned value boundary，不是当前已接线的 runtime path，也不授权把 Developer controls 注册到 APVTS。
`WaterProductValues` 是 Water-domain normalized product value representation；推荐未来路径为
`src/dsp/water/WaterProductValues.h`，`WaterModel` 在同一或相邻 Water-domain value layer。
`ParameterMapper` 是这些值的 producer/adapter，不拥有类型定义；`WaterMacroMapper` 是 consumer 和唯一的
mode-specific semantic mapper。允许 `app -> Water domain`，禁止 `Water domain -> app`；不得把生产定义
放在 app/plugin/ui。该 small/plain value type 无 JUCE/APVTS/UI、分配或 DSP runtime state；完整排除项见
[Architecture §5.3](FRAZIL_PROJECT_ARCHITECTURE_v0.3.md#53-waterprocessor)。
当前九参数 Snapshot/Mapper 路径与独立的 Developer experiment snapshot 保持原状；未来实验适配应复用同一
产品值语义，不另建一套 DSP destination mapping。`ParameterMapper` 只解释 Host/raw values、执行 finite
fallback、clamp、choice -> enum、dB -> linear，并准备 normalized product/domain values。
NaN/Inf、越界值和 invalid choice/enum 在此 application boundary 按明确 fallback 规则清理；WaterMacroMapper
接收 finite normalized values 和有效 WaterModel，仍须生成 defensive bounded targets，不把任意 clamp 散落到 primitives。
ParameterMapper 不认识 BubbleConfig、DropletConfig、FlowConfig、ModalConfig，也不决定 decay seconds、event probability、
trajectory interval、modal coefficient 或 voice lifetime。

`WaterMacroMapper` 唯一拥有 normalized Water values -> mode-specific bounded targets 的转换；它位于
Water domain，以小型 pure-C++ value types 表达，deterministic、allocation-free、unit-testable，独立于
JUCE/APVTS/UI 和 DSP state。DSP primitive 只消费 `decaySeconds`、`eventRateHz`、`targetIntervalSeconds`、
`rootFrequencyHz` 等工程量，不认识 `water.decay` / `water.motion` 等产品 ID。这里不实现 mapper 或 framework。

- `water.model`：ParameterMapper 将稳定 choice 转为 WaterModel enum；WaterMacroMapper 按该 enum 选择目标集；engine/ADR 决定 residual ownership
  和 transition。预期结果是可辨识且可切换的 Fluid/Resonant 行为，不改变 routing mode。
- `water.size`：WaterMacroMapper 在 Fluid 主要映射 bubble radius/population scale -> resonance-frequency distribution；
  Resonant 映射 modal/root frequency scale -> coherent mode-family scaling。较大尺度通常对应较低
  resonance scale，较小尺度对应较高 resonance scale。Size 不映射 overall amount、general loudness、
  event density、Motion speed 或 response lifetime；最终频率范围和曲线必须由实验决定。
- `water.motion`：temporal activity。Fluid 可映射 bubble/droplet event activity/scheduling、Flow movement /
  trajectory rate 和 bounded stochastic variation；Resonant 可映射 subtle modal drift、excitation-distribution
  movement 和 bounded temporal variation。WaterMacroMapper 的 Motion 分支不直接控制 bubble/droplet/modal decay targets，
  不得主要变成 loudness、Amount 或任意 random depth；补偿策略由测量决定。
- `water.decay`：response persistence intention -> normalized candidate -> WaterMacroMapper -> bounded
  bubble/droplet response-decay 或 modal-damping quantities -> audible persistence -> smoothing / existing-state
  policy -> experiment/listening/property/state evidence。较高 Decay 指向更长响应；Resonant 对应较弱 damping，
  较低值对应较强 damping。WaterMacroMapper 的 Decay 分支不直接控制 event-rate / trajectory-rate targets；Flow 默认无直接
  Decay destination，不为覆盖全部组件而创造 Flow decay。Droplet 的 `refractorySeconds` 是当前 SPIKE 的
  scheduling quantity，不冻结为 Motion mapping；后续实验比较 sensitivity/probability/scheduling/refractory。

本候选修订由 [DOC-W-DECAY-001 / #32](https://github.com/jjjphens-dot/FRAZIL/issues/32) 跟踪，
其 proposal 已获独立 review；candidate baseline 随 PR #35 合入 main 生效，详见
[review/finalization evidence](planning/WATER_DECAY_CANDIDATE_REVISION.md#review-evidence-and-finalization-gate)。
这不是 Host adoption 或 Water instance acceptance。四个问题分别是 Model = what behavior、Size = how large、
Motion = how active、Decay = how persistent。职责遵守
**responsibility orthogonality + perceptual separability + bounded interaction**，不承诺所有声学结果严格正交。
High Motion + Long Decay 可以自然增加 overlap、
active voices、tail energy、apparent density 和 voice stealing；这些必须有界、可测并经音乐可用性 review。

Decay 的 `[0, 1]` 只是实验归一化表达，不冻结 Host range、unit、default 或 curve。Bubble、Droplet、Modal
分别研究 logarithmic/exponential/其他单调 perceptual curves；同一 normalized 值不意味着相同秒数。
低端不用 `Dry`，以免与 Amount/global mix 混淆。Decay 不是 gain、Amount、mix/balance、event rate、
Motion/Flow speed、generic reverb wetness/size、source-envelope release 或 Foley playback length。
Water 保持 continuous input-driven transformation；输入停止后已有响应/延迟 state 自然消散，不增加
Water Duration、Effect Duration、Run Time、Hold Time 或 whole-effect trigger/retrigger/hold/release/restart。

现有 SPIKE 的 prepare-time `decaySeconds` 不证明 realtime-automatable `water.decay`。EXP-W-002 比较
live damping（已有响应平滑采用新 damping）与 event-latched decay（新事件捕获值、旧事件保留值），记录
latched automation memory/lag；允许跨模式采用不同策略，但不在此预选。audio callback 不得因旋钮变化
调用完整 `prepare()`、分配、阻塞或重建不安全 state。正式采用仍须 EXP-W-001 accepted -> EXP-W-002/003
evidence -> Joint Gate -> ADR-W-001 Accepted -> parameter/state compatibility -> WATER-003/007；
`PARAM-FREEZE-001` 仍在 M2/M3 Exit 后。

`water.size` 的 compact UI display direction 候选为 `Fine <-> Deep`；tooltip/help 可以解释
small/bright 到 large/deep 的 material-scale 含义。最终 label、数值范围、默认值和曲线仍由
UX/listening/property evidence 决定，当前 candidate contract 不冻结这些选择。

`water.amount` 仍只表示 Serial Water stage amount，`parallel.balance` 仍只表示 Parallel 的 Water/Ice
比例，`global.mix` 仍是完整插件 dry/wet。Water source-preserving carrier 是内部架构属性，不新增
`water.dryWet`，以免形成 Water internal mix × Water Amount × Global Mix 的重叠用户语义。

内部 algorithmic LFO、smoothed random process 和 probabilistic events 可以由 Water engine 使用，
但不是用户参数。v1 当前不加入通用用户 LFO/modulation matrix；未来只有在真实用户证据支持时，
才可研究 destination 固定为 Motion 的受限 `Motion Mod`（LFO/Random、Rate、Depth、Smooth），且需
新的参数/state/automation review。

### 1.2 Developer/Experiment controls are not Host parameters

`DEV-UI-001` may operate the nine current Host parameters through the existing narrow parameter interface. During
Water experiments it may also display Developer/Experiment controls named Water Model, Water Size, Water Motion
and the planned Water Decay extension before their possible Host adoption. Current source only implements the
first three experiment controls; Decay UI/export is a separate follow-up. In that state:

- a developer control is not evidence of Host parameter adoption;
- `water.model`, `water.size`, `water.motion` and `water.decay` must not be added to `ParameterLayout` or `schemaVersion=1`;
- no stable ID/order/range/default/automation/state compatibility promise is created;
- export to an experiment config is a Sound Lab handoff, not a plugin state/preset format;
- only evidence, Joint Gate, the applicable Water ADR and explicit state/compatibility work may promote a control
  into the static Host registry.

Developer UI visibility never changes the Host-visible parameter set. The complete tooling boundary is maintained
in [`DEVELOPER_SOUND_TOOLS.md`](DEVELOPER_SOUND_TOOLS.md).

### 1.3 Protect research proposal (outside approved candidate baseline)

`Protect` / possible `Water Protect`, candidate ID `water.protect`, is a **PROPOSED / NOT ACCEPTED** research
control in [DOC-W-PROTECT-001](planning/WATER_PROTECT_CANDIDATE_REVISION.md). It is not part of v1.4's accepted
Model/Size/Motion/Decay candidate scope. No fifth macro, Host registration, range/default, choice order,
automation contract or persisted field is frozen. Current nine Host parameters and `schemaVersion=1` remain
unchanged; Developer snapshot/editor/export currently contain only Model/Size/Motion.

Experimental depth `P in [0,1]` would mean how much Water temporarily yields around source attacks. Candidate
tooltip: “Keeps source attacks clear by briefly reducing Water texture around transients.” P=0 is proposed
OFF; this is not a public parameter default. It must be distinguishable from Amount, Global Mix, Parallel
Balance, Motion and Decay. Attack/release/threshold/ratio are engineering controls, not proposed main UX.

The outcome may be Reject, Internal safeguard or User macro. Even an internal safeguard needs the applicable
product/algorithm adoption gate. Only a justified User macro outcome opens explicit registry/state/automation
design. If adopted, follow the existing Snapshot -> ParameterMapper -> Water-domain values -> WaterMacroMapper
-> DSP target boundary; no app-owned Water type or public `protect` field is introduced by this proposal.

### Contract freeze stages

M1 是 `core contract stabilization`：建立静态参数注册、Snapshot、mapping、state、automation granularity 和基础 smoothing 合同，但允许 Water/Ice 实验在 v1 API 范围内提出经过验证的 product macros。M2 Water 与 M3 Ice 完成后，`PARAM-FREEZE-001` 才是 `v1 host API freeze`：它冻结最终 Parameter ID、order、choice index、range、default、unit、smoothing、inactive-mode behavior，并要求 automation tests 与 state compatibility fixtures。此后 M4/M5 不得随意修改 Host Parameter ID；变更必须有 ADR 与 migration/compatibility review。

## 2. 已知 pre-v1 差异

M0 基线曾注册 `water.enable` 与 `ice.enable`；已合入 `main` 的 M1 `ParameterLayout` 已完成一次性迁移，当前生产代码采用合同 ID `water.enabled` 与 `ice.enabled`。公开版本前仍需补齐 state compatibility/migration 证据。

处理顺序：

1. 在任何公开 release/preset/session 之前完成一次性更名；
2. 将参数定义从 `PluginProcessor.cpp` 移至 `src/plugin/ParameterLayout.*`；
3. 添加精确 ID、顺序、类型、范围和默认值枚举测试；
4. 添加 state round-trip 测试；
5. 若确认已有外部 session 使用旧 ID，再通过 ADR 决定兼容读取，禁止默默丢值。

## 3. 信号和参数语义

```text
raw input
    ↓ input.gain
post-input signal ──────────────→ dry reference ──────┐
    ↓                                                  │
RoutingEngine -> Water/Ice -> wet path ────────────────┤
                                                       ↓ global.mix
                                                    mixed
                                                       ↓ output.gain
                                                    output
```

当前 Material DSP artifact 的 `ARCH-LAT-001` / ADR-0005 合同仍为 Host-reported 0 samples。
新的 latency policy 见 ADR-0007 proposed supersession：未来在 Joint Gate 后允许满足 fidelity gate 的最小确定 processing latency，显式测量并报告 Host；dry/wet、carrier、bypass、Parallel 与 Serial 路径必须补偿 engineering alignment。普通参数自动化不得改变 Host latency。本轮只实施离线研究与政策提案，不修改当前九参数、schema 或 Host reporting。
Water/Ice intentional effect delay/tail 由算法合同描述；它与 processing latency 不同，不被 branch compensation 消除。`ROUTE-011` 验证无未声明/未补偿 infrastructure latency，不要求 Water/Ice 的物理响应波形等同 dry waveform。


### Parallel

两模块都开启时，初版合同：

```text
wet = (1 - b) * Water(x) + b * Ice(x)
b = parallel.balance in [0, 1]
```

最终 crossfade law 可经 loudness-matched 听测调整，但端点、单调性和 ID 不能改变。只开启单支路时，wet 等于该支路；两者关闭时 wet path 等于 post-input pass-through。`water.amount`/`ice.amount` 保留值但不参与计算。

### Serial

Water -> Ice：

```text
w = mix(x, Water(x), water.amount)
wet = mix(w, Ice(w), ice.amount)
```

Ice -> Water：

```text
i = mix(x, Ice(x), ice.amount)
wet = mix(i, Water(i), water.amount)
```

关闭某模块时该 stage pass-through，其 amount 值保持不变。`parallel.balance` 保留值但不参与计算。

## 4. Snapshot 与 mapping

audio block 开始时从原子 Host 参数值建立一次 `ParameterSnapshot`，随后映射成只包含 POD/值类型的 `EngineParameters`。同一 block 不得在多个时间点直接读取 APVTS。

```cpp
struct ParameterSnapshot
{
    bool waterEnabled;
    bool iceEnabled;
    int routingModeIndex;
    float parallelBalance;
    float waterAmount;
    float iceAmount;
    float inputGainDb;
    float globalMix;
    float outputGainDb;
};
```

`ParameterSnapshot::capture` 从 plugin 在构造阶段缓存的 raw parameter atomics 形成这组值，每个 source 在一个 block 边界只读取一次。`ParameterMapper` 负责 finite fallback、边界夹紧、choice index 转 `RoutingMode` 和 dB→linear 语义；未来 Water 扩展只准备 normalized `WaterProductValues`，Water-specific DSP targets 由 `WaterMacroMapper` 负责。它不处理 buffer、不读取 UI、不拥有 smoother。当前 `EngineParameters` 使用 `inputGainLinear`/`outputGainLinear`，以明确 DSP 内部单位。

## 5. Smoothing 初始策略

M1/M4 的初始实现目标（不是不可变产品参数）：

- gain/mix/amount/balance：每个 process block 只建立一次 Host snapshot，再以 sample-aware smoother 处理；约 10 ms ramp 仅作为测试基线；
- enable/routing：离散状态在 block snapshot 中采样，再以约 20 ms 的 click-free transition 作为基线；这不是 sample-accurate Host automation 承诺；
- sample rate 改变或 `prepare()` 后重新计算 ramp samples；
- reset/state restore 的第一 block 不得产生未初始化 ramp；
- 最终时长由快速 automation、transient 素材、CPU profiling 和盲听共同决定，并记录 ADR。

避免为每个参数重复实现平滑器；但也不得把不同语义硬塞进一个全局 smoother。

## 6. Host automation 合同

- FRAZIL v1 不承诺 sample-accurate Host automation；Host-visible parameter state 在每个 process block 开始建立一次 coherent snapshot；
- 连续参数可被枚举、录制、编辑和回放；DSP 内可 sample-aware 平滑，快速变化不得明显 click/zipper；
- `routing.mode` 与 enabled 离散变化要有可预测、有限时长的显式 transition；
- inactive-mode 参数仍可被 Host 写入、保存和恢复；切回对应模式时使用最新值；
- Host automation timeline 优先：插件内部 Undo 恢复某值后，后续 automation point 可以再次覆盖；
- v1 不增加内部 morph time、sequencer 或 automation recorder；测试重点是无 click、无 zipper、无 NaN/Inf、block size 改变稳定和最终值正确。

### RandomSource semantics

测试的 deterministic fixed seed 与生产环境的 instance seed 是不同合同：

- render/property/unit test 可以注入 fixed seed，保证 candidate A/B 和 regression 可复现；
- production instance 不得因为共享全局 seed 而同步，具体 decorrelation 方式由 Water/Ice ADR 决定；
- 未来算法 ADR 必须说明不同 instance 是否 decorrelated、save/reopen 是否恢复 random state、offline render 是否 deterministic、实时播放是否允许每次不同、random state 是否属于 persistent state，以及 routing transition 时 random state 如何推进；
- 在这些问题决定前，不得把测试 fixed seed 直接当作生产持久化语义。

## 7. State schema

M1 应从裸 APVTS XML 约定升级为有版本的 state adapter：

```text
root type: FRAZIL
schemaVersion: 1
parameters:
  exactly nine canonical static Host parameter values;
  each serialized as a PARAM node with `id` and `value` attributes by the versioned Host State Adapter
APVTS remains the Host-side source/restore target and is not the stable wire-format contract itself.
non-parameter persistent UI state: 仅在确有需要时加入
edit history: 永不序列化
```

当前 STATE-001 实现使用 `schemaVersion = 1`。没有 schema version 但包含已知参数的 pre-v1 state，以及显式 `schemaVersion = 0`，只通过受限 migration 入口转换为当前 schema；`water.enable`/`ice.enable` 只作为历史输入名映射到 `water.enabled`/`ice.enabled`。未知 schema、损坏输入或缺失/越界字段使用安全默认值，不形成对所有历史 build 的永久兼容承诺。

Current schemaVersion=1 contains the nine canonical STATE-001 Host parameters.
This is the current schema contract, not a declaration that FRAZIL v1.0 will always expose exactly nine Host parameters.
This Water documentation revision does not change schemaVersion=1. Candidate `water.model`, `water.size`,
`water.motion`, `water.decay` must not be silently added as new required fields. Future Water/Ice macros adopted during M2/M3
must not be silently added as new required fields to schemaVersion=1.
Adding a new persistent Host parameter requires explicit state compatibility review, migration/default fixtures and an
agreed schema evolution strategy before registry/state changes are merged. This guard does not create schemaVersion=2.

规则：

- 保存/恢复全部静态参数，包括当前模式下暂时无效的值；
- 无效或未知 state 不崩溃，保留安全默认值；
- 旧 schema 的迁移发生在 message/control 路径，不进入 audio callback；
- Host restore 完成后清空 EditHistoryManager；
- state 测试至少覆盖 default、extremes、每个 routing、inactive value retention、损坏输入和旧 schema fixture。

## 8. 兼容性冻结点

在第一个公开 alpha 之前允许有记录的一次性合同修正。自公开 alpha 起：

- M1 的基础合同稳定不等于 v1 Host API 已冻结；`PARAM-FREEZE-001` 是冻结点；
- Parameter ID/choice index 视为 API；
- 删除参数应保留兼容占位或提供明确迁移；
- 范围和 mapping 的听感变化必须评估旧 automation；
- state schema 只前向演进并保留受支持版本的迁移测试。
