# FRAZIL Module Index

本索引是模块边界、公共接口、依赖、线程和证据的快速入口。`Planned` 只表示计划合同，不表示当前源码已经存在；实现前必须先关联对应的 [Coding Plan](CODING_PLAN.md) work item。

| Module | Path | Responsibility | Public Interface | Depends On | Thread | Tests | ADR | Plan / Work Item | Status |
|---|---|---|---|---|---|---|---|---|---|
| PluginProcessor | `src/plugin/PluginProcessor.*` | JUCE lifecycle、bus、Host 参数/状态适配、把 block 交给 app | `prepareToPlay`、`processBlock`、state API | JUCE、app | message/audio boundary | `tests/`、CTest、pluginval | 0002, 0003 | `HOST-000`, `HOST-001`, `PARAM-001`, `STATE-001` | Implemented M1 parameter path; state remains skeleton |
| ParameterLayout | `src/plugin/ParameterLayout.*` | Host/JUCE-facing 静态 ID、类型、范围、默认值、label、choice、smoothing contract | layout factory/definitions | JUCE parameter types | message/setup | parameter regression | 0002 | `PARAM-001` | Implemented M1 |
| ParameterSnapshot | `src/app/ParameterSnapshot.*` | 每 block 一致的 POD Host 值 | `capture`、snapshot value type | value-type/atomic parameter source、app value types | audio read boundary | snapshot consistency/unit | 0002, 0003 | `PARAM-002` | Implemented M1 |
| ParameterMapper | `src/app/ParameterMapper.*` | product 参数到 engine 参数的 clamp、enum 和语义映射 | `map`、mapping value types | ParameterSnapshot、app value types | audio-safe | table-driven unit | 0002 | `PARAM-003` | Implemented M1 |
| StateModel | planned `src/app/StateModel.*` | versioned plugin state、Host restore 与内部 history 边界 | state model value API | app value types only | message/setup | round-trip/migration | 0002 | `STATE-001`, `STATE-002` | Planned M1 |
| AudioEngine | `src/app/AudioEngine.*` | 处理链生命周期与 gain/global mix 编排；wet path 当前为 pass-through；维护 ProcessSpec buffer invariant | `prepare`、`reset`、`process` | JUCE audio buffer、DSP primitives | audio | `frazil_tests` | 0003 | `ARCH-001`, `APP-001`, `DSP-001..004` | Implemented M1 skeleton with priming/invariant guard |
| WaterProcessor | planned `src/dsp/water/` | Water material transform | prepare/reset/process + Water params | dsp primitives only | audio | property/render/listening/perf | future Water ADR | `WATER-001..007` | Planned M2 |
| IceProcessor | planned `src/dsp/ice/` | Ice material transform | prepare/reset/process + Ice params | dsp primitives only | audio | property/render/listening/perf | future Ice ADR | `ICE-001..007` | Planned M3 |
| RoutingEngine | planned `src/dsp/routing/` | Parallel、Water -> Ice、Ice -> Water topology与transition | routing process/value types | stage interfaces, no algorithm internals | audio | routing matrix/click-free | 0001, 0003, ADR-R-001 | `ROUTE-001..011` | Planned M4 |
| StageMixer | planned `src/dsp/` | Serial stage dry/processed mix law | mix function/value types | no Host/UI | audio | endpoint/monotonicity | 0001 | `ROUTE-002` | Planned M4 |
| DryWetMixer | `src/dsp/` | Global dry/wet mix law | `mix` | no Host/UI | audio | endpoint/finite/monotonicity | 0001, 0003 | `DSP-002`, `ROUTE-008` | Implemented M1 primitive |
| RandomSource | `src/dsp/primitives/RandomSource.*` | M1 generic、可注入、可复现的 DSP 随机源；M2/M3 使用并补 algorithm-specific semantics | seed/next/reseed value API | standard library only | audio | fixed-seed determinism、instance isolation | future algorithm ADRs only when semantics become production decisions | `DSP-005` | Implemented M1 primitive |
| Smoothing primitives | `src/dsp/primitives/` | 连续参数 smoothing 和离散 transition primitives；重复 target 不重启 ramp，变化 target 可 retarget | `prepare`/`reset`/`setTarget`/`getNextValue` | standard library/value types | audio | step/transition/finite | 0003 | `DSP-004`, `AUTO-001` | Implemented M1 continuous primitive; discrete transition remains M4 |
| EditHistoryManager | planned `src/app/EditHistoryManager.*` | 有界 UI transaction undo/redo；隔离 Host automation/restore | transaction/undo/redo/clear | app value types only | message only | capacity/source/clear | 0002 | `HIST-001..004` | Planned M5 |
| PluginEditor | `src/plugin/PluginEditor.*` | 当前 M0 占位 editor 与 JUCE lifecycle | editor component | JUCE、PluginProcessor interface | message/UI | lifecycle/resize/pluginval | — | `UI-001` | Implemented M0 placeholder |
| UI components | planned `src/ui/` | 产品参数表达、attachment、gesture 和 UI transaction | narrow plugin parameter interface、narrow app edit/history command interface | plugin parameter interface、app edit/history command interface | message only | interaction/resize/automation | 0002 | `UI-001..008`, `HIST-002..004` | Planned M5 |

## Registration and update rules

- 新模块进入本表前必须有真实需求、路径、公共接口、依赖方向、线程模型、测试入口和 Coding Plan ID；人员 owner 由 GitHub Issue/Project 维护，不写死在长期索引中。
- 文件移动、公共接口、线程模型、参数/state/routing 边界或 ADR 变化时，同一 PR 更新本表和对应 module README。
- 当前不存在的 planned module 不得被 include、CMake target 或生产代码提前依赖；若计划与源码冲突，优先记录冲突并停止扩大范围。

## Modification Policy

本索引属于 LEVEL 3 MAINTAINED 文档，但其中的公共接口和依赖事实必须服从 Level 1/2 合同。修改需在相关代码/文档 PR 中同步，不能用索引文字掩盖未实现模块。
