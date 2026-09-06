# FRAZIL Module Index

本索引是模块边界、公共接口、依赖、线程和证据的快速入口。`Planned` 只表示计划合同，不表示当前源码已经存在；实现前必须先关联对应的 Coding Plan work item。

| Module | Path | Responsibility | Public Interface | Depends On | Thread | Tests | ADR | Status |
|---|---|---|---|---|---|---|---|---|
| PluginProcessor | `src/plugin/PluginProcessor.*` | JUCE lifecycle、bus、Host 参数/状态适配、把 block 交给 app | `prepareToPlay`、`processBlock`、state API | JUCE、app | message/audio boundary | `tests/`、CTest、pluginval | 0002, 0003 | Implemented M0 skeleton |
| ParameterLayout | planned `src/app/ParameterLayout.*` | 静态 ID、类型、范围、默认值、label、smoothing contract | layout factory/definitions | JUCE parameter types | message/setup | parameter regression | 0002 | Planned M1 |
| ParameterSnapshot | planned `src/app/ParameterSnapshot.*` | 每 block 一致的 POD Host 值 | snapshot value type | Host parameter interface | audio read boundary | snapshot consistency/unit | 0002, 0003 | Planned M1 |
| ParameterMapper | planned `src/app/ParameterMapper.*` | product 参数到 engine 参数的 clamp、enum 和语义映射 | mapping function/value types | ParameterSnapshot、app value types | audio-safe | table-driven unit | 0002 | Planned M1 |
| StateModel | planned `src/app/StateModel.*` | versioned plugin state、Host restore 与内部 history 边界 | state model/adapter | app value types、plugin adapter | message/setup | round-trip/migration | 0002 | Planned M1 |
| AudioEngine | `src/app/AudioEngine.*` | 处理链生命周期与编排；当前为 pass-through | `prepare`、`reset`、`process` | JUCE audio buffer today; future dsp | audio | `tests/unit/AudioEngineTests.cpp` | 0003 | Implemented M0 skeleton |
| WaterProcessor | planned `src/dsp/water/` | Water material transform | prepare/reset/process + Water params | dsp primitives only | audio | property/render/listening/perf | future Water ADR | Planned M2 |
| IceProcessor | planned `src/dsp/ice/` | Ice material transform | prepare/reset/process + Ice params | dsp primitives only | audio | property/render/listening/perf | future Ice ADR | Planned M3 |
| RoutingEngine | planned `src/dsp/routing/` | Parallel、Water -> Ice、Ice -> Water topology与transition | routing process/value types | stage interfaces, no algorithm internals | audio | routing matrix/click-free | 0001, 0003, ADR-R-001 | Planned M4 |
| StageMixer | planned `src/dsp/` | Serial stage dry/processed mix law | mix function/value types | no Host/UI | audio | endpoint/monotonicity | 0001 | Planned M4 |
| DryWetMixer | planned `src/dsp/` | Global dry/wet mix law | mix function/value types | no Host/UI | audio | endpoint/finite | 0001, 0003 | Planned M1/M4 |
| RandomSource | planned `src/dsp/primitives/` | 可注入、可复现的 DSP 随机源 | seed/next value API | standard library only | audio | fixed-seed determinism | future DSP ADR | Planned M2/M3 |
| Smoothing primitives | planned `src/dsp/primitives/` | 连续参数 smoothing 和离散 transition primitives | prepare/setTarget/process | standard library/value types | audio | step/transition/finite | 0003 | Planned M1/M4 |
| EditHistoryManager | planned `src/app/EditHistoryManager.*` | 有界 UI transaction undo/redo；隔离 Host automation/restore | transaction/undo/redo/clear | StateModel value types | message only | capacity/source/clear | 0002 | Planned M5 |
| PluginEditor | `src/plugin/PluginEditor.*` | 当前 M0 占位 editor 与 JUCE lifecycle | editor component | JUCE、PluginProcessor interface | message/UI | lifecycle/resize/pluginval | — | Implemented M0 placeholder |
| UI components | planned `src/ui/` | 产品参数表达、attachment、gesture 和 UI transaction | component/value binding API | plugin parameter interface | message only | interaction/resize/automation | 0002 | Planned M5 |

## Registration and update rules

- 新模块进入本表前必须有真实需求、owner、路径、公共接口、依赖方向、线程模型、测试入口和 Coding Plan ID。
- 文件移动、公共接口、线程模型、参数/state/routing 边界或 ADR 变化时，同一 PR 更新本表和对应 module README。
- 当前不存在的 planned module 不得被 include、CMake target 或生产代码提前依赖；若计划与源码冲突，优先记录冲突并停止扩大范围。

## Modification Policy

本索引属于 LEVEL 3 MAINTAINED 文档，但其中的公共接口和依赖事实必须服从 Level 1/2 合同。修改需在相关代码/文档 PR 中同步，不能用索引文字掩盖未实现模块。
