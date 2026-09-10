# src/plugin/

## Purpose

提供 JUCE `AudioProcessor`/editor、VST3/Standalone glue、Host 参数入口和 state adapter。该层是 Host 边界，不承载 Water/Ice 或 routing 算法。

## Responsibilities

- `PluginProcessor` 的 bus、生命周期、APVTS 当前骨架、`processBlock` 和 versioned XML state round-trip；
- `PluginEditor` 的当前 M0 占位界面与 JUCE editor lifecycle；
- `src/plugin/ParameterLayout.*` 负责 Host/JUCE-facing 静态参数注册，并使用 app 层 `ParameterContract` 的共享数值合同；`PluginProcessor` 缓存原子参数源并在 audio block 边界创建 Snapshot，再调用 app 层。

## Non-responsibilities

不得在 PluginProcessor 中实现 DSP transform、routing math、mix law、UI business logic 或内部 undo/history；不得让 DSP 直接访问 APVTS。

## Architecture / Data Flow

```text
Setup / Construction:
PluginProcessor -> src/plugin/ParameterLayout -> APVTS / Host Parameter Registry

Audio Runtime:
Host Parameter Atomics -> ParameterSnapshot -> ParameterMapper -> EngineParameters -> AudioEngine -> dsp

State:
Host state <-> Plugin Host State Adapter -> app StateModel

PluginEditor -> plugin parameter interface
```

## Public Interfaces

当前 `PluginProcessor` 暴露 JUCE lifecycle、`prepareToPlay`、`processBlock`、editor 和 state API，并只提供 `parameter`、`rawParameterValue`、`parameterRange` 三个狭窄的 setup/integration accessor；APVTS 与 state tree 保持 private。参数 layout 已迁移到 `src/plugin/ParameterLayout.*`，并由 M1 合同测试固定顺序和 ID。`HostStateAdapter` 将 APVTS state 转换为带 `schemaVersion=1` 的稳定 envelope，并把 restore 留在非音频线程。

`ParameterLayout` 只参与 Plugin construction/setup 的稳定 Host 参数注册，不参与 per-block audio runtime chain；runtime 从 Host Parameter Atomics 建立 `ParameterSnapshot` 开始。

## Parameter / State Contract

当前代码注册合同 ID `water.enabled`/`ice.enabled` 以及其余 7 个核心参数。state XML 现在由 `HostStateAdapter` 输出 `FRAZIL` root、`schemaVersion=1` 和全部 9 个 canonical 参数；schema-less/legacy `water.enable`、`ice.enable` 进入已知 pre-v1 migration，未知或损坏输入使用安全默认值。Host restore 与 plugin history 的边界由 STATE-001 定义；`EditHistoryManager` remains planned for M5 (HIST-001..004)。

## Ownership & Lifetime

PluginProcessor 拥有 APVTS、AudioEngine 和 editor 生命周期；JUCE factory/editor 中的生命周期分配属于框架边界。Plugin Host State Adapter 调用 app StateModel，但 StateModel 不反向依赖 plugin。新增跨层对象必须明确 owner、线程和销毁顺序，不得引入全局 service locator。

## Threading / Realtime Rules

`processBlock` 是实时入口：不得 I/O、logging、阻塞锁、UI/history 访问或不可控分配。Host 参数在 block 开始形成一致 Snapshot；state restore 在非音频线程执行，并清空未来的内部 history。

## Implementation Overview

M1 当前已把 `processBlock` 接入一次 Snapshot、Mapper 和带 smoothing 的 gain/mix skeleton；wet path 仍为 pass-through。M1-C 已把 `processBlock` 之外的 state save/restore 接入 versioned `HostStateAdapter`/`StateModel` boundary；plugin integration evidence 已覆盖连续 gain automation 进入 audio path，以及三种 routing mode 切换后的 inactive value retention/state reopen。真实 Water/Ice、routing DSP 和产品 UI 按 Coding Plan 后续实现；`EditHistoryManager` remains planned for M5 (HIST-001..004)。

## Tests

当前证据为 CTest smoke/lifecycle、参数/mapper/engine/state unit tests、实际 PluginProcessor integration、TEST-002 processor property matrix（含 silence DC/max-magnitude、短/奇数 callback 和标准 reprepare lifecycle）、ARCH-LAT-001 neutral/dry impulse/metadata 与 M1 skeleton-tail regression、PERF-BASE-001 callback baseline 和 RENDER-001 offline smoke；这些测试覆盖参数类型/名称/单位/choice、versioned state round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、legacy ID migration、duplicate/nonnumeric/malformed invalid parser fallback、三种 routing、inactive retention、smoothing block regression、首 block priming、reset、runtime buffer invariant、finite output 和 M1 neutral/deterministic path repeatability。真实 state DAW restore、DAW matrix 与当前变更后的 pluginval/Release/ASAN evidence 必须按本轮实际运行结果记录。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)、[ADR-0004](../../docs/adr/0004-juce-and-ci-dependency-strategy.md)、[ADR-0005](../../docs/adr/0005-zero-sample-processing-latency.md)。

## Files

`PluginProcessor.*`、`PluginEditor.*`、`ParameterLayout.*`、`StateAdapter.*`、`README.md`。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。Host 参数、state、生命周期或依赖边界变化时，必须同步 `PARAMETERS.md`、相关 ADR、测试、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md) 和 PR 影响字段。
