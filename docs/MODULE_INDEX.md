# FRAZIL Module Index

本索引是模块边界、公共接口、依赖、线程和证据的快速入口。`Planned` 只表示计划合同，不表示当前源码已经存在；实现前必须先关联对应的 [Coding Plan](CODING_PLAN.md) work item。

ProcessSpec 当前实际位于 `src/app/ProcessSpec.h`，由 M1 AudioEngine 消费。未来生产 Water/Ice/Routing 使用
共享 prepare contract 前，必须先将一个 canonical type re-home 到下游 DSP/common 值层（推荐未来
`src/dsp/ProcessSpec.h`）或落实明确 reviewed 的等价方案；不得 include app header 或保留同字段镜像/模块副本。
下表 planned ProcessSpec 指未来下游类型；当前 app public interface/path 不因本说明改变。

| Module | Path | Responsibility | Public Interface | Depends On | Thread | Tests | ADR | Plan / Work Item | Status |
|---|---|---|---|---|---|---|---|---|---|
| PluginProcessor | `src/plugin/PluginProcessor.*` | JUCE lifecycle、bus、Host 参数/状态适配、把 block 交给 app；Debug/ASAN developer diagnostics 与 temporary comparison boundary | `prepareToPlay`、`processBlock`、state API、Debug/ASAN developer snapshot/override consumer API | JUCE、app、bounded diagnostics、developer-only state | message/audio boundary | `tests/`、CTest、pluginval、plugin integration | 0002, 0003 | `HOST-000`, `HOST-001`, `PARAM-001`, `STATE-001`, `STATE-002`, `AUTO-001`, `DEV-UI-001` | M1 foundation and HOST-001 are complete; Live/FL are `Development Validated` for the recorded Windows x64 Debug VST3 scope; no official-support or future-routing claim; DEV-UI-001 usability acceptance and M5 EditHistoryManager (HIST-001..004) remain planned |
| ParameterLayout | `src/plugin/ParameterLayout.*` | Host/JUCE-facing 静态 ID、类型、范围、默认值、label、choice、smoothing contract | layout factory/definitions | JUCE parameter types | message/setup | parameter regression | 0002 | `PARAM-001` | Implemented M1 |
| ParameterSnapshot | `src/app/ParameterSnapshot.*` | 每 block 一致的 POD Host 值 | `capture`、snapshot value type | value-type/atomic parameter source、app value types | audio read boundary | snapshot consistency/unit | 0002, 0003 | `PARAM-002` | Implemented M1 |
| ParameterMapper | `src/app/ParameterMapper.*` | Host/application values 的 finite fallback、clamp、enum、dB -> linear；未来只准备 normalized WaterProductValues，不拥有其类型定义或 Water DSP target mapping | `map`、mapping value types | ParameterSnapshot、app value types；planned Water-domain WaterProductValues / WaterModel | audio-safe | table-driven unit | 0002 | `PARAM-003` | Implemented M1; Water extension planned |
| StateModel | `src/app/StateModel.*` | versioned plugin state、Host restore 与内部 history 边界 | `Values`、`SerializedState`、`serialize`、`deserialize` | app value types only | message/setup | round-trip/migration/fallback/integration | 0002 | `STATE-001`, `STATE-002` | Implemented M1-C foundation; STATE-002 mode-value-retention integration verified; EditHistoryManager remains planned for M5 (HIST-001..004) |
| AudioEngine | `src/app/AudioEngine.*` | 处理链生命周期与 gain/global mix 编排；wet path 当前为 pass-through；维护 ProcessSpec buffer invariant | `prepare`、`reset`、`process` | JUCE audio buffer、current app ProcessSpec、DSP primitives；planned shared DSP/common ProcessSpec migration before downstream consumption | audio | `frazil_tests`, `frazil_render`, `frazil_render_cli` | 0003 | `ARCH-001`, `APP-001`, `DSP-001..004` | Implemented M1 skeleton with priming/invariant guard and offline render smoke |
| WaterProcessor | planned `src/dsp/water/` | 双模式、input-driven、source-preserving Water material transform；Fluid=A+B+D，Resonant=C；不拥有 routing/stage/global mix | planned prepare/reset/process + Water-domain WaterProductValues；同域 pure-value WaterMacroMapper 唯一映射 Model/Size/Motion/Decay -> FluidTargets / ResonantTargets -> DSP components | Water-domain values / DSP primitives、planned DSP/common ProcessSpec；no src/app, PluginProcessor, APVTS or UI；mapper independent of DSP state | audio | property/render/listening/perf | Proposed 0006 (`ADR-W-001`) | `WATER-001..008` | Planned M2; no production Water DSP or production Host controls registered; [SPIKE-W-DSP-001 objective A/B/D/C feasibility](../experiments/water/SPIKE-W-DSP-001/README.md) exist; no production adoption |
| IceProcessor | planned `src/dsp/ice/` | Ice material transform | prepare/reset/process + Ice params | DSP primitives、planned DSP/common ProcessSpec；no src/app | audio | property/render/listening/perf | future Ice ADR | `ICE-001..007` | Planned M3 |
| RoutingEngine | planned `src/dsp/routing/` | Parallel、Water -> Ice、Ice -> Water topology与transition | routing process/value types | stage interfaces、planned DSP/common ProcessSpec；no src/app or algorithm internals | audio | routing matrix/click-free | 0001, 0003, ADR-R-001 | `ROUTE-001..011` | Planned M4 |
| StageMixer | planned `src/dsp/` | Serial stage dry/processed mix law | mix function/value types | no Host/UI | audio | endpoint/monotonicity | 0001 | `ROUTE-002` | Planned M4 |
| DryWetMixer | `src/dsp/` | Global dry/wet mix law | `mix` | no Host/UI | audio | endpoint/finite/monotonicity | 0001, 0003 | `DSP-002`, `ROUTE-008` | Implemented M1 primitive |
| RandomSource | `src/dsp/primitives/RandomSource.*` | M1 generic、可注入、可复现的 DSP 随机源；M2/M3 使用并补 algorithm-specific semantics | seed/next/reseed value API | standard library only | audio | fixed-seed determinism、instance isolation | future algorithm ADRs only when semantics become production decisions | `DSP-005` | Implemented M1 primitive |
| Smoothing primitives | `src/dsp/primitives/` | 连续参数 smoothing 和离散 transition primitives；重复 target 不重启 ramp，变化 target 可 retarget | `prepare`/`reset`/`setTarget`/`getNextValue` | standard library/value types | audio | step/transition/finite | 0003 | `DSP-004`, `AUTO-001` | Implemented M1 continuous primitive; discrete transition remains M4 |
| EditHistoryManager | planned `src/app/EditHistoryManager.*` | 有界 UI transaction undo/redo；隔离 Host automation/restore | transaction/undo/redo/clear | app value types only | message only | capacity/source/clear | 0002 | `HIST-001..004` | Planned M5 |
| PluginEditor | `src/plugin/PluginEditor.*` | Debug/ASAN 的 DEV-UI-001 Developer Control Surface 与 Release 静态占位 editor；JUCE lifecycle | editor component；effective-state ownership、Return Host、non-APVTS temporary A/B/Reset、Dry/Processed actions、draft experiment-config actions | JUCE、PluginProcessor interface、diagnostics presentation | message/UI | lifecycle/resize/pluginval；workflow acceptance remains pending | — | `UI-001`, `DEV-UI-001` | Debug/ASAN implementation is in `main`; not Production UI; Live/FL HOST-001 developer/Host boundary case is accepted within M1; full DEV-UI workflow usability acceptance remains pending |
| Developer Control Surface | `src/plugin/PluginEditor.*` + `src/plugin/DeveloperDiagnostics.h` + developer state headers; GUI candidate in `src/ui/Developer*` | realtime sound exploration、current-parameter control、bounded coherent prepared/latest diagnostics、temporary A/B/Reset、Dry/Processed comparison 和 experiment-config export；不承担 Host acceptance 或 Production UI | narrow parameter interface + bounded diagnostics + developer-only comparison boundary | plugin parameter interface；non-realtime diagnostics consumer | message/UI；audio thread only publishes bounded data and reads atomic override/mode | Debug/Release/ASAN build isolation and CTest compile/runtime evidence；interactive usability/config tests pending | — | `DEV-UI-001` | Implementation is in `main` and engineering-ready for final workflow usability acceptance; not Production UI; final workflow acceptance remains pending |
| Developer diagnostics presentation | `src/ui/DeveloperDiagnosticsView.*`, `src/ui/DeveloperLevelMeter.*` | compact runtime/finite text and aggregate INPUT/OUTPUT dBFS meters; no measurement or state ownership | `update(snapshot, routing)`; `setLevels(peak, rms)` | JUCE; existing diagnostics value type only in the view; no Processor/APVTS/engine access | message only, editor-owned | Debug/Release/ASAN isolation, CTest, pluginval and local size/signal observations; see Project Status 2.9 | 0003 (unchanged) | `DEV-UI-001` | Implementation candidate; human usability acceptance pending; not Production UI |
| Production UI components | planned `src/ui/` | 产品参数表达、attachment、gesture 和 UI transaction | narrow plugin parameter interface、narrow app edit/history command interface | plugin parameter interface、app edit/history command interface | message only | interaction/resize/automation | 0002 | `UI-001..008`, `HIST-002..004` | Planned M5 |

