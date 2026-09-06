# src/plugin/

## Purpose

提供 JUCE `AudioProcessor`/editor、VST3/Standalone glue、Host 参数入口和 state adapter。该层是 Host 边界，不承载 Water/Ice 或 routing 算法。

## Responsibilities

- `PluginProcessor` 的 bus、生命周期、APVTS 当前骨架、`processBlock` 和 XML state round-trip；
- `PluginEditor` 的当前 M0 占位界面与 JUCE editor lifecycle；
- `src/plugin/ParameterLayout.*` 负责 Host/JUCE-facing 静态参数注册；`PluginProcessor` 缓存原子参数源并在 audio block 边界创建 Snapshot，再调用 app 层。

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

当前 `PluginProcessor` 暴露 JUCE lifecycle、`prepareToPlay`、`processBlock`、editor 和 state API；APVTS 仍为 public member，是后续需要收窄审查的技术债。参数 layout 已迁移到 `src/plugin/ParameterLayout.*`，并由 M1 合同测试固定顺序和 ID。

`ParameterLayout` 只参与 Plugin construction/setup 的稳定 Host 参数注册，不参与 per-block audio runtime chain；runtime 从 Host Parameter Atomics 建立 `ParameterSnapshot` 开始。

## Parameter / State Contract

当前代码注册合同 ID `water.enabled`/`ice.enabled` 以及其余 7 个核心参数。state XML 当前仍只证明骨架 round-trip，不代表 M1 版本化 StateModel、automation/history 边界已完成。

## Ownership & Lifetime

PluginProcessor 拥有 APVTS、AudioEngine 和 editor 生命周期；JUCE factory/editor 中的生命周期分配属于框架边界。Plugin Host State Adapter 调用 app StateModel，但 StateModel 不反向依赖 plugin。新增跨层对象必须明确 owner、线程和销毁顺序，不得引入全局 service locator。

## Threading / Realtime Rules

`processBlock` 是实时入口：不得 I/O、logging、阻塞锁、UI/history 访问或不可控分配。Host 参数在 block 开始形成一致 Snapshot；state restore 在非音频线程执行，并清空未来的内部 history。

## Implementation Overview

M1 当前已把 `processBlock` 接入一次 Snapshot、Mapper 和带 smoothing 的 gain/mix skeleton；wet path 仍为 pass-through。真实 Water/Ice、routing、版本化 state 和产品 UI 按 Coding Plan 后续实现。

## Tests

当前证据为 CTest smoke/lifecycle、Debug/Release/ASAN build 和参数/mapper/engine unit tests，包含参数类型/名称/单位/choice、smoothing block regression、首 block priming、reset 和 runtime buffer invariant；本轮 Debug VST3 的 pluginval strictness 5 已通过，automation、state compatibility、render、完整 property 和 DAW matrix 仍待执行。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)、[ADR-0004](../../docs/adr/0004-juce-and-ci-dependency-strategy.md)。

## Files

`PluginProcessor.*`、`PluginEditor.*`、`ParameterLayout.*`、`README.md`。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。Host 参数、state、生命周期或依赖边界变化时，必须同步 `PARAMETERS.md`、相关 ADR、测试、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md) 和 PR 影响字段。
