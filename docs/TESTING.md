# FRAZIL 测试、听测与发布门槛

> 目标：让“声音正确、实时安全、Host 可用、状态兼容”都由可重复证据支持，而不是只依赖编译成功或主观印象。

## Modification Policy

测试合同、测试层级、验收门槛和拒收条件属于 CONTROLLED 内容，修改必须有 issue/review，并同步 `CODING_PLAN.md` 或相关 ADR。实际运行结果属于 STATUS/EVIDENCE，只记录可复现的命令、环境和结果；计划中的测试不得写成已通过。

## 1. 测试分层

| 层级 | 位置 | 主要问题 | 适用条件 |
|---|---|---|---|
| L0 Build/Smoke | CMake/CTest/portability checker | repository 是否可配置、编译、启动且不含机器专属路径 | `src/**`、production test infrastructure、CMake/presets、dependency/bootstrap、CI workflow、build script 或 executable tooling 变更必需；纯 docs/template 可标记 N/A，除非改变可执行命令、preset 或 CI/build/test 行为 |
| L1 Unit | `tests/unit/` | mapping、mix、gain、smoother、history 边界是否精确 | 相关变更必需 |
| L2 DSP Property | `tests/dsp/` | 极值、随机输入、prepare/reset 下是否 finite/stable | DSP 变更必需 |
| L3 Render Regression | `tests/render/` + `testdata/` | canonical engineering input、algorithm probe、固定 seed/参数下声音输出是否可复现 | 声音/routing 变更必需 |
| L4 Integration | `tests/integration/` | Processor、参数、state、mode retention | Host/app 变更必需 |
| L5 Plugin Validation | pluginval/VST3 validator | 生命周期、总线、editor、automation fuzz | 插件变更必需 |
| L6 DAW Acceptance | 手工矩阵 | 真实枚举、录制、回放、保存、重开 | HOST work item、明确兼容性任务或 milestone/release gate |
| L7 Listening | review pack | 声音是否达到 Water/Ice 产品目标 | 声音变更必需 |
| L8 Performance | benchmark | callback 时间、CPU、allocation、memory | DSP/beta/release |

Validation is impact-based：只有变更可能影响某层验证目标时，该层才是 required。纯 documentation、ownership、
planning、Issue/PR template、typo 或 non-executable governance 变更，不触发无关 Windows build、CTest、ASAN、
DSP property、pluginval、render、DAW、listening 或 performance validation。相关但未执行的层级必须以
`N/A` / `NOT RUN` 记录简短理由；这不降低 code、DSP、plugin、Host 或 release 变更原有的适用 gate。

### 1.1 Evidence ownership 与验收交接

- Engineering Lead 是 L0-L5 和 L8 工程 harness/measurement 的默认 Implementation DRI；Sound & Host
  Lead 提供 representative workload、风险场景和产品可理解性 review。
- Sound & Host Lead 是 L6 DAW acceptance 和 L7 listening evidence 的默认 Implementation DRI；Engineering
  Lead 检查环境、步骤、产物和结论是否可复现，并修复由 finding 交回的 production 问题。
- Acceptance DRI 默认只 read、run、reproduce、review 和 create finding，不接管对方 owner 的 substantial
  production implementation。当前 scope 内的 typo、小型 test/docs 或 trivial integration correction 不触发
  DRI Transfer；只有 substantial implementation responsibility 确实换人时才记录 transfer。
- M1 Exit Gate 必须同时具备 Engineering Evidence 与 Sound / Host Evidence。自动测试不替代真实 DAW
  acceptance，DAW 中“听起来正常”也不替代 finite/property/state/performance evidence。

具体 work-item DRI、路径边界与 handoff 见 [`COLLABORATION_ROLES.md`](COLLABORATION_ROLES.md)。

### 1.2 L6/L7 work-item mapping

`L6 DAW Acceptance` 归属 `HOST-001`：负责 scan/load、parameter enumeration、automation、
save/reopen、DAW render 和 Host compatibility evidence。

