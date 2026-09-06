# src/app/

## Purpose

应用层把 Host-facing 参数/状态合同转换为 DSP 可消费的值类型，并编排处理链生命周期。它不拥有 JUCE editor，也不实现 Water/Ice 算法。

## Responsibilities

- `AudioEngine` 的 prepare/reset/process 生命周期和未来的 gain/routing/global mix 编排；
- `ParameterSnapshot`、`ParameterMapper`、`StateModel` 和 `EditHistoryManager` 的应用层边界；
- 为 plugin 层提供小而明确的接口，为 dsp 层提供不依赖 APVTS/Host 的 engine 参数；为 UI 提供狭窄的 message-thread edit/history command boundary。

## Non-responsibilities

AudioEngine/DSP orchestration 不得直接注册 Host 参数、访问 UI 或 UndoManager；`EditHistoryManager` 只在 message thread 提供狭窄的编辑命令边界。app 不实现 Water/Ice 算法，也不决定 RoutingMode 的内部算法。

## Architecture / Data Flow

```text
PluginProcessor -> src/plugin/ParameterLayout -> ParameterSnapshot -> ParameterMapper -> AudioEngine -> dsp
Plugin Host State Adapter -> StateModel
UI -> narrow plugin parameter interface
UI -> narrow app edit/history command interface -> EditHistoryManager (message thread only)
```

## Public Interfaces

当前 `AudioEngine` 实际接口为 `prepare(double sampleRate, int maximumBlockSize, int numChannels)`、`reset()` 和 `process(juce::AudioBuffer<float>&) noexcept`。Snapshot/Mapper/StateModel/History 接口尚未实现，不能按规划接口写成当前事实。

## Parameter / Data Types

正式 Host 参数由 plugin 层当前 inline 注册；M1 `PARAM-001` 将迁移到 `src/plugin/ParameterLayout.*`。AudioEngine 当前没有正式 `EngineParameters`/`ProcessSpec`，这些属于 M1 合同。app 不依赖 ParameterLayout、PluginProcessor 或 Host adapter。

## Ownership & Lifetime

AudioEngine 为 PluginProcessor 的实例成员，拥有其生命周期内的准备状态；当前没有 app 层全局 runtime state。未来 buffer、smoother 和 DSP module 的所有权必须实例化并在 `prepare()` 前准备。

## Threading / Realtime Rules

`process` 位于音频线程边界：不得 I/O、logging、阻塞锁、UI/history 访问、运行时分配或直接多次读取 APVTS。每个 block 使用一致的 snapshot；规则详见 [CODE_STANDARDS.md](../../docs/CODE_STANDARDS.md) 和 [TESTING.md](../../docs/TESTING.md)。

## Implementation Overview

M0 只有 pass-through `AudioEngine`。真实 gain、Snapshot/Mapper、routing、Water/Ice 和 state/history boundary 按 [CODING_PLAN.md](../../docs/CODING_PLAN.md) 的 M1-M5 顺序实现。

## State / Tail / Latency

app 层不得自行宣称算法 tail 或 latency。v1 Host-reported processing latency 合同为 0 samples；Water/Ice 的 intentional effect delay/tail 由算法文档和测试描述，不能与 plugin processing latency 混同。

## Tests

当前以 `tests/unit/AudioEngineTests.cpp` 和 CTest smoke/lifecycle 为证据；后续按 [TESTING.md](../../docs/TESTING.md) 增加 snapshot、mapping、state、signal-chain 和 finite-output 测试。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)。

## Files

`AudioEngine.*` 当前存在；`ParameterSnapshot.*`、`ParameterMapper.*`、`StateModel.*`、`EditHistoryManager.*` 为 app 计划路径；`ParameterLayout.*` 为 `src/plugin/` 计划路径。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。公共接口、依赖或线程事实变化时必须与代码、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md)、相关测试和合同文档同一 PR 更新。
