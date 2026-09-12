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

`L7 Listening` 归属 `LISTENING-001` 与 `WATER-006`，以及 Ice 恢复后的 `ICE-006`：负责 representative
musical material、license/provenance、perceptual review suitability 和 product-sound evidence。
`LISTENING-001` 可以在 M1 期间准备，但不是 M1 Exit Gate；当前必须在 `EXP-W-003` 和 `WATER-006`
前 ready。`EXP-W-002` engineering experiments 可以直接使用 `TESTDATA-001`，不需要等待 listening
corpus 完成。Ice 当前 DEFERRED；恢复时 `EXP-I-003` / `ICE-006` 仍遵守相同 listening prerequisite。

Listening WAV 可以被放入 DAW 播放或作为 DAW 测试输入，但这不会把 `LISTENING-001` 变成
DAW compatibility evidence 的 owner；该证据链仍归 `HOST-001`。

### 1.3 Host, interactive and offline entry points

| Entry point | Question answered | Evidence boundary |
|---|---|---|
| Host Test | Does the DAW <-> VST3 contract work? | `HOST-001` uses DAW enumeration, automation, state restore, save/reopen and render. Developer UI cannot substitute it. |
| Developer Interactive Testing | Can sound designers rapidly control, compare and diagnose the current experiment? | `DEV-UI-001` usability and bounded diagnostics. It is not deterministic proof or Host acceptance. |
| Offline Experiment Testing | Is a candidate/config reproducible and analyzable? | Fixed input/config/seed render, sweep, analysis and review pack. It cannot substitute human listening. |

The detailed boundary, diagnostics rules and realtime-to-offline config handoff are defined in
[`DEVELOPER_SOUND_TOOLS.md`](DEVELOPER_SOUND_TOOLS.md).

The follow-up feature-branch candidate for `DEV-UI-001` is available only in Debug/ASAN builds and is covered by
the corresponding compile/runtime CTest paths, including effective-state ownership while the developer override is
active, conditional token rejection of stale Editor edits after Host clear, same-session last-coherent fallback under
continuous set-only publication, clear/session cache invalidation, Host-change/state-restore transitions,
Dry/Processed path, bounded A/B slot round-trips and coherent latest-callback telemetry. Concurrent regressions
require reader participation and coherent observations; the exact Editor/attachment lifecycle is still not automated.
This is engineering evidence, not GUI automation. Interactive usability/listening acceptance and Offline Sound Lab
handoff remain acceptance work. Release builds intentionally retain the non-developer editor placeholder; the current
development baseline remains planned until the candidate is reviewed and accepted. The current candidate status is
Engineering Ready for Sound & Host Lead usability acceptance, not `Done`.

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

对工作项定义的 representative sample-rate / nominal-block matrix、参数极值及固定 seed
输入：

```text
finite input -> finite output
silence -> no unexplained DC/NaN/Inf
active/default prepare -> process -> release/reprepare -> process 可恢复
prepared nominal matrix 内的 zero/short/odd actual callbacks 不越界
```

`nominal/prepared block size` 与实际 callback 的 sample count 是两个不同维度；当前 `1024`
只是 representative nominal upper test value，不是 FRAZIL 已定义的 public maximum support
limit。active lifecycle 只验证 recovery、finite output 和 valid processing state；需要 byte-exact
repeatability 的 lifecycle 或 fresh-processor case 只适用于 ADR-0005 一致的 M1 neutral/dry
fixture，不冻结未来 Water/Ice production randomness。

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
review-pack/
  00-dry.wav
  01-baseline.wav
  02-candidate-a.wav
  03-candidate-b.wav
  manifest.json
  analysis/
    candidate-a.json
    candidate-b.json
  plots/
    waveform.png
    spectrum.png
    spectrogram.png
  LISTENING_REVIEW.md