`L7 Listening` 归属 `LISTENING-001` 与 `WATER-006` / `ICE-006`：负责 representative musical
material、license/provenance、perceptual review suitability 和 Water/Ice product-sound evidence。
`LISTENING-001` 可以在 M1 期间准备，但不是 M1 Exit Gate；它必须在 `EXP-W-003` / `EXP-I-003`
以及 `WATER-006` / `ICE-006` 前 ready。`EXP-W-002` / `EXP-I-002` engineering experiments
可以直接使用 `TESTDATA-001`，不需要等待 listening corpus 完成。

Listening WAV 可以被放入 DAW 播放或作为 DAW 测试输入，但这不会把 `LISTENING-001` 变成
DAW compatibility evidence 的 owner；该证据链仍归 `HOST-001`。

## 2. 自动化测试必须覆盖

### ParameterLayout

- 参数数量、顺序、精确 ID、类型、范围、step、默认值和 choice index；
- 全部核心参数始终存在，不受 routing 影响；
- 目标 ID 为 `water.enabled`/`ice.enabled`；旧占位 ID 的处理有显式测试；
- 参数值格式（dB、百分比、choice label）稳定。

### ParameterMapper / Snapshot

- 原子值到 Snapshot 在一个 block 内一致；
- choice index 安全映射到 `RoutingMode`，越界值使用安全 fallback；
- 所有连续值 clamp；dB 转 linear 的参考点：-24、0、+24 dB；
- mode-dependent 参数只改变“是否参与 DSP”，不破坏保存值；
- v1 Host automation 不承诺 sample-accurate：每个 process block 建立一次 coherent snapshot，连续参数由 DSP 内 sample-aware smoother 处理，离散参数走显式 transition；
- 不分配、不抛异常。

### Gain / mix / routing

- `input.gain=0 dB`、`output.gain=0 dB` 为 unity；
- `global.mix=0` 返回 post-input dry reference，`1` 返回完整 wet；
- routing/mixing infrastructure 不得引入额外未声明 processing latency；v1 Host-reported processing latency 为 0 samples；Water/Ice intentional effect delay/tail 可以存在并由算法 ADR/tests 描述；
- Parallel 端点与中心符合当前 crossfade law；
- 两个 serial 顺序的 0/0、100/0、0/100、100/100；
- enable 四组合；
- routing/enable/amount/gain 快速变化无非有限样本和异常峰值；
- mono/stereo 结果和通道独立性正确。

### Inactive-control routing invariants

这些是 routing invariant regression tests，不是 UI 显隐测试：

Parallel：

- Water only 时，`parallel.balance = 0 / 0.5 / 1` 不得改变 Water-only 输出；
- Ice only 时，`parallel.balance = 0 / 0.5 / 1` 不得改变 Ice-only 输出；
- 两个模块 disabled 时，任意 `parallel.balance` 不得改变 pass-through 结果；
- 测试必须防止用未经 enable gating 的 `(1 - balance) * water` 逻辑错误衰减 active branch。

Serial：

- Water disabled 时，`water.amount = 0 / 0.5 / 1` 不得改变 Water stage 的 pass-through 行为；
- Ice disabled 时，`ice.amount = 0 / 0.5 / 1` 不得改变 Ice stage 的 pass-through 行为；
- inactive amount 仍保留 Host/state 值，但不得影响当前关闭的 stage 输出。

### Processor property

对所有支持的 sample rate/block size、参数极值及固定 seed 随机输入：

```text
finite input -> finite output
silence -> no unexplained DC/NaN/Inf
prepare -> process -> reset -> prepare 可重复
zero/short/maximum supported block 不越界
```

若算法有合理 tail，测试 tail reporting 与衰减；无 tail 时验证清零/旁路行为。

### RandomSource / DSP-005

M1 建立 generic `RandomSource` 的 fixed-seed、explicit injection、reseed、instance isolation、deterministic sequence、realtime-safe 和无 audio-thread allocation 合同。M2/M3 只在各自算法 ADR/测试中定义 production seed source、save/reopen、offline render、实时播放和 routing transition 的 algorithm-specific random semantics。

### State / History

