# src/ui/

## Purpose

承载 Developer diagnostics 呈现组件和未来产品控件。当前 `DeveloperDiagnosticsView` / `DeveloperLevelMeter` 是 Debug/ASAN-only GUI candidate；正式产品 UI 仍未实现，editor lifecycle 位于 `src/plugin/PluginEditor.*`。

## Responsibilities

- 当前 candidate：只读呈现既有 aggregate INPUT/OUTPUT Peak/RMS、runtime metadata 和 finite 状态；
- 未来产品参数控件、attachment、gesture、resize/accessibility 和 UI transaction；
- 只通过 plugin parameter interface 修改 Host 参数；
- 将 UI 事件交给 message-thread 的 state/history boundary。

## Non-responsibilities

不得直接创建或持有 Water/Ice/Routing DSP object，不得依赖 AudioEngine、DSP primitives、DSP buffer/state、PluginProcessor internals 或 APVTS internals，也不得在 UI 中执行实时音频处理。UI 只使用 narrow plugin parameter interface、narrow app edit/history command interface 和只读 diagnostics value boundary。

## Architecture / Data Flow

```text
UI -> narrow plugin parameter interface -> Host/APVTS
UI -> narrow app edit/history command interface -> EditHistoryManager (message thread)
PluginEditor -> DeveloperDiagnosticsView (snapshot values + effective routing text) -> DeveloperLevelMeter (linear amplitudes)
Audio thread -> no UI dependency
```

## Public Interfaces

`DeveloperDiagnosticsView::update` 在 message thread 接收既有 `DeveloperDiagnosticsSnapshot` 和 effective routing 文本（包含 active developer override）；`DeveloperLevelMeter::setLevels` 仅接收 linear peak/RMS。Meter 不读取 Processor、APVTS 或 transport。正式产品 UI component/attachment API 仍为 Planned。

## Ownership & Lifetime

UI 组件由 editor/component tree 拥有；attachment 的有效期不得超过对应 parameter/editor owner。不得使用全局 UI state 或隐藏 singleton。

## Threading / Realtime Rules

UI 只在 message thread 工作，不阻塞 audio thread；UI 不能读取或写入未经设计的实时 DSP 状态。Host automation 与 UI gesture 的边界必须由参数/state/history 合同说明。

## Implementation Overview

M5 才实现正式产品 UI。现有 DEV-UI-001 editor 的 diagnostics 呈现已在本 candidate 中抽离到本目录：
RMS 填充、Peak 竖线、-60 至 0 dBFS 图形标尺和当前快照数值；10 Hz message-thread 更新，无 smoothing、
history 或 Peak Hold。Release 不编译这些呈现源文件，并保留静态非开发占位界面。
该 candidate 的人工可用性验收仍待完成，不创建生产参数依赖。

若 M2 candidate controls 经 Water ADR、state compatibility 和 parameter freeze 正式采纳，planned Water
主区保持 Enable、Mode（Fluid/Resonant）、Size、Motion 与 Decay 的固定层级；切换 mode 不替换完整 panel，
Size/Motion/Decay 在两模式保持同一位置和高层语义。bubble radius、Q、modal count、event probability、Flow
delay depth 和 PRNG seed 等 engineering controls 不属于 first-pass main UI。以上均为 planned behavior，
当前 Production UI 未实现；Developer surface 目前仅有 Model/Size/Motion，Decay 为单独的 planned 扩展。

Size 的 compact display direction 候选为 `Fine <-> Deep`，tooltip/help 可解释 small/bright 到
large/deep 的 material-scale 含义；最终 label 仍待 UX/listening review。Fluid/Resonant 的简短描述应帮助
用户把它们理解为两种 Water behavior，而不是 real/fake、good/bad 或 quality levels。

## Tests

未来需要正式 UI 的 interaction、resize、attachment、automation display 和 pluginval/DAW evidence；
本目录尚无正式产品 UI 自动化测试框架。Diagnostics candidate 的构建、pluginval、尺寸/静音/合成信号
检查见 [GUI candidate evidence](../../docs/PROJECT_STATUS.md#29-diagnostics-gui-candidate)。
DEV-UI-001 的历史 Debug/ASAN 工程测试与窄范围 Host smoke 记录见
[`PROJECT_STATUS.md`](../../docs/PROJECT_STATUS.md#27-dev-ui-001-engineering-validation) 和
[`HOST-001 DAW smoke evidence`](../../docs/evidence/HOST-001-DAW-SMOKE-2026-09-13.md)；这些不等于产品 UI
或完整 Host acceptance。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)，以及未来 UI/state/history boundary ADR。

## Files

- `DeveloperDiagnosticsView.*`：runtime metadata、finite 和 INPUT/OUTPUT meter 的组合布局。
- `DeveloperLevelMeter.*`：无 Processor 依赖的 linear amplitude 到 dBFS 呈现。
- 正式产品 UI 仍为 M5 planned scope。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。UI 公共接口、参数 binding、线程或 transaction 边界变化时，必须同步 [MODULE_INDEX.md](../../docs/MODULE_INDEX.md)、参数/state/history 文档和测试计划。
