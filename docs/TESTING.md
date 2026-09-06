# FRAZIL 测试、听测与发布门槛

> 目标：让“声音正确、实时安全、Host 可用、状态兼容”都由可重复证据支持，而不是只依赖编译成功或主观印象。

## 1. 测试分层

| 层级 | 位置 | 主要问题 | 每次 PR |
|---|---|---|---|
| L0 Build/Smoke | CMake/CTest | target 能否配置、编译、启动 | 必需 |
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
- 不分配、不抛异常。

### Gain / mix / routing

- `input.gain=0 dB`、`output.gain=0 dB` 为 unity；
- `global.mix=0` 返回 post-input dry reference，`1` 返回完整 wet；
- Parallel 端点与中心符合当前 crossfade law；
- 两个 serial 顺序的 0/0、100/0、0/100、100/100；
- enable 四组合；
- routing/enable/amount/gain 快速变化无非有限样本和异常峰值；
- mono/stereo 结果和通道独立性正确。

### Processor property

对所有支持的 sample rate/block size、参数极值及固定 seed 随机输入：

```text
finite input -> finite output
silence -> no unexplained DC/NaN/Inf
prepare -> process -> reset -> prepare 可重复
zero/short/maximum supported block 不越界
```

若算法有合理 tail，测试 tail reporting 与衰减；无 tail 时验证清零/旁路行为。

### State / History

- 默认 state、全参数极值、三种 routing 的 round-trip；
- inactive-mode 参数值跨切换和保存恢复保持；
- 损坏/空/未知 schema 不崩溃；
- Host restore 清空 Undo/Redo；
- 一次 drag 只生成一个 transaction；离散点击一项一步；
- automation callback 不进入 history；新编辑清空 redo branch；history 有容量上限。

## 3. Render regression 协议

每个 case 固定：

- 输入 WAV 的内容 hash 与许可证来源；
- sample rate、channel count、block size；
- 完整参数 JSON/文本 fixture；
- routing 和 enable 状态；
- random seed；
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

### 固定素材

在版权允许且体积受控的前提下维护：drums、vocal、piano、guitar、pad、bass、noise、impulse。原始素材放 `testdata/input/`，来源和许可写入 manifest。

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

DAW matrix 在 M0 由团队选定并记录版本。最低建议：

- 一个主要 Windows VST3 制作宿主；
- 一个轻量交叉验证宿主（如 REAPER）；
- JUCE Standalone 作为开发调试，不算 DAW 兼容性替代品。

## 6. Automation acceptance

在目标 DAW 对每个正式参数验证：枚举、lane、录制、编辑、回放、touch/gesture（适用时）、project save/reopen。重点场景：

- `parallel.balance`: 0 -> 1 -> 0 快速和慢速曲线；
- Serial: `water.amount 1 -> 0` 与 `ice.amount 0 -> 1` 独立、重叠、非对称；
- gains: -24 -> +24 dB，检查 click、zipper、峰值和恢复；
- routing/enable: 播放中切换，检查 click 和参数值保留；
- 模式切换后 inactive automation 继续写值，再切回时使用最新值。

证据记录 DAW/版本、插件 SHA、音频设置、步骤、结果和问题链接。

## 7. Performance protocol

Beta 前固定 Reference Machine、Reference DAW 和 48 kHz/128 samples baseline。记录：平均与峰值 callback time、deadline 使用率、CPU、内存、allocation count、denormal 行为。

场景：Water only、Ice only、Parallel、两个 Serial、routing transition、automation stress、idle editor、animated editor、多实例。性能预算由 M1 baseline 测量后在 ADR 中锁定；在有测量前不得编造固定百分比目标。

## 8. 验证命令

本机基础：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug

cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release

cmake --preset windows-asan
cmake --build --preset windows-asan
ctest --preset windows-asan
```

pluginval 路径与完整 MSVC 环境初始化见 `docs/ENVIRONMENT.md`。CI 命令应使用 portable preset，不能依赖本机 F: 盘绝对路径。

## 9. Milestone gates

- M0：fresh clone 可配置/构建/测试，CI PASS，模板和 branch protection 就绪。
- M1：参数枚举/state/automation smoke、gain skeleton、finite output、offline render、pluginval PASS。
- M2/M3：各自 vertical slice 的 property/render/listening/pluginval PASS。
- M4：完整 routing matrix、mode retention、click-free automation、loudness A/B PASS。
- M5：UI attachment、gesture/history、resize/accessibility 基线 PASS。
- M6：全矩阵、ASAN、长稳、多实例、DAW、CPU/memory、listening regression PASS。
- M7：Release clean build、VST3 validation、兼容性和 packaging 签核，known blockers=0。