- 当前 `frazil_unit` 覆盖 STATE-001 的 schemaVersion=1 value round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、默认/非法输入 fallback（含 duplicate known ID、nonnumeric schemaVersion/value 和 malformed bool）、已知 pre-v1 schema、`water.enable`/`ice.enable` migration、全部 9 个参数、三个 routing choice 和 inactive-mode value retention；frazil_plugin_integration 额外覆盖 prepareToPlay -> setStateInformation -> processBlock 生命周期 restore；真实 Host/DAW restore 与 M5 EditHistoryManager acceptance 仍待执行。
- 默认 state、全参数极值、三种 routing 的 round-trip；
- inactive-mode 参数值跨切换和保存恢复保持；
- 损坏/空/未知 schema 不崩溃；
- Host restore 清空 Undo/Redo；
- 一次 drag 只生成一个 transaction；离散点击一项一步；
- automation callback 不进入 history；新编辑清空 redo branch；history 有容量上限。
- 测试 fixed seed 与 production instance seed 分离；验证多实例不因测试 seed 而被迫同步。

## 3. Render regression 协议

每个 case 固定：

- canonical engineering WAV 或明确生成的 algorithm probe、内容 hash/生成参数与许可证来源；
- sample rate、channel count、block size；
- 完整参数 JSON/文本 fixture；
- routing 和 enable 状态；
- test random seed，以及 production instance seed 的来源/隔离策略；
- commit SHA、构建类型、算法版本；
- 输出 peak、RMS/LUFS、DC、NaN/Inf、长度和文件 hash。

至少保留以下 routing cases：

```text
Parallel: Water endpoint / center / Ice endpoint / each branch disabled / both disabled
Water->Ice: 0/0, 100/0, 0/100, 100/100
Ice->Water: 0/0, 100/0, 0/100, 100/100
Global: mix 0/50/100, gain min/unity/max, rapid automation fixture
```

不要用严格逐样本相等替代声音判断。确定性算法可比较 hash；浮点/平台差异使用明确的 abs/relative tolerance、能量/频谱特征和人工听测。更新 reference 必须在 PR 中说明原因，禁止为“让 CI 绿”无解释覆盖。

当前 RENDER-001 smoke 由 `frazil_render` 与 `tools/render_testdata.py` 提供：前者在
非实时命令行进程中读取 WAV，以固定 block size 调用当前 `AudioEngine`，拒绝非有限输出并写出
WAV；后者使用同一 input/config/seed 连续运行两次，要求输出逐字节一致，并写入包含输入/输出
metadata、完整当前 EngineParameters 配置、seed、输入 hash 和输出 hash 的 manifest。CTest 在构建
target 后运行该 smoke，并将产物隔离到当前 preset 的 ignored build tree（例如
`build/windows-debug/rendered/`）；手工运行的默认产物仍写入 ignored `testdata/rendered/`，不依赖
音频设备或 DAW。`frazil_render_cli` 还回归检查 `--help` 成功返回以及 enable、routing、balance
和 amount 的非法值拒绝。当前证据是 M1 pass-through engine 的 deterministic offline smoke，seed
仅作为测试元数据记录，因为当前 AudioEngine 不含随机 DSP；同一 CLI 回归还用一组非默认完整
配置检查 manifest 字段逐项保留请求值，但不据此宣称 Water/Ice/Routing DSP 已实现；不等同于
Water/Ice render matrix、听测或 DAW acceptance。该 CLI 回归还验证同一 output path 连续 render
时输出文件被覆盖而不是追加第二个 RIFF/WAV。

## 4. Listening Review

### TESTDATA-001 engineering diagnostic corpus

`TESTDATA-001` 现在正式定义为 **Engineering Diagnostic Corpus**。它只承担客观
property、render、algorithm 和 measurement 输入；它不模拟真实乐器，也不替代听测素材。
提交到 `testdata/input/` 的固定集合是以下十个以 DSP 目标命名的 48 kHz、stereo、PCM24
signals：

