# src/ui/

## Purpose

承载未来产品编辑器和控件。UI 负责表达产品参数、发起 gesture/transaction 和显示状态；当前产品 UI 尚未实现，M0 占位 editor 位于 `src/plugin/PluginEditor.*`。

## Responsibilities

- 参数控件、attachment、gesture、resize/accessibility 和 UI transaction；
- 只通过 plugin parameter interface 修改 Host 参数；
- 将 UI 事件交给 message-thread 的 state/history boundary。

## Non-responsibilities

不得直接创建或持有 Water/Ice/Routing DSP object，不得依赖 AudioEngine、DSP primitives、DSP buffer/state、PluginProcessor internals 或 APVTS internals，也不得在 UI 中执行实时音频处理。UI 只使用 narrow plugin parameter interface 和 narrow app edit/history command interface。

## Architecture / Data Flow

```text
UI -> narrow plugin parameter interface -> Host/APVTS
UI -> narrow app edit/history command interface -> EditHistoryManager (message thread)
Audio thread -> no UI dependency
```

## Public Interfaces

正式 UI component/attachment API 为 Planned；必须使用小而稳定的 plugin parameter interface 和 app edit/history command interface，不把 DSP 类型传播到 UI。

## Ownership & Lifetime

UI 组件由 editor/component tree 拥有；attachment 的有效期不得超过对应 parameter/editor owner。不得使用全局 UI state 或隐藏 singleton。

## Threading / Realtime Rules

UI 只在 message thread 工作，不阻塞 audio thread；UI 不能读取或写入未经设计的实时 DSP 状态。Host automation 与 UI gesture 的边界必须由参数/state/history 合同说明。

## Implementation Overview

M5 才实现正式 `src/ui/` 组件；当前 `src/plugin/PluginEditor.*` 在 Debug/ASAN 提供 DEV-UI-001 开发面板，
在 Release 保留静态非开发占位界面。该面板不是产品 UI；不得因 README 中的规划内容提前创建生产依赖。

若 M2 candidate controls 经 Water ADR、state compatibility 和 parameter freeze 正式采纳，planned Water
主区保持 Enable、Mode（Fluid/Resonant）、Size 与 Motion 的固定层级；切换 mode 不替换完整 panel，
Size/Motion 在两模式保持同一位置和高层语义。bubble radius、Q、modal count、event probability、Flow
delay depth 和 PRNG seed 等 engineering controls 不属于 first-pass main UI。以上均为 planned behavior，
当前占位 editor 未实现。

Size 的 compact display direction 候选为 `Fine <-> Deep`，tooltip/help 可解释 small/bright 到
large/deep 的 material-scale 含义；最终 label 仍待 UX/listening review。Fluid/Resonant 的简短描述应帮助
用户把它们理解为两种 Water behavior，而不是 real/fake、good/bad 或 quality levels。

## Tests

未来需要 interaction、resize、attachment、automation display 和 pluginval/DAW evidence；当前无正式 UI 测试，具体 gate 见 [TESTING.md](../../docs/TESTING.md)。

## Related ADRs

[ADR-0002](../../docs/adr/0002-parameter-and-state-contract.md)，以及未来 UI/state/history boundary ADR。

## Files

当前仅有本 README；M5 计划路径为 `src/ui/`。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。UI 公共接口、参数 binding、线程或 transaction 边界变化时，必须同步 [MODULE_INDEX.md](../../docs/MODULE_INDEX.md)、参数/state/history 文档和测试计划。
