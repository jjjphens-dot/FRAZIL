# src/plugin/

## Purpose

提供 JUCE `AudioProcessor`/editor、VST3/Standalone glue、Host 参数入口和 state adapter。该层是 Host 边界，不承载 Water/Ice 或 routing 算法。

## Responsibilities

- `PluginProcessor` 的 bus、生命周期、APVTS 当前骨架、`processBlock` 和 XML state round-trip；
- `PluginEditor` 的当前 M0 占位界面与 JUCE editor lifecycle；
- `src/plugin/ParameterLayout.*` 将来负责 Host/JUCE-facing 静态参数注册，再创建 Snapshot 并调用 app 层。

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

当前 `PluginProcessor` 暴露 JUCE lifecycle、`prepareToPlay`、`processBlock`、editor 和 state API；APVTS 目前为 public member，是 M1 需要收窄审查的技术债。当前参数 layout 仍 inline 于 `PluginProcessor.cpp`，目标由 `PARAM-001` 迁移到 `src/plugin/ParameterLayout.*`。

`ParameterLayout` 只参与 Plugin construction/setup 的稳定 Host 参数注册，不参与 per-block audio runtime chain；runtime 从 Host Parameter Atomics 建立 `ParameterSnapshot` 开始。

## Parameter / State Contract

当前代码仍注册 `water.enable`/`ice.enable` 占位 ID；目标合同是 `water.enabled`/`ice.enabled`。state XML 当前只证明骨架 round-trip，不代表 M1 版本化 StateModel、automation/history 边界已完成。

## Ownership & Lifetime

PluginProcessor 拥有 APVTS、AudioEngine 和 editor 生命周期；JUCE factory/editor 中的生命周期分配属于框架边界。Plugin Host State Adapter 调用 app StateModel，但 StateModel 不反向依赖 plugin。新增跨层对象必须明确 owner、线程和销毁顺序，不得引入全局 service locator。

## Threading / Realtime Rules

`processBlock` 是实时入口：不得 I/O、logging、阻塞锁、UI/history 访问或不可控分配。Host 参数在 block 开始形成一致 Snapshot；state restore 在非音频线程执行，并清空未来的内部 history。

## Implementation Overview

M0 当前是 pass-through plugin shell：`processBlock` 将 buffer 交给 pass-through AudioEngine；正式 gain、Snapshot/Mapper、真实 DSP、routing 和产品 UI 按 Coding Plan 实现。

## Tests

当前证据为 CTest smoke/lifecycle、Debug/portable build 和已记录的 pluginval 结果；参数、automation、state compatibility、render 与 DAW matrix 仍按 [TESTING.md](../../docs/TESTING.md) 分阶段补齐。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)、[ADR-0004](../../docs/adr/0004-juce-and-ci-dependency-strategy.md)。

## Files

`PluginProcessor.*`、`PluginEditor.*`、planned `ParameterLayout.*`、`README.md`。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。Host 参数、state、生命周期或依赖边界变化时，必须同步 `PARAMETERS.md`、相关 ADR、测试、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md) 和 PR 影响字段。