| Diagnostic signal | Primary purpose | Required semantic evidence |
|---|---|---|
| `zero_input__silence.wav` | zero-input stability、DC、finite output、tail | every sample exactly zero |
| `zero_state_response__impulse.wav` | zero-state/latency/modal/tail response | declared pre-silence、single -6 dBFS impulse、post-silence |
| `frequency_response__log_sweep.wav` | frequency response and spectral shaping | upward log sweep, start/end frequency regions, finite output |
| `harmonic_response__stepped_sine_1khz.wav` | level-dependent harmonic response | 1 kHz windows at -36/-24/-12/-6 dBFS and exact gaps |
| `intermodulation_response__two_tone.wav` | nonlinear mixing and IMD | 997/1499 Hz components, level, active window |
| `broadband_response__white_noise.wav` | broadband energy/spectral response | deterministic per-signal seed, near-zero DC, RMS, no LFO |
| `envelope_response__gated_sine.wav` | attack/release and threshold behavior | gate boundaries, exact silence, frequency and levels |
| `transient_response__pitch_decay.wav` | transient detection and pitch-decay preservation | event positions/levels, decreasing frequency, decaying amplitude |
| `aliasing_response__high_frequency_sine.wav` | high-frequency/aliasing response | frequency equals `0.22 * sampleRate` at 44.1/48/96 kHz |
| `stereo_isolation__channel_probe.wav` | channel isolation and crossfeed | inactive channel exactly zero in both windows |

The delayed impulse has approximately 250 ms pre-silence and at least 3 s of
post-silence. The pitch-decay signal replaces the old drum-oriented concept;
it uses phase accumulation for `f(t) = f_end + (f_start - f_end) exp(-t/tau_f)`
and `A(t) = A0 exp(-t/tau_a)`, so its frequency trajectory is measurable rather
than a synthetic instrument approximation. Parameter timelines, smoother
retargets, routing transitions, block-size variation, prepare/reset, and extreme
matrices remain unit/property/integration concerns and are not encoded as WAVs.

Manifest schemaVersion 2 records `testObjective`, `signalClass`,
`signalParameters`, `expectedProperties`, `analysisMethods`,
`analysisWindows`, and `targetTests`, in addition to provenance, license,
redistribution, WAV metadata, storage policy, SHA-256, and the existing
`role`/`signalType`/`purpose`/`definition`/`generationParameters`/`expectedUses`/
`analysisHints`/per-signal `channelRelation` fields. The generator uses
stable per-signal seeds derived from `BASE_SEED + signal ID`; adding or reordering
an unrelated signal cannot alter an existing randomized fixture. The committed
sample rate is 48 kHz; the same generator validates temporary 44.1 kHz and
96 kHz corpora.

`tools/test_testdata.py` regenerates the complete corpus in a temporary
directory, compares manifest semantics, WAV bytes and SHA-256, and runs the
standard-library semantic checks above. `tools/verify_testdata.py` is limited to
metadata, contract, provenance, file-integrity, PCM24, and manifest/input set
checks. `tools/signal_generators.py` remains an in-memory Layer C probe utility;
it does not create a parallel canonical corpus or render harness.

### Diagnostic and listening responsibilities

`testdata/input/**` is for objective engineering evidence. The separate
`testdata/listening/**` boundary is for the future **Representative Listening
Corpus** (`LISTENING-001`): licensed musical material, Water/Ice musical
usefulness, loudness-matched A/B review, and product-sound listening evidence.
DAW compatibility and Host acceptance belong to `HOST-001`. No real listening
material is introduced by this TESTDATA follow-up. Water/Ice engineering measurement
must use the diagnostic signals above; product sound acceptance must not rely
on impulse, sweep, or synthetic diagnostic tones alone.

### 四层测试分工

| 层 | 核心问题 |
|---|---|
| Canonical engineering signal | DSP 是否基本正确？ |
| Generated algorithm probe | 当前候选算法是否按照数学设计工作？ |
| Unit/property fixture | gain、mapping、smoothing、routing、event boundary 等精确数学是否正确？ |
| Listening corpus | 产品声音是否有价值？ |

Layer A 的 gain mapping、dB 转换、smoother ramp/retarget、routing endpoint、enable gating、
RandomSource、block partition 和 controlled Water/Ice stub 输出应优先由 unit/property fixture
验证，而不是不断增加 WAV。Layer D 的真实音乐素材未来放入独立的 `testdata/listening/`，不得
混入 TESTDATA-001 或被当作 byte-exact canonical reference。

