# src/app/

## Purpose

应用层把 Host-facing 参数/状态合同转换为 DSP 可消费的值类型，并编排处理链生命周期。它不拥有 JUCE editor，也不实现 Water/Ice 算法。

## Responsibilities

- `AudioEngine` 的 prepare/reset/process 生命周期、当前 gain/global-mix 编排和未来的 routing 编排；
- `ParameterContract.h` 提供不依赖 JUCE 的 Host-facing 范围、step、default 和 routing count；
- `ParameterSnapshot`、`ParameterMapper`、`StateModel` 和 `EditHistoryManager` 的应用层边界；
- 为 plugin 层提供小而明确的接口，为 dsp 层提供不依赖 APVTS/Host 的 engine 参数；为 UI 提供狭窄的 message-thread edit/history command boundary。

## Non-responsibilities

AudioEngine/DSP orchestration 不得直接注册 Host 参数、访问 UI 或 UndoManager；`EditHistoryManager` 只在 message thread 提供狭窄的编辑命令边界。app 不实现 Water/Ice 算法，也不决定 RoutingMode 的内部算法。

## Architecture / Data Flow

```text
Setup / Construction:
PluginProcessor -> src/plugin/ParameterLayout -> APVTS / Host Parameter Registry

Audio Runtime:
Host Parameter Atomics -> ParameterSnapshot -> ParameterMapper -> EngineParameters -> AudioEngine -> dsp

State:
Plugin Host State Adapter -> StateModel

UI -> narrow plugin parameter interface
UI -> narrow app edit/history command interface -> EditHistoryManager (message thread only)
```

## Public Interfaces

当前 M1 切片提供 `ProcessSpec`、`EngineParameters`、`ParameterSnapshot`、`ParameterMapper`，以及 `AudioEngine::prepare(const ProcessSpec&)`、`reset()` 和 `process(juce::AudioBuffer<float>&, const EngineParameters&) noexcept`。M1-C STATE-001 进一步提供 JUCE-free 的 `StateModel::Values`、`SerializedState`、`serialize`/`deserialize` value API；`EditHistoryManager` remains planned for M5 (HIST-001..004)。

## Parameter / Data Types

正式 Host 参数的 ID、名称和注册顺序由 plugin 层 `src/plugin/ParameterLayout.*` 集中注册；范围、step、default 和 routing count 的值合同由 app 层 `ParameterContract.h` 共享。app 层的 Snapshot 只接收由 plugin 缓存的原子参数指针，Mapper 输出不含 Host 对象的 `EngineParameters`；app 不依赖 PluginProcessor 或 Host adapter。

## Ownership & Lifetime

AudioEngine 为 PluginProcessor 的实例成员，拥有其生命周期内的准备状态；当前没有 app 层全局 runtime state。未来 buffer、smoother 和 DSP module 的所有权必须实例化并在 `prepare()` 前准备。

## Threading / Realtime Rules

`process` 位于音频线程边界：不得 I/O、logging、阻塞锁、UI/history 访问、运行时分配或直接多次读取 APVTS。每个 block 使用一致的 snapshot；规则详见 [CODE_STANDARDS.md](../../docs/CODE_STANDARDS.md) 和 [TESTING.md](../../docs/TESTING.md)。

## Implementation Overview

M1 已实现参数到 `AudioEngine` 的 Snapshot/Mapper 路径、Input/Output gain、global dry/wet mix law、prepare 阶段 dry scratch 和基础 smoothing。首个有效 block 会从实时 `EngineParameters` priming smoother；runtime buffer 超出 `ProcessSpec` 时不在音频线程扩容，而使用有限、确定性的 fallback。M1-C STATE-001 已实现版本化 state value/schema、known pre-v1 migration、invalid fallback 和全部 9 个参数的保留；JUCE `ValueTree`/XML conversion 由 plugin Host State Adapter 持有。Water/Ice、Routing 仍按 [CODING_PLAN.md](../../docs/CODING_PLAN.md) 的后续 milestone 实现；`EditHistoryManager` remains planned for M5 (HIST-001..004)；当前 wet path 仍是 post-input pass-through。

## State / Tail / Latency

app 层不得自行宣称算法 tail 或 latency。v1 Host-reported processing latency 合同为 0 samples；Water/Ice 的 intentional effect delay/tail 由算法文档和测试描述，不能与 plugin processing latency 混同。

## Tests

当前以 CTest smoke、`frazil_tests` 的 ProcessSpec/ParameterLayout/Snapshot/Mapper/StateModel/Host State Adapter unit cases、`frazil_plugin_integration` 的实际 PluginProcessor automation/state path、`frazil_processor_property` 的代表性 nominal/short-odd callback、silence、lifecycle/finite matrix、`frazil_latency_contract` 的 neutral/dry impulse/metadata 与 M1 skeleton-tail regression、手动运行的 `frazil_performance` steady-state/parameter-retarget baseline，以及 `frazil_render` 的 M1 pass-through offline smoke 为证据；完整 render matrix、真实 DAW restore 和完整 M1 gate 仍未完成。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)、[ADR-0005](../../docs/adr/0005-zero-sample-processing-latency.md)。

## Files

`AudioEngine.*`、`ProcessSpec.h`、`EngineParameters.h`、`ParameterContract.h`、`ParameterSnapshot.*`、`ParameterMapper.*` 和 `StateModel.*` 当前存在；`EditHistoryManager.*` 仍为 app 计划路径；`ParameterLayout.*` 属于已实现的 `src/plugin/` M1 路径。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。公共接口、依赖或线程事实变化时必须与代码、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md)、相关测试和合同文档同一 PR 更新。