The standalone Water engineering preview lives in
`experiments/water/SPIKE-W-DSP-001/preview/`: `PreviewSettings` owns application values/export,
`PreviewEngine` owns existing research DSP selection/lifecycle, `PreviewController` owns source/device
and audio callback state. `ResearchSessionModel` owns draft/applied experiment, engineering, provenance
and A/B values; `PreviewPanel` coordinates commands and validation, `ResearchViews` renders two views,
and `PreviewMain` owns application/window setup. It reuses the value-only diagnostic
view/meter. Dependencies point from research application to existing DSP/UI primitives; FRAZIL targets
do not depend on the preview. Tests and limitations: [validation](evidence/WATER_PREVIEW_VALIDATION.md).

The control-bridge integration adds isolated `preview/TimeValue.h` UI/tooling helpers for adaptive
ms/s display and strict exact entry. They own no session or DSP state, use only the C++ standard
library, and are tested by `frazil_water_preview`; all research time widgets consume these helpers.
See [staged execution](evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md).
`ControlDescriptor.h` supplies typed IDs, module groups, units/display policy, baseline provenance,
range and lifecycle metadata for the 23 research controls (21 original plus two Modal Motion fields). `PreviewSettings` consumes the descriptors
without changing module JSON; DSP continues to use its existing typed config structs.
`SessionCodec.h` owns the separate versioned research manifest including provisional Decay;
`SessionJsonSyntax.h` bounds and validates its richer JSON syntax before schema decoding. Imports
produce candidates and use the controller's existing DSP validation before restoring the model.
`SessionMetadata.h` separates portable source/build context from audio bytes; generated
`PreviewBuildInfo.h` records configure-time Git/build provenance for both preview and its tests.
Module imports reuse renderer parsing/defaults and the existing prepare validators.
`ProtectControls.h` adapts typed research config and separate calibration/enable memory;
`ProtectView.h` owns its research presentation. `ExactValueControl.h` validates text before slider
clamping and serves all engineering/Protect values, with fine gestures and baseline reset.
`DraftSummary.h` formats applied-to-draft differences without owning parameter state.
`PreviewEngine` reuses `ResidualProtect`/`applyFluidProtect`,
while `PreviewController` transports only the live Depth target through a lock-free atomic value.
`ProtectDiagnostics.h` provides a fixed 256-entry SPSC block-summary queue from callback to the
message thread, with last-sample/peak values and explicit dropped-block accounting. It owns no
algorithm state; UI formatting stays in `ProtectView` and no production target depends on it.