### Review pack

每个核心声音 PR 提供：

```text
00-dry.wav
01-baseline.wav
02-candidate-a.wav
03-candidate-b.wav（如有）
manifest.json（参数、seed、build、响度数据）
LISTENING_NOTES.md
```

候选尽量 loudness-match。至少记录：材质辨识度、输入可辨识度、动态保留、刺耳/浑浊、瞬态、立体声、噪声/DC、极端参数和偏好结论。重要声音方向由两名开发者共同 review。

### Listening rubric

所有 Water candidate 使用同一组 1–5 维度：

- Water Identity；
- Input Recognizability；
- Motion / Fluidity；
- Musical Usefulness；
- Artifact Severity（1–5，越低越好）。

所有 Ice candidate 使用同一组 1–5 维度：

- Crystal Identity；
- Friction / Fracture Identity；
- Input Recognizability；
- Transient Quality；
- Musical Usefulness；
- Artifact Severity（1–5，越低越好）。

Rubric 不规定总分公式。评审必须保留每个维度的分数、简短理由和两位评审的独立结论，再记录 accepted、revise 或 reject；不得只用一个总分替代听感判断。

### Reject criteria

Water/Ice candidate 至少在以下任一情况发生时 reject 或退回 experiment：

- 出现不可控爆峰；
- 正常参数范围频繁产生严重 artifact；
- 效果主要变成额外 one-shot 拟音，而不是输入驱动的材质变化；
- 正常 amount 下输入主体不可辨识；
- 不同输入得到高度相似的独立拟音输出；
- 随机行为失控；
- 无法通过 automation stress；
- 相对 `PERF-BASE-001` baseline 出现未解释的严重 realtime regression；正式硬阈值由后续 `PERF-001`/Beta 阶段锁定；
- routing infrastructure 引入未声明 processing latency，或 plugin-reported latency 不是 0 samples；intentional Water/Ice delay/tail 本身不构成 latency violation。

## 5. 支持矩阵

基础自动矩阵：

| 维度 | 值 |
|---|---|
| Sample rate | 44.1, 48, 96 kHz |
| Block size | 32, 64, 128, 256, 512, 1024 |
| Channels | mono, stereo |
| Build | Debug, Release, MSVC ASAN |
| Routing | Parallel, Water -> Ice, Ice -> Water |

PR 可跑风险相关子集；nightly/beta 跑笛卡尔矩阵。每个新增 sample-rate-dependent 模块必须至少覆盖 44.1/48/96 kHz。

DAW matrix 由 `HOST-000` 在 M0 冻结并记录版本。它必须区分正式支持、开发验证和 best effort，并至少记录 OS、架构、插件格式、primary development DAW、primary validation DAW、secondary validation host 以及 Standalone 的角色。最低建议：

- 一个主要 Windows VST3 制作宿主；
- 一个轻量交叉验证宿主（如 REAPER）；
- JUCE Standalone 作为开发调试，不算 DAW 兼容性替代品。

## 6. Automation acceptance

v1 automation contract：FRAZIL 不承诺 sample-accurate Host automation。Host-visible parameter state 在每个 process block 开始建立一次 coherent snapshot；连续参数可在 DSP 内 sample-aware 平滑；离散参数使用显式、click-free transition。测试检查的是 block 边界一致性和最终值正确，而不是 Host 在 block 内的 sample-accurate 观察。

在 `HOST-000` 定义的 target DAW 对每个正式参数验证：枚举、lane、录制、编辑、回放、touch/gesture（适用时）、project save/reopen。重点场景：

- `parallel.balance`: 0 -> 1 -> 0 快速和慢速曲线；
- Serial: `water.amount 1 -> 0` 与 `ice.amount 0 -> 1` 独立、重叠、非对称；
- gains: -24 -> +24 dB，检查 click、zipper、峰值和恢复；
- routing/enable: 播放中切换，检查 click 和参数值保留；
- 模式切换后 inactive automation 继续写值，再切回时使用最新值。

所有 automation stress 至少检查：无 click、无 zipper、无 NaN/Inf、block size 改变后行为稳定、参数最终值正确；不得把“sample/block ramp”写成 Host sample-accurate 保证。

