# FRAZIL 参数与状态合同

> 状态：Pre-v1 contract draft  
> 适用：M1 至 v1.0  
> 变更规则：任何 ID、范围、默认值、单位、routing 语义或 state schema 变更必须同步测试与 ADR。

## 1. Host 参数注册表

所有参数在插件构造时一次性静态注册。`enabled` 与 `routing.mode` 也作为可自动化离散参数暴露；Undo/Redo 不注册。

| ID | Host 名称 | 类型/范围 | 默认 | DSP 语义 | 当前模式相关性 | 平滑/切换 |
|---|---|---|---:|---|---|---|
| `water.enabled` | Water Enabled | bool | on | Water stage/branch 是否参与处理 | 全部 | click-free bypass |
| `ice.enabled` | Ice Enabled | bool | on | Ice stage/branch 是否参与处理 | 全部 | click-free bypass |
| `routing.mode` | Routing Mode | choice: Parallel, Water -> Ice, Ice -> Water | Parallel | wet path 拓扑 | 全部 | 短交叉淡化 |
| `parallel.balance` | Parallel Balance | 0..1 | 0.5 | 0=Water，1=Ice | Parallel | sample/block ramp |
| `water.amount` | Water Amount | 0..1 | 1.0 | Serial Water stage dry/wet | Serial | sample/block ramp |
| `ice.amount` | Ice Amount | 0..1 | 1.0 | Serial Ice stage dry/wet | Serial | sample/block ramp |
| `input.gain` | Input Gain | -24..+24 dB | 0 dB | 完整插件输入 trim | 全部 | dB target -> linear ramp |
| `global.mix` | Global Mix | 0..1 | 1.0 | post-input dry reference 与完整 wet path 混合 | 全部 | sample/block ramp |
| `output.gain` | Output Gain | -24..+24 dB | 0 dB | Global Mix 后最终 trim | 全部 | dB target -> linear ramp |

计划中的声音 macro 仅在实验验证后加入，例如 `water.character`、`water.motion`、`ice.character`、`ice.fracture`。加入前必须定义用户听感语义、范围、默认值、mapping、smoothing、automation test 和 state behavior。

## 2. 已知 pre-v1 差异

当前 M0 代码注册的是 `water.enable` 与 `ice.enable`。目标合同采用 `water.enabled` 与 `ice.enabled`，与架构总纲一致。

处理顺序：

1. 在任何公开 release/preset/session 之前完成一次性更名；
2. 将参数定义从 `PluginProcessor.cpp` 移至 `ParameterLayout.*`；
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
    RoutingMode routingMode;
    float parallelBalance;
    float waterAmount;
    float iceAmount;
    float inputGainDb;
    float globalMix;
    float outputGainDb;
};
```

`ParameterMapper` 负责边界夹紧、choice 转 enum、dB/归一化语义和未来 macro 映射；不处理 buffer、不读取 UI、不拥有 smoother。

## 5. Smoothing 初始策略

M1/M4 的初始实现目标（不是不可变产品参数）：

- gain/mix/amount/balance：先以约 10 ms ramp 作为测试基线；
- enable/routing：先以约 20 ms 的 click-free crossfade 作为基线；
- sample rate 改变或 `prepare()` 后重新计算 ramp samples；
- reset/state restore 的第一 block 不得产生未初始化 ramp；
- 最终时长由快速 automation、transient 素材、CPU profiling 和盲听共同决定，并记录 ADR。

避免为每个参数重复实现平滑器；但也不得把不同语义硬塞进一个全局 smoother。

## 6. Host automation 合同

- 连续参数可被枚举、录制、编辑和回放；快速变化不得明显 click/zipper；
- `routing.mode` 与 enabled 离散变化要有可预测、有限时长的过渡；
- inactive-mode 参数仍可被 Host 写入、保存和恢复；切回对应模式时使用最新值；
- Host automation timeline 优先：插件内部 Undo 恢复某值后，后续 automation point 可以再次覆盖；
- v1 不增加内部 morph time、sequencer 或 automation recorder。

## 7. State schema

M1 应从裸 APVTS XML 约定升级为有版本的 state adapter：

```text
root type: FRAZIL
schemaVersion: integer
parameters: APVTS state
non-parameter persistent UI state: 仅在确有需要时加入
edit history: 永不序列化
```

规则：

- 保存/恢复全部静态参数，包括当前模式下暂时无效的值；
- 无效或未知 state 不崩溃，保留安全默认值；
- 旧 schema 的迁移发生在 message/control 路径，不进入 audio callback；
- Host restore 完成后清空 EditHistoryManager；
- state 测试至少覆盖 default、extremes、每个 routing、inactive value retention、损坏输入和旧 schema fixture。

## 8. 兼容性冻结点

在第一个公开 alpha 之前允许有记录的一次性合同修正。自公开 alpha 起：

- Parameter ID/choice index 视为 API；
- 删除参数应保留兼容占位或提供明确迁移；
- 范围和 mapping 的听感变化必须评估旧 automation；
- state schema 只前向演进并保留受支持版本的迁移测试。