`ResearchOperationHistory.h` owns a message-thread 50-entry runtime operation ring and a single
pending transaction; `ResearchOperations` borrows the session and coordinates gesture/debounce
boundaries through injected lifecycle callbacks. `ResearchSlider.h` exposes physical mouse
boundaries independently of JUCE's wheel/key drag notifications. These are research diagnostics,
not M5 undo/redo or Host history; neither is serialized or accessed by DSP.

## Proposed Protect research

[DOC-W-PROTECT-001](planning/WATER_PROTECT_CANDIDATE_REVISION.md) records the theory. The user-authorized
[PROTECT-EXP-001](planning/WATER_PROTECT_EXECUTION.md) adds `ProtectDetector`, `ResidualProtect` and pure
`applyFluidProtect` in the existing opt-in research tree. They own linked detection/gain state and residual
composition, with prepare/reset/sample processing and research unit/CLI tests; no production target depends
on them. Offline listening preparation separates fixed-source primary and RMS-matched preference evidence,
with explicit D0/D1 conditions and a real-renderer CTest regression. See the
[research README](../experiments/water/SPIKE-W-DSP-001/README.md#protect-follow-up--protect-exp-001).
Production Water-domain ownership/adoption gates remain; no fifth accepted macro or production module is created.

## Experiment support

`experiments/water/reference_intake.py` is the bounded EXP-W-001 offline reference-intake CLI.
It reads 1–6 explicit IDs from `REFERENCE_INDEX.csv`, resolves audio inside a caller-supplied external
library, and reuses `tools/analyze_testdata.py` for development-only analysis/plots. JSON and plots stay
under ignored `testdata/rendered/`; it does not copy audio, infer listening labels or enter production targets.
See [reference usage](../experiments/water/REFERENCE_INDEX.md) and
[acceptance/validation record](../experiments/water/task_plan.md). Its runtime is Python plus
`requirements-dsp.txt`, on the offline caller thread; it has no plugin/app/DSP dependency.

## Registration and update rules

- 新模块进入本表前必须有真实需求、路径、公共接口、依赖方向、线程模型、测试入口和 Coding Plan ID；人员 owner 由 GitHub Issue/Project 维护，不写死在长期索引中。
- 文件移动、公共接口、线程模型、参数/state/routing 边界或 ADR 变化时，同一 PR 更新本表和对应 module README。
- 当前不存在的 planned module 不得被 include、CMake target 或生产代码提前依赖；若计划与源码冲突，优先记录冲突并停止扩大范围。

## Modification Policy

本索引属于 LEVEL 3 MAINTAINED 文档，但其中的公共接口和依赖事实必须服从 Level 1/2 合同。修改需在相关代码/文档 PR 中同步，不能用索引文字掩盖未实现模块。

`ResearchWaterMacroMapper.h` owns plain, allocation-free research curves;
`ResearchMappingAdapter.h` owns the engineering destination table used by session commands.
Neither is a production mapper; see [mapping v0.1](../experiments/water/SPIKE-W-DSP-001/RESEARCH_MAPPING.md).

Research `LiquidModalResonator` additionally owns six normalized excitation weights and an instance
RNG in seed domain 4. Fixed coefficients remain prepare-only; preview/renderer share the optional
Motion fields with zero-depth legacy behavior. No production DSP dependency is introduced.

`ResearchListeningCalibration.h` owns four research-session starting gains independently of macro
mapping and typed renderer defaults. Session commands/codec track MAPPED/CUSTOM status; no DSP path added.

`AuditionMonitor.h` owns preview-only 10 ms carrier/E/output ramps after Protect. Controller
transfers validated linear trim atomically; model/session own dB context, excluded from DSP JSON.

`WaterDiagnostics.h` owns fixed numeric Water readouts/energy accumulation in the research preview.
Existing Protect block transport carries both readouts with one bounded producer/consumer; the UI
formats them in `WaterDiagnosticsText.h`. DSP only exposes scalar activity getters, with no UI dependency.

ResearchAuditionWorkflow coordinates message-thread stop/prepare/start once per completed Sound Lead gesture. ResearchPresentation formats actual targets and bounded operation history; neither owns DSP. PreviewController exposes detached prepare/start with explicit source-rate and callback-state guards.

Research config exporter `render/research_cases.cpp` reuses the UI mapper/adapter/calibration; `render/listening_handoff.py` orchestrates the existing renderer and decoded checks. Both are offline, opt-in research tools with no production dependency.