证据记录 DAW/版本、插件 SHA、音频设置、步骤、结果和问题链接。

## 7. Performance protocol

所有性能记录必须注明 Reference Machine、OS、compiler、compiler flags、build type、Reference DAW、sample rate、block size、测量工具、warm-up、采样窗口、线程/实例配置和统计方法。至少记录：

- mean callback time；
- P95 callback time；
- P99 callback time；
- worst observed callback time；
- callback deadline 与 deadline 使用率；
- CPU、峰值内存/常驻内存；
- allocation observation/count；
- denormal 行为。

### Milestone performance scope

| Milestone / work item | 允许测量的场景 | 目的 |
|---|---|---|
| M1 / `PERF-BASE-001` | pass-through AudioEngine、Parameter Snapshot/Mapper overhead、Input Gain、Global Mix、Output Gain、automation/smoothing baseline | 建立 48 kHz/128 samples 的 baseline；此时不得依赖尚未实现的 Water/Ice/Routing/UI |
| M2 / `WATER-005` | Water only | 相对 M1 baseline 记录 Water vertical slice 的增量与 mean/P95/P99/worst |
| M3 / `ICE-005` | Ice only | 相对 M1 baseline 记录 Ice vertical slice 的增量与 mean/P95/P99/worst |
| M4 | Parallel、Water -> Ice、Ice -> Water、routing transition | 相对 baseline 记录完整 routing 的增量和 transition 峰值；不得出现未解释的严重 realtime regression |
| M5 | editor closed、editor idle、editor animated | 记录 UI 生命周期/动画对 callback 的影响，不将 UI 状态混入 DSP baseline |
| M6 / `PERF-001` | full matrix、multi-instance、long-run、最终 release 场景 | 基于累积实测锁定 formal performance budget / Beta threshold |

`PERF-BASE-001` 只建立 baseline，不定义正式性能预算。M2/M3/M4 进行 regression tracking；M6 `PERF-001` 才能锁定正式阈值。在有测量前不得编造固定 CPU 百分比目标。

## 8. 验证命令

本机基础：configure preset 会为 configure 阶段的 JUCE nested build 注入 CMAKE_BUILD_PARALLEL_LEVEL=6；build 必须通过 build_safe wrapper。Debug、Release、ASAN 和其他重型 configure/build/test pipeline 必须串行执行，不得并发启动。

仓库 portability：

    python tools/check_portability.py
    python tools/test_check_portability.py
    python tools/test_build_safe.py


```powershell
cmake --preset windows-debug
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug

cmake --preset windows-release
python tools/build_safe.py --preset windows-release
ctest --preset windows-release

cmake --preset windows-asan
python tools/build_safe.py --preset windows-asan
ctest --preset windows-asan
```

pluginval 路径与完整 MSVC 环境初始化见 `docs/ENVIRONMENT.md`。CI 命令应使用 portable preset，不能依赖本机绝对路径。

## 9. Milestone gates

- M0：fresh clone 可配置/构建/测试，CI PASS，模板和 branch protection 就绪。
- M1：`HOST-000` target DAW matrix、`TESTDATA-001` manifest、`PERF-BASE-001` report、`AUTO-001` automation contract、`ARCH-LAT-001` 的 0-sample Host reporting 与 latency policy、参数枚举/state/automation smoke、gain skeleton、finite output、offline render、pluginval PASS。
- M2/M3：各自 vertical slice 的 property/render/listening rubric/Reject Criteria/pluginval PASS，并引用同一 reference corpus 与 performance baseline。
- M4：`PARAM-FREEZE-001` 已完成，`ADR-R-001` Accepted，`ROUTE-011` 的 routing infrastructure latency 检查通过；inactive-control routing invariants、完整 routing matrix、mode retention、click-free automation、loudness A/B PASS。
- M5：UI attachment、gesture/history、resize/accessibility 基线 PASS。
- M6：全矩阵、ASAN、长稳、多实例、DAW、CPU/memory、listening regression PASS。
- M7：Release clean build、VST3 validation、兼容性和 packaging 签核，known blockers=0。
