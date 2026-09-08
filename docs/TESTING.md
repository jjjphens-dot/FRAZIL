# FRAZIL 测试、听测与发布门槛

> 目标：让“声音正确、实时安全、Host 可用、状态兼容”都由可重复证据支持，而不是只依赖编译成功或主观印象。

## Modification Policy

测试合同、测试层级、验收门槛和拒收条件属于 CONTROLLED 内容，修改必须有 issue/review，并同步 `CODING_PLAN.md` 或相关 ADR。实际运行结果属于 STATUS/EVIDENCE，只记录可复现的命令、环境和结果；计划中的测试不得写成已通过。

## 1. 测试分层

| 层级 | 位置 | 主要问题 | 每次 PR |
|---|---|---|---|
| L0 Build/Smoke | CMake/CTest/portability checker | repository 是否可配置、编译、启动且不含机器专属路径 | 必需 |
| L1 Unit | `tests/unit/` | mapping、mix、gain、smoother、history 边界是否精确 | 相关变更必需 |
| L2 DSP Property | `tests/dsp/` | 极值、随机输入、prepare/reset 下是否 finite/stable | DSP 变更必需 |
| L3 Render Regression | `tests/render/` + `testdata/` | 固定输入/seed/参数下声音输出是否可复现 | 声音/routing 变更必需 |
| L4 Integration | `tests/integration/` | Processor、参数、state、mode retention | Host/app 变更必需 |
| L5 Plugin Validation | pluginval/VST3 validator | 生命周期、总线、editor、automation fuzz | 插件变更必需 |
| L6 DAW Acceptance | 手工矩阵 | 真实枚举、录制、回放、保存、重开 | milestone/release gate |
| L7 Listening | review pack | 声音是否达到 Water/Ice 产品目标 | 声音变更必需 |
| L8 Performance | benchmark | callback 时间、CPU、allocation、memory | DSP/beta/release |

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

- 输入 WAV 的内容 hash 与许可证来源；
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

## 4. Listening Review

### 固定素材（TESTDATA-001）

`TESTDATA-001` 必须建立并维护同一套 Water/Ice 共用的 reference corpus：impulse、noise、drums、vocal、piano、guitar、pad、bass。每个素材的 machine-readable manifest 至少记录 id、filename、purpose、source、author、license、redistribution、content hash、sample rate、bit depth、channel count、duration（秒）、storage location，以及 repository/artifact/LFS 策略。未完成许可和 hash 审计的素材不能成为 regression reference。

当前仓库提供八个由 `tools/generate_testdata.py` 固定 seed 生成的双声道
`PCM_S16LE` 输入，并将小型合成 fixture 直接存放在 `testdata/input/`；manifest 明确标记为
generated synthetic reference corpus、无第三方音频，并记录每项的 provenance、redistribution、
WAV metadata 和 SHA-256。`tools/verify_testdata.py` 是 manifest review 的自动化入口；未来加入
外部音乐素材时，必须先替换或扩展 manifest，并重新完成许可审计，不得把未授权素材直接作为 reference。

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