```

该结构是 PLANNED target；现有 RENDER-001 smoke 不因此被描述为完整 Sound Lab/review-pack generator。
候选尽量 loudness-match。至少记录：材质辨识度、输入可辨识度、动态保留、刺耳/浑浊、瞬态、立体声、
噪声/DC、极端参数和偏好结论。重要声音方向由两名开发者共同 review。

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

每个 rubric 维度在进入对应 listening stage 时必须补充 1 / 3 / 5 anchors。例如 Water Identity：

- 1：基本不存在 Water material identity；
- 3：明显存在，但跨素材一致性有限；
- 5：Water identity 强且稳定，同时保持输入主体。

Objective features are **Objective Proxies**, not perceptual truth. Peak/RMS/DC/finite/crest factor and later
LUFS/true peak/spectral/onset/pitch/tail/stereo features may support a question, but no single metric or scalar
quality score may replace the rubric and human `ACCEPT / REVISE / REJECT` decision. See
[`PERCEPTUAL_CONTRACT.md`](PERCEPTUAL_CONTRACT.md).

### Water dual-mode planned acceptance

本节定义 M2/`ADR-W-001` 的计划证据，不表示 Fluid、Resonant 或候选参数已经实现。Ice 的算法、参数、
work item 和测试在本次 Water-focused revision 中保持不变。

#### Product/listening evidence (`LISTENING-001`, `EXP-W-003`, `WATER-006`)

同一 representative material、输入片段和 matched-loudness procedure 至少比较 dry、baseline、Fluid 和
Resonant，并分别记录：

- Fluid Water identity：bubble/liquid character、droplet/transient response、continuous irregular motion
  和 non-periodic behavior；
- Resonant Water identity：stable/cohesive liquid resonance、tonal/pitched-material compatibility、
  predictable musical response；
- Fluid 与 Resonant 分别按各自 mode-specific responsibilities 验收；qualitative review 只需确认二者都是
  intentional Water models 且不被理解为 real/fake、good/bad 或 quality switch。当前不要求
  perceptual-distance metric、classification threshold 或 mode-separation score；
- normal product settings 下的 input recognizability，包含代表性的 `global.mix=100%` Water-only case；
  这不是 sample-equality assertion，也不能只靠降低 Global Mix 通过；
- transient preservation、RMS/LUFS relationship、residual energy、peak growth、spectral change、DC 和
  tail behavior；实验前不设 universal residual-to-input dB threshold；
- **Semantic Predictability**：不解释内部 DSP 时，用户能预测 `water.size` 从 Fine/Small/Bright 到
  Large/Deep 表示尺度更大、更深，`water.motion` 从 Calm/Stable 到 Active/Flowing 表示时间行为更
  活跃、更流动；Size 的 compact display 可候选 `Fine <-> Deep`，exact label 不在本测试计划冻结；
- **Cross-Mode Consistency**：Fluid/Resonant mapping 可以不同，但切换后 Size/Motion 保持同一高层
  感知方向，且 Resonant 的 Motion 变化刻意比 Fluid subtle；
- **Orthogonality**：用户能区分 Size、Motion、`water.amount`、`global.mix` 和
  `parallel.balance`；Motion 不得主要表现为 Amount/output gain/simple loudness increase，Size 不得
  主要表现为 loudness，Mode 不得表现为 quality switch；任何 compensation 由测量和 loudness-matched
  review 决定，不能预填固定 dB；
- **Discoverability**：Fluid 与 Resonant 被理解为两种 Water behavior，而不是 real/fake、good/bad 或
  high/low quality；记录是否需要简短描述或 tooltip；
- **Interaction Cost**：评估常用 Water sound design 是否能用 Enable、Mode、Size、Motion 完成，而不
  暴露 bubble radius、resonator Q、delay depth、event probability 等 engineering controls；
- **Automation Readability**：Host lane 的 Water Model、Water Size、Water Motion 无需内部 DSP 知识即可
  理解；
- musical usefulness、artifact severity、最终 mapping 理由、risk 和 tradeoff。

#### Engineering/property evidence (`EXP-W-002`, `WATER-001..005/008`)

- 分别测量 Bubble Ensemble、Droplet/Impact、Flow Modulator 和 Liquid/Modal Resonator；Fluid A+B+D
  集成仍保留 A/B/D component ablation，C 作为 Resonant baseline/mode；
- 每个 sub-engine 明确输出 residual 或 complete processed signal；若 Resonant 已含 direct feedthrough，
  验证 higher-level composition 不重复 `x`，并检查 unintended gain/comb artifact；
- fixed test seed 下可重复；production instances 不因 mutable global/shared seed 同步；production seed、
  offline determinism、save/reopen persistence 和 transition progression 由 Water ADR 定义；
- finite input、silence、input/parameter extremes、prepare/reset/reprepare、short/odd/zero callbacks 下无
  NaN/Inf、out-of-range access 或 unexplained state corruption；
- 记录 DC、peak、tail decay/termination、denormal behavior、residual energy 和 bounded event/voice count；
- sample rates 至少覆盖 44.1/48/96 kHz；representative blocks 覆盖 32/64/128/256/512/1024 的风险相关
  子矩阵，并按算法语义验证 block-partition consistency；
- Size：Fluid 的 bubble scale -> resonance distribution 与 Resonant 的 root/mode-family scale 在适用区间
  保持预期单调方向；Size 不改变 event density、Motion speed、Amount 或 general gain；
- Motion：Fluid 的 event/Flow/stochastic destinations 与 Resonant 的 subtle drift/excitation destinations
  保持预期 temporal-activity 方向；检查 loudness correlation，防止 Motion 主要成为 gain；
- Water mode transition 检查 click/zipper、异常 peak、rapid repeated automation、最终 mode、tail/state
  ownership、random progression、reset/prepare/state restore 和 transition CPU upper bound；不把通用
  约 20 ms 测试 baseline 当作 final Water duration；
- `water.enabled` transition 继续单独验证 pass-through、macro value retention 和 click-free behavior；
- 相对 `PERF-BASE-001` 分别记录 Fluid、Resonant 和 transition 的 mean/P95/P99/worst；M2 只做增量
  regression tracking，不提前冻结最终 performance budget；
- plugin `getLatencySamples()` 仍为 0，Water 不引入需要 lookahead/FFT block latency/linear-phase/
  convolution/Host PDC 的 production dependency；intentional effect delay/tail 单独描述和测试。

#### Candidate Host parameter evidence (`WATER-003/007`)

`water.model`、`water.size`、`water.motion` 只有在正式采纳后才进入 ParameterLayout/Host/state tests。采纳
前必须定义并验证：稳定静态注册与 deterministic choice ordering、range/default、ParameterMapper 的
mode-specific mapping、smoothing/transition、automation lane/record/edit/playback、inactive value retention、
save/reopen、schema evolution/default/migration fixtures 和 compatibility fallback。当前
`schemaVersion=1` 与九参数 registry 不因本测试计划扩展。

不增加 `water.dryWet` 测试或参数。责任保持为：`water.amount`=Serial stage amount，
`parallel.balance`=Parallel proportion，`global.mix`=完整插件 dry/wet。内部 algorithmic LFO/random tests
不构成 public LFO/modulation-matrix contract；通用用户 LFO 和可选 `Motion Mod` 均为 deferred。

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

### 5.1 TEST-002 processor property harness

`frazil_processor_property` 复用真实 `FRAZILAudioProcessor -> ParameterSnapshot ->
ParameterMapper -> AudioEngine` 路径，不建立第二套 processor 或 DSP。它用数据驱动的
sample-rate/block-size/channel matrix 覆盖 44.1/48/96 kHz、32/64/128/256/512/1024 samples
和 mono/stereo；参数、输入和 lifecycle 维度使用 canonical 子矩阵，避免为每个高成本 JUCE
processor construction 重复完整 Cartesian product。当前 cases 必须覆盖：

- default/minimum/maximum/intermediate 参数、四种 enable combination 和三个 routing choice；
- silence、impulse、固定 seed deterministic noise、extreme but finite input；
- silence 输出的有限性、DC 与最大幅度约束；
- active/default prepare/process/release/reprepare/process recovery、repeated prepare、repeated
  release/prepare、zero-length block；
- separate M1 neutral/dry prepare/process/release/reprepare/process exact-repeatability case；
- 一次 nominal prepare 后的 0、1、7、31 和 1024 sample 实际 callback；
- buffer dimensions、finite output，以及只针对 M1 neutral/deterministic path 的
  fresh-processor repeatability。

该 harness 是可供后续 Water/Ice processor 复用的基础；它不代表 Water/Ice、Routing 或真实
Host/DAW 已完成。

基础自动矩阵：

| 维度 | 值 |
|---|---|
| Sample rate | 44.1, 48, 96 kHz |
| Representative nominal block values | 32, 64, 128, 256, 512, 1024；不是未经定义的 maximum support limit |
| Actual callback sizes after nominal 1024 prepare | 0, 1, 7, 31, 1024 |
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
- 若 M2 正式采纳 Water candidates：`water.model` 的 Fluid/Resonant rapid switching/final value、
  `water.size` 与 `water.motion` 的快慢曲线、跨模式 value retention 和 save/reopen；采纳前不把这些
  candidate 写成当前 Host acceptance 项。

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

M1 的 `frazil_performance` 是手动运行的 headless `AudioEngine` benchmark，不加入 CTest
通过/失败门槛。它固定 48 kHz、128 samples、stereo 和预分配 buffer，使用跨 block 保持 phase
的 deterministic 440 Hz reference oscillator，分别测量 steady-state 与
parameter-retarget/smoothing 场景；同一个 oscillator state 连续贯穿 warm-up 和 measured
section，每个场景 warm up 2000 blocks，再测量 20000 blocks。
报告必须同时记录 Reference Machine、OS、compiler、effective compiler flags、build type、
Reference DAW、measurement tool、thread/instance configuration、statistical method、
mean/P95/P99/worst、2.666 ms callback deadline、mean/worst deadline utilization、
`harness_process_cpu_percent`、Windows process working set、selected `AudioEngine::process` workload 的 measured-callback
allocation count、finite-output 结果和 denormal probe 结果。输出字段
`configured_commit`/`configured_source_state` 是 CMake configure 时读取的 Git provenance，
`runtime_commit`/`runtime_source_state` 是 benchmark 启动、正式 measurement 前读取的 Git
provenance；只有两次 commit 相同、两次 source state 都是 `clean` 时，
`formal_provenance_status=PASS`。dirty、unknown 或 commit mismatch 必须显式标记为
`NOT RUN`，但不使 benchmark 因 provenance mismatch 强制失败。allocation observation 只覆盖
该 `AudioEngine::process` workload，不是完整 `FRAZILAudioProcessor::processBlock` callback
allocation-free evidence。denormal 输出使用 `denormal_probe_status=OBSERVED` 和
`denormal_finite_output_status=PASS/FAIL`；后者只表示输出 finite，不表示 FTZ/DAZ 已验证。
`harness_process_cpu_percent` 是 benchmark 进程在整个 measurement window 内的 process-wide
observation，包含 reference generation、parameter setup、timing calls、result bookkeeping
和 finite-output scan；它不是 `AudioEngine::process` 单独的 CPU utilization metric，也不是
formal CPU budget。
measurement buffers/result storage 在 observation 开启前预分配，避免把 harness bookkeeping
误报成 audio-thread allocation。该 work item 只提供 baseline，不把结果转换成正式 CPU 百分比
门槛，也不替代 pluginval、DAW 或 listening evidence。

```powershell
cmake --fresh --preset windows-release
python tools/build_safe.py --preset windows-release
.\build\windows-release\frazil_performance.exe
```

### Milestone performance scope

| Milestone / work item | 允许测量的场景 | 目的 |
|---|---|---|
| M1 / `PERF-BASE-001` | pass-through AudioEngine、Parameter Snapshot/Mapper overhead、Input Gain、Global Mix、Output Gain、automation/smoothing baseline | 建立 48 kHz/128 samples 的 baseline；此时不得依赖尚未实现的 Water/Ice/Routing/UI |
| M2 / `WATER-005` | Fluid only、Resonant only、Water mode transition | 相对 M1 baseline 分别记录双模式 steady-state 与 transition 的增量、mean/P95/P99/worst；不在 M2 预设最终硬预算 |
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
- `DEV-UI-001`：Developer interactive controls/diagnostics/config export 的 usability gate；是大规模
  `EXP-W-002` 前的 Water M2 readiness prerequisite，不是 M1 Exit gate，且不替代 Host/offline evidence。
- M2：Fluid/Resonant 双模式分别满足各自 mode-specific responsibilities，完成 component ablation、
  property/render、qualitative non-quality-switch review、Size/Motion semantic consistency、normal setting 与
  代表性 `global.mix=100%` input recognizability、click-free mode
  transition、state/automation、performance increment、Water rubric/Reject Criteria 和 pluginval PASS；
  引用 `TESTDATA-001`、`LISTENING-001` 与 `PERF-BASE-001`，且 Host-reported processing latency 保持 0。
- M3：Ice vertical slice 的 property/render/listening rubric/Reject Criteria/pluginval PASS，并引用同一 reference corpus 与 performance baseline；本次 Water revision 不改变 Ice gate。
- M4：`PARAM-FREEZE-001` 已完成，`ADR-R-001` Accepted，`ROUTE-011` 的 routing infrastructure latency 检查通过；inactive-control routing invariants、完整 routing matrix、mode retention、click-free automation、loudness A/B PASS。
- M5：UI attachment、gesture/history、resize/accessibility 基线 PASS。
- M6：全矩阵、ASAN、长稳、多实例、DAW、CPU/memory、listening regression PASS。
- M7：Release clean build、VST3 validation、兼容性和 packaging 签核，known blockers=0。
