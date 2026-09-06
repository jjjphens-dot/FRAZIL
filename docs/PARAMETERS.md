# FRAZIL 参数与状态合同

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

计划中的声音 macro 仅在实验验证后加入，例如 `water.character`、`water.motion`、`ice.character`、`ice.fracture`。加入前必须定义用户听感语义、范围、默认值、mapping、smoothing、automation test 和 state behavior。

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

v1 Material DSP 采用 `ARCH-LAT-001` 的 processing-latency 合同：Host-reported processing latency 必须为 0 samples；Water/Ice 不得依赖 lookahead、FFT block latency、linear-phase、convolution 或 Host PDC 才能正确工作。Water/Ice 内部允许属于声音设计的 intentional effect delay/tail，例如 micro-delay、resonant ringing、comb/modal structure 或 natural decay；`intentional effect delay/tail != plugin processing latency`。`ROUTE-011` 只验证 routing/mixing infrastructure 不引入额外未声明 latency，不要求处理后的 Water/Ice waveform 与 dry waveform 逐样本对齐。

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

`ParameterSnapshot::capture` 从 plugin 在构造阶段缓存的 raw parameter atomics 形成这组值，每个 source 在一个 block 边界只读取一次。`ParameterMapper` 负责边界夹紧、choice index 转 `RoutingMode`、dB→linear 语义和未来 macro 映射；不处理 buffer、不读取 UI、不拥有 smoother。当前 `EngineParameters` 使用 `inputGainLinear`/`outputGainLinear`，以明确 DSP 内部单位。

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
schemaVersion: integer
parameters: APVTS state
non-parameter persistent UI state: 仅在确有需要时加入
edit history: 永不序列化
```

当前 STATE-001 实现使用 `schemaVersion = 1`。没有 schema version 但包含已知参数的 pre-v1 state，以及显式 `schemaVersion = 0`，只通过受限 migration 入口转换为当前 schema；`water.enable`/`ice.enable` 只作为历史输入名映射到 `water.enabled`/`ice.enabled`。未知 schema、损坏输入或缺失/越界字段使用安全默认值，不形成对所有历史 build 的永久兼容承诺。

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
