# FRAZIL 分阶段 Coding Plan

> 版本：1.2<br>
> 状态：Approved Development Baseline<br>
> 日期：2026-09-11
> 输入：`FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` + 当前源码/构建/远端审计  
> 目标：把产品语义转化为可排序、可分工、可验收、可在 GitHub 追踪的工程工作。

## Modification Policy

本计划是 CONTROLLED 工程合同。工作项、依赖、验收标准和 exit gate 的修改必须通过 issue/review，并同步受影响的架构、测试、参数或治理文档；本文件不记录实时 issue 状态，也不以状态文字替代验证证据。

`CODING_PLAN.md v1.1` 是当前 FRAZIL M0 到 M7 的 Approved Development Baseline，不等于 FRAZIL plugin v1.0 release。文中较早的 v1.0 表述属于历史文字，不覆盖当前 v1.1 合同；插件当前仍处于 M0/M1 早期开发阶段。

## 1. 计划使用方式

本计划中的每一行工作项都应成为一个 GitHub issue，稳定 ID 写入 issue title、branch、PR 和 changelog。状态只允许：Backlog、Ready、In Progress、Review、Validation、Done、Blocked。

Issue 进入 Ready 前必须填写 Implementation DRI、Acceptance DRI、scope/non-goals 和验收标准。write
ownership、Allowed/Forbidden paths、双方输入/输出、handoff condition、Joint Gate 和 related contract/ADR
只在 cross-module、production DSP、contract/ownership-sensitive、Host handoff 或 milestone-gate 工作中要求；
普通 bounded task 不承担完整字段税。DRI 描述工作 ownership，不等同于 PR creator、commit
author/committer 或 reviewer identity。完整规则见
[`COLLABORATION_ROLES.md`](COLLABORATION_ROLES.md#8-issue--pr-task-contract)。

一个工作项只有在代码/文档、自动测试、必要听测/DAW 证据和 review 全部满足后才是 Done。`[x]` 只表示本地审计确认，不表示已在 GitHub 关闭。

### 文档职责边界

`CODING_PLAN.md` 是稳定的工程计划合同，只定义 milestone、工作项、依赖、优先级、交付物、验收标准、exit gate 以及架构/流程约束。它不维护某个 issue 的实时状态，也不替代 GitHub Issues 或 Project。

`docs/PROJECT_STATUS.md` 记录当前 milestone、已验证能力、blocker、验证结果和下一步建议；GitHub Issues/Project 是单个工作项实时状态的唯一来源。计划中的工作项即使已经在本地或远端完成，也保留在这里作为稳定定义，不在本文件维护动态 todo 清单。

`docs/CORE_IMPLEMENTATION_GUIDE.md` 维护实现解释、候选 DSP 数学和工程方法参考。它不定义 milestone status、work-item dependencies、acceptance criteria、parameter contracts 或 Accepted architecture decisions；这些仍由本文件、`PARAMETERS.md`、架构总纲和 ADR 负责。

### Production Definition of Done

任何 production issue 只有同时满足以下条件才可标记 Done：

```text
Implementation complete
AND Tests PASS
AND Code quality review complete
AND Comment & Documentation Pass complete
AND MODULE_INDEX/module README synchronized
AND No unexplained realtime regression
AND Required review complete
```

Issue 必须明确回答 Architecture impact、Public interface impact、Parameter/state impact、Realtime impact、Ownership impact、Tests、Comments、Module docs、`MODULE_INDEX` 和 ADR；无影响项写 `N/A`，不得省略。

所有 production issue 同时受 [Documentation Synchronization Gate](DOCUMENT_GOVERNANCE.md#5-documentation-synchronization-gate) 约束。Documentation Impact Analysis、same-PR documentation synchronization 以及 Documentation Consistency Review 属于 Production Definition of Done。

优先级：

- P0：当前 milestone exit gate 的必要条件；
- P1：显著改善质量，但不阻塞当前 milestone；
- P2：v1 明确推迟。

不采用日历式虚假精度。团队用 issue 大小和依赖图排期；单个 issue 若无法由一人用一个短生命周期 PR 完成，必须继续拆分。

## 2. 计划关键路径与依赖

### Critical path

```text
M0
Repository / CI / Governance
  |
  +-> HOST-000
  +-> CI-001 / Hosted CI validated
  |
  v

M1
Parameter / State / Engine Contract
  |
  +-> TESTDATA-001
  +-> PERF-BASE-001
  +-> automation granularity contract
  |
  v

Engineering Evidence (TESTDATA/PERF/ARCH/TEST-002) --------+
                                                            |
Sound / Host Evidence (HOST-001) ---------------------------+
                                                            v
                       M1 Joint Exit Review
                              |
                 +------------+------------+
                 |                         |
                 v                         v
              M2 Water                  M3 Ice
                 |                         |
                 v                         v
              EXP-W-002                EXP-I-002
                 |                         |
                 v                         v
               EXP-W-003                EXP-I-003
       (dual-mode validation)     (candidate selection)
          (LISTENING-001 ready)    (LISTENING-001 ready)
                 |                         |
                 v                         v
               ADR-W-001                ADR-I-001
                 |                         |
          Water production          Ice production
           WATER-001..008             ICE-001..007
                 |                         |
                 v                         v
              M2 Exit                  M3 Exit
                 |                         |
                 +------------+------------+
                              |
                              v
                       PARAM-FREEZE-001
                              |
                              v
                           ADR-R-001
                              |
                              v
                           M4
                              |
                              v

M5
UI / Edit History
                 |
                 v

M6
Beta Hardening
                 |
                 v

M7
v1.0 Release

Parallel during M1 (preparatory only; does not change milestone state):
  EXP-W-001 / EXP-I-001 perceptual brief preparation
  LISTENING-001 representative corpus preparation

Shared listening side dependency (not a Water/Ice synchronization barrier):
  LISTENING-001
       +--> EXP-W-003
       +--> EXP-I-003
```

M2 Water 和 M3 Ice 在 M1 Joint Exit 之后是 independent pipelines。`EXP-W-*` 阶段不依赖对应的
`EXP-I-*` 阶段，反之亦然；除计划明确写出的 shared gate 外，两条 pipeline 不互相等待。每条
Water pipeline 独立经过 dual-mode product-direction validation，Ice pipeline 独立经过 candidate selection；
两者随后各自经过 algorithm ADR、production implementation 和自己的
milestone exit；两条 pipeline 只有在 M2 Exit 与 M3 Exit 都完成后，才在 `PARAM-FREEZE-001` 汇合。
`EXP-W-002` / `EXP-W-003` 是 M2 Water work items，`EXP-I-002` / `EXP-I-003` 是 M3 Ice work items；
图中将它们放在对应的 M2/M3 分支下，不表示它们是 milestone 之前的前置条件。
M1 期间只允许准备 `EXP-W-001` / `EXP-I-001` perceptual briefs 和 `LISTENING-001` corpus，且仅限
non-production preparation；这不表示 M2/M3 已正式开始，也不授权 production Water/Ice DSP 或 candidate
production integration。Engineering candidate work 从 M1 Joint Exit 后开始。
`PARAM-FREEZE-001` 必须在 M2/M3 完成后、M4 开始前完成；`ADR-R-001` 必须在任何 `ROUTE-006`
实现前 Accepted。任何 Water/Ice 生产实现都依赖 M1 的 ProcessSpec、EngineParameters、Snapshot、统一测试
素材和性能 baseline。

## 3. 跨模块完成合同

| 模块 | 必须提供 | 明确不负责 | 核心验收 |
|---|---|---|---|
| PluginProcessor | JUCE lifecycle、bus、Snapshot 入口、state adapter | Water/Ice 算法、routing math、UI layout | pluginval、mono/stereo、state/parameter tests |
| ParameterLayout | `src/plugin/ParameterLayout.*` 的 Host/JUCE-facing 静态 ID/type/range/default/label/choice | DSP mapping、显隐 | 精确枚举 regression |
| ParameterSnapshot | 每 block 一致的 POD 值 | smoothing、APVTS ownership | 无分配、一致性测试 |
| ParameterMapper | user -> engine 语义、clamp、enum | buffer、Host timeline | table-driven unit tests |
| AudioEngine | gain/routing/global mix 编排与生命周期 | 算法细节、UI/history | signal-chain/integration tests |
| WaterProcessor | 完整 Water transform 与少量 macro | stage amount、routing、Host | property/render/listening/perf |
| IceProcessor | 完整 Ice transform 与少量 macro | stage amount、routing、Host | property/render/listening/perf |
| RoutingEngine | Parallel/两个 Serial、enable、transition | Water/Ice 内部算法 | routing matrix、click-free、retention |
| StageMixer | dry/processed mix law | Host/UI | endpoint/monotonicity/energy tests |
| EditHistoryManager | UI transaction、bounded undo/redo | Host automation、audio thread | gesture/source/state tests |
| UI | 产品参数表达、attachment、gesture，以及 narrow app edit/history command interface 的调用 | DSP 执行、动态参数注册、直接持有 DSP object | interaction/resize/automation tests |

## 4. M0 — Repository, Governance & Reproducible Build

### 目标

从“本机可构建目录”变成“fresh clone 可复现、可以安全协作、CI 可作为合并门槛”的仓库。此阶段不开发声音功能。

### 工作包

| ID | P | 工作内容 | 交付物 | 验收标准/证据 | 依赖 |
|---|---:|---|---|---|---|
| REPO-001 | P0 | 初始化 `main`、关联 `origin`、审计首次提交 | Git history + remote main | `git status` clean；remote 正确；忽略项未入库；GitHub 可 fresh clone | owner 授权 push |
| DOC-001 | P0 | 纳入架构总纲、plan、status、parameters、testing、workflow、ADR、贡献规范 | `docs/` + AGENTS/README/templates | 文档链接有效；现状与目标分离；review 通过 | 无 |
| ENG-STD-001 | P0 | 建立 code quality 与 documentation governance | `docs/CODE_STANDARDS.md`、`docs/DOCUMENT_GOVERNANCE.md`、`docs/MODULE_INDEX.md`、AGENTS 和 module README 更新 | cohesion、coupling、ownership、realtime、comments、module docs、document protection 和 post-code documentation pass 形成强制合同 | DOC-001 |
| LEGAL-001 | P0 | 由 owner 选择并加入开源许可证 | `LICENSE` + README 声明 | 许可证与第三方 notices 一致；提交音频许可可追溯 | owner 产品决定 |
| DEP-001 | P0 | 决定 JUCE 固定与补丁策略 | Accepted ADR-0004 | fresh directory 获取精确 JUCE revision；补丁可审计；不依赖 C: 或临时文件 | REPO-001 |
| BUILD-001 | P0 | 建立 repository-portable Windows preset 与路径防回归检查 | presets/toolchain/CMake | 本地三 preset 与 CI preset 不引用开发者路径；scanner 在新增个人绝对路径时失败 | DEP-001 |
| BUILD-002 | P0 | 建立依赖 bootstrap/checksum | script + docs | 空 `external/` 可按固定版本恢复；重复执行幂等；失败信息清晰 | DEP-001 |
| TEST-001 | P0 | 接入可扩展单元测试框架 | `frazil_tests` target | CTest 能发现多个 case；失败返回非零；不依赖 plugin GUI | BUILD-001 |
| CI-001 | P0 | Windows PR workflow | `.github/workflows/ci.yml` | clean checkout 先通过 portability scan，再完成 Debug configure/build/test PASS；最小权限；缓存失效规则正确 | BUILD-001/2, TEST-001 |
| CI-002 | P1 | Release/ASAN/pluginval scheduled jobs | workflow jobs | 可手动触发；artifact/log 可追踪 commit；失败可诊断 | CI-001 |
| HOST-000 | P0 | Freeze initial platform and DAW compatibility matrix | 支持矩阵 ADR/文档 | 记录 v1 正式支持、开发验证、best-effort 的 OS/架构/格式/DAW/Standalone 角色；目标 DAW 可用于后续 M1 smoke | REPO-001 |
| GH-001 | P0 | 创建 labels、M0-M7 milestones、Project board | GitHub metadata | issue 可按 type/area/priority/milestone 查询 | REPO-001 |
| GH-002 | P0 | 配置 main protection | ruleset | PR、1 approval、CI、resolved conversations、no force push | CI-001 稳定 |
| TOOL-001 | P1 | 增加 formatting check | clang-format target/job | 与 `.clang-format` 一致；不自动改 vendor/generated | CI-001 |

### M0 stable ownership

| Work | Implementation DRI | Acceptance DRI | Write boundary |
|---|---|---|---|
| `GH-001/002`、CI、build、portability、dependency regression | Engineering Lead | Sound & Host Lead | repository/build/workflow paths；不重做 Accepted JUCE dependency strategy |
| `HOST-000` compatibility content/support classification | Sound & Host Lead | Engineering Lead | compatibility/evidence docs；不得把未执行 Host evidence 写成事实 |
| audio/reference licensing product review | Sound & Host Lead | Engineering Lead | license/evidence；frozen engineering corpus 变更需显式 Issue |

### 实施顺序

1. DOC-001、ENG-STD-001、LEGAL-001 与首次提交审计；
2. REPO-001；
3. DEP-001 -> BUILD-001/002；
4. HOST-000；
5. TEST-001 -> CI-001；
6. GH-001/002；
7. CI-002/TOOL-001。

### Exit gate

```text
fresh clone
-> repository portability scan PASS
-> obtain pinned dependencies
-> configure portable Debug
-> build FRAZIL_All + tests
-> CTest PASS
-> CI required check PASS
-> HOST-000 matrix recorded
-> code/document governance and module index present
-> Standalone launches
-> VST3 can be inspected by pluginval smoke
```

Proof artifacts：CI run URL、依赖 revision/checksum、首次提交 tree、local/CI command log、已启用的 branch ruleset 截图或导出。

## 5. M1 — Audio Skeleton, Parameter & State Contract

### 目标

在声音算法前证明完整的 Host -> Snapshot -> Engine -> Output 和 state round-trip 路径。M1 结束时插件仍可无特色，但不能再是“参数不生效的 pass-through”。

### 推荐 PR 序列

#### M1-A：核心类型与参数

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| ARCH-001 | P0 | `ProcessSpec` + `EngineParameters` | 值类型、自包含 header；sampleRate/maxBlock/channels 前置校验；无 JUCE Host 对象泄漏进 DSP | lifecycle unit tests |
| PARAM-001 | P0 | 提取 `src/plugin/ParameterLayout.*` | 在 Platform / Host Adapter 层集中注册 9 个核心参数；enabled ID 改为 `.enabled`；固定顺序/choice | 精确 contract test |
| PARAM-002 | P0 | `ParameterSnapshot` | 缓存 raw parameter atomic pointers；每 block 一次 load；不查字符串、不分配 | snapshot consistency test |
| PARAM-003 | P0 | `ParameterMapper` | routing enum、clamp、dB/normalized、inactive 值保留 | table-driven boundary tests |
| PARAM-004 | P0 | automation gesture smoke | Host/UI 写值能进入下一 block Snapshot；参数枚举稳定 | integration + pluginval |
| AUTO-001 | P0 | Define v1 automation granularity contract | Host 参数每 block 建立 coherent snapshot；连续参数由 DSP sample-aware smoother 处理；离散参数显式 transition；不承诺 sample-accurate Host automation | block-size/automation/no-click contract tests |

PARAM-001 合并前必须确认是否存在任何外部构建/session 依赖旧 ID；若有，先更新 ADR-0002 的迁移方案。

#### M1-B：真实基础信号链

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| APP-001 | P0 | AudioEngine 重构 | `prepare(ProcessSpec)`、`reset()`、`process(buffer, EngineParameters)`；明确 dry scratch ownership | prepare/reset/process tests |
| DSP-001 | P0 | Input Gain | dB -> linear；post-input 同时喂 dry reference 与 wet；prepare 后平滑初始化 | impulse/sine/gain automation |
| DSP-002 | P0 | Global DryWetMixer | `mix=0` 精确 dry，`1` wet；中间 law 先显式记录 | endpoint/monotonicity tests |
| DSP-003 | P0 | Output Gain | mix 后执行；不反馈进 Water/Ice | reference level tests |
| DSP-004 | P0 | smoothing primitive/policy | sample-rate aware；不分配；reset/retarget 可预测 | step response + no-click proxy |
| DSP-005 | P0 | RandomSource contract | M1 建立最小 generic RandomSource primitive/contract：deterministic fixed seed、explicit seed injection、production instance decorrelation、无全局可变状态、instance isolation、reseed、realtime-safe API 和无 audio-thread allocation；M2/M3 只补 algorithm-specific random semantics | exact sequence/reseed/instance-isolation tests |

M1 的 wet path 可暂时等于 post-input pass-through，以单独验证 gain/global mix。此时 `global.mix` 不应改变声音，因为 dry/wet 相同；测试需解释这一点，不能误判为参数未接入。

#### M1-C：State boundary 与工具

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| STATE-001 | P0 | versioned StateModel/adapter and history boundary | `schemaVersion`、全部参数、invalid input fallback；非音频线程迁移；明确 Host automation/restore 不进入 plugin history 的边界 | fixtures + round-trip + boundary review |
| STATE-002 | P0 | mode value retention | 切换三种 mode、保存、恢复，不重置 inactive values | integration scenario |
| TESTDATA-001 | P0 | Establish deterministic DSP diagnostic corpus | Layer B canonical ten-signal diagnostic corpus；SignalSpec mathematical definitions；manifest schema v2；PCM24/provenance/hash/storage；standard-library semantic regression；temporary 44.1/48/96 kHz generation；与 Layer D listening corpus 解耦 | deterministic regeneration + byte/hash equality + per-signal semantic checks + RENDER-001 shared-input regression |
| PERF-BASE-001 | P0 | Establish realtime performance baseline | Reference Machine、OS、compiler、build type、48 kHz/128、测量方法；记录 mean/P95/P99/worst callback、deadline、memory、allocation observation | reproducible baseline report；不预设百分比阈值 |
| ARCH-LAT-001 | P0 | Define v1 processing latency and intentional delay/tail policy | v1 Host-reported processing latency 为 0 samples；不依赖 lookahead、FFT block latency、linear-phase、convolution 或 Host PDC；Water/Ice 允许属于声音设计的 intentional effect delay/tail，但不得依赖 Host latency compensation | ADR + latency metadata/infrastructure acceptance |
| RENDER-001 | P0 | offline WAV harness | 固定 `TESTDATA-001` input/config/seed -> WAV + manifest；不依赖实时设备；candidate A/B 可复现 | deterministic smoke render |
| TEST-002 | P0 | processor property harness | finite/random/extreme/prepare-reset；矩阵参数化 | 44.1/48/96 + block subset |
| HOST-001 | P0 | pluginval + Host smoke protocol | 枚举/state/editor/bus/automation；结果可追踪 SHA；DAW 目标来自 HOST-000 | strictness 5 PASS |

### M1 ownership and handoff

任一工作项的实现与验收进入 `main` 后，只做 regression/finding follow-up，不得创建平行实现；实际
完成状态以 GitHub 和 `PROJECT_STATUS.md` 为准。`RENDER-001` 的后续验收或 matrix 必须复用既有 harness，
不能另建平行 render architecture。

| Work item | Implementation DRI | Acceptance DRI / input owner | Allowed scope | Explicit non-goals |
|---|---|---|---|---|
| `RENDER-001` acceptance/follow-up | Engineering Lead | Sound & Host Lead | existing render target/tools/tests/docs | Water/Ice/Routing、listening choice、DAW acceptance |
| `TEST-002` | Engineering Lead | Sound & Host Lead 提供风险场景 | processor property/unit/integration tests | 新声音算法、routing/UI production |
| `PERF-BASE-001` | Engineering Lead | Sound & Host Lead 提供 workload/material/block size | performance harness/report/build-test wiring | formal budget、Water/Ice/Routing/UI |
| `ARCH-LAT-001` | Engineering Lead | Sound & Host Lead 验收 intentional delay/tail | latency metadata、impulse evidence、ADR/docs | lookahead/FFT/convolution、routing implementation |
| `HOST-001` | Sound & Host Lead | Engineering Lead 提供 build/plugin 并修复 handed-back findings | DAW evidence、matrix/result、findings | 直接修改 StateModel/Mapper/Snapshot/AudioEngine/PluginProcessor |
| M1 Joint Exit Review | 双方各自维护所属 evidence | Joint Gate | Engineering Evidence + Sound/Host Evidence | 任一证据链替代另一条 |

Acceptance DRI 发现 production 问题时默认先创建可复现 finding，再交回 Implementation DRI。当前 task
scope 明确包含的 typo、小型 test/docs 或 trivial integration fix 可由另一角色完成；只有 substantial
implementation responsibility 换人时才记录 Implementation DRI Transfer。

### M1 Exit gate

`TESTDATA-001` 的独立 exit criteria 是：十个 diagnostic signal 存在且 filename/ID 直接表达
测试目标；manifest schema v2 的 signal definition、provenance、license、redistribution、hash
和 storage audit 完整；generator 支持 per-signal duration、PCM24、stable per-signal seed 和
44.1/48/96 kHz temporary generation；每个 canonical signal 有 standard-library semantic
verification；analysisMethods 已为未来 measurement tooling 记录。FFT/PSD/STFT 等大型分析基础
仍不是本 follow-up 的实现目标。M1 不要求
final Water/Ice probes、algorithm-specific acceptance thresholds 或真实 listening corpus；这些
随 `EXP-W-*`、`EXP-I-*`、`ADR-W-001` 和 `ADR-I-001` 推进。

- Parameter registry 与 `docs/PARAMETERS.md` 完全一致；
- AudioEngine 实际消费 Snapshot/EngineParameters；
- gain/global mix 位置与数学由 tests 证明；
- state version、round-trip、inactive retention 和 invalid state 通过；
- Host automation、state restore 与 plugin EditHistory 的边界已记录；M1 不实现完整 history store；
- process path 审查无 I/O/lock/allocation/history/UI；
- `AUTO-001` 的 block snapshot、sample-aware smoothing 和离散 transition 语义有测试证据；
- `TESTDATA-001` manifest/hash/license 可追溯，`RENDER-001` 使用统一 corpus；
- `PERF-BASE-001` 已记录 baseline，不以未测量的 CPU 百分比作为门槛；
- `ARCH-LAT-001` 已接受，routing/mixing infrastructure 不引入未声明 processing latency，v1 Host-reported latency 为 0 samples；Water/Ice intentional effect delay/tail 由各自 ADR/tests 描述；
- offline render 可复现，property tests 无 NaN/Inf；
- Debug/Release/ASAN + pluginval PASS；
- `HOST-000` 中定义的 primary target DAW 完成参数枚举与 project save/reopen smoke。

### M2/M3 Shared Perceptual Preparation

`LISTENING-001` 是跨 milestone 的 **Shared Representative Listening Corpus**，当前不属于
M1-C，也不是 M1 Exit Gate 的 P0 blocker。其 Implementation DRI 为 Sound & Host Lead，
Acceptance DRI 为 Engineering Lead。允许的 scope 包括 licensed representative musical
material、source/author、license、redistribution permission、hash、sample rate、bit depth、
channel count、duration、storage policy 和 Water/Ice listening rubric suitability。

Acceptance criteria：representative coverage accepted；source/license/redistribution audit complete；
metadata/hash/storage evidence complete；material suitable for the Water/Ice listening rubric；
Engineering Lead evidence review complete。DAW acceptance 不属于 `LISTENING-001`。

明确 non-goals：`TESTDATA-001` engineering fixtures、production Water/Ice DSP、HOST-001
兼容性证据、DAW automation evidence，以及 DSP mathematical acceptance。

依赖关系必须保持为：`LISTENING-001` 可以在 M1 期间并行准备，不阻塞 M1 Exit，也不阻塞
`EXP-W-002` / `EXP-I-002` engineering experiments；但它必须在 `EXP-W-003` Water dual-mode
validation、`EXP-I-003` Ice candidate selection 以及 `WATER-006` / `ICE-006` final listening evidence 之前
ready。`TESTDATA-001` 负责 engineering evidence，`LISTENING-001` 负责 perceptual evidence，
两者不可互相替代。

## 6. M2 — Water Dual-Mode Material Processor

### 目标

验证并实现一个可被音乐素材驱动、可辨识为 Water、保持输入可辨识度且实时安全的双模式
material processor。已决定的产品方向包含两个有意区分、同属 Water 的行为：

- `Fluid`：A（input-driven Bubble Ensemble）+ B（input-driven Droplet/Impact Exciter）+
  D（Flow Modulator），强调气泡/液体身份、水滴瞬态与连续不规则流动；
- `Resonant`：C（input-driven Liquid/Modal Resonator），强调稳定凝聚的液体共振、对有音高素材的
  兼容性、较低复杂度和可预测音乐响应。

二者都不是独立 Water Foley generator。WaterProcessor 的 source-preserving 设计原则是
`W(x) = x + E_water(x)`，其中 `E_water(x)` 是由输入驱动的材质 residual；这是 FRAZIL 的工程/产品
推论，不是引用论文给出的物理定律。每个 sub-engine 必须显式声明返回 residual 还是 complete
processed signal，优先采用 residual-oriented architecture，避免 Resonant feedthrough 被上层重复相加。
本 milestone 不加入水声采样层，也不改变 Parallel/Serial routing ownership。

本次 Water-focused revision 不改变 Ice 的算法、参数、work item 或测试；Ice 留待后续独立修订。

### 研究波次

| ID | P | 工作 | 交付/验收 |
|---|---:|---|---|
| EXP-W-001 | P0 | Water dual-mode perceptual/product brief | 定义 common Water identity、Fluid identity、Resonant identity、Size/Motion 语义、正常设置及 `global.mix=100%` Water-only 下的 input recognizability、反例、Motion 与 Amount 区别，以及双模式可用性评价表 |
| EXP-W-002 | P0 | 分组件工程实验与集成 | 使用 `TESTDATA-001` 分别测量 Bubble Ensemble、Droplet/Impact、Flow Modulator、Liquid/Modal Resonator 及集成；Fluid 最终目标仍为 A+B+D，但要求 A/B/D ablation；C 作为 Resonant baseline/mode；随机路径使用 fixed seed；记录参数空间、finite/DC/peak/tail 和 CPU 初测，不把 diagnostic WAV 当作 musical acceptance |
| EXP-W-003 | P0 | 双模式方向验证与 refinement | 使用 `LISTENING-001` 做 Fluid vs Resonant loudness-matched 双人 review；验证模式区分、两模式 Size/Motion 语义一致性、input recognizability、musical usefulness、最终 mapping 理由、风险与 tradeoff；不再以“只选一个 vertical slice”为目标 |
| ADR-W-001 | P0 | Water 双模式算法 ADR | 在 Joint Gate 后记录 Fluid A+B+D、Resonant C、source-preserving/residual 语义、macro mapping、random、latency/tail、mode transition、state implications、performance 与 failure modes；证据不足时保持 Proposed，不得标记 Accepted |

工程实验必须把 high-fidelity coupled-bubble/full-fluid/FDTD literature 当作研究依据和近似误差提示，
不得把完整流体模拟或数十万气泡处理设为 v1 realtime callback 要求。可将声学相关 fluid events
映射为有界、轻量的 synthesis primitives；具体算法、数值范围和非线性 mapping 仍由实验与
`ADR-W-001` 决定。

### 生产实现

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| WATER-001 | P0 | `WaterProcessor` dual-engine lifecycle/ownership | `prepare/reset/process(buffer, WaterParameters)`；明确 Fluid/Resonant state、tail、random 和 residual/complete-signal ownership；mono/stereo；无 routing/amount/APVTS | lifecycle/property/ownership review |
| WATER-002 | P0 | Fluid + Resonant cores | Fluid 实现经采纳的 A+B+D 有界组合，Resonant 实现经采纳的 C；固定 test seed 可复现；component ablation、输入驱动、参数极值和 source-preserving output 有证据 | render + finite + ablation + recognizability evidence |
| WATER-003 | P0 | shared product macros | 评估 candidate `water.model`、`water.size`、`water.motion`；ParameterMapper 集中 mode-specific mapping；完整语义链、自动化/平滑、state evolution 和兼容性证据齐备后才可正式注册；不得增加 `water.dryWet` | mapping/automation/listening/state compatibility tests |
| WATER-004 | P0 | click-free enable | disabled=pass-through；重新开启保留 macro；过渡无异常峰值 | transient automation render |
| WATER-005 | P0 | dual-mode performance/tail | 分别记录 Fluid、Resonant 和 mode-transition 相对 `PERF-BASE-001` 的 mean/P95/P99/worst；不引入非零 Host-reported processing latency；intentional effect delay/tail 由 Water ADR/tests 描述；M2 不预设最终硬预算 | benchmark + plugin metadata |
| WATER-006 | P0 | dual-mode listening pack | 使用 `LISTENING-001` 做 dry/baseline/Fluid/Resonant loudness-matched review；验证 Water identity、模式区分、Size/Motion 方向、正常设置及 `global.mix=100%` 下 input recognizability；工程诊断仍由 `TESTDATA-001` 提供 | rubric accepted；未触发 Reject Criteria |
| WATER-007 | P0 | parameter/state/integration | Water-only AudioEngine 临时路径；如正式采纳新 Host controls，先完成静态注册、choice ordering、automation、inactive retention、state compatibility/evolution fixtures；不引入 routing 语义 | pluginval + DAW automation + state compatibility |
| WATER-008 | P0 | Water model transition | 在 `ADR-W-001` Accepted 后实现 Fluid/Resonant click-free bounded transition；定义 dual-engine execution、state/tail/random progression、rapid automation、reset/prepare/restore 和 CPU upper bound；不预设 transition duration | automation stress + render + property + performance |

### M2 pipeline ownership

| Work | Implementation DRI | Acceptance / decision |
|---|---|---|
| `EXP-W-001` | Sound & Host Lead | Engineering Lead feasibility review |
| `EXP-W-002` candidate implementation/render/measurement | Engineering Lead | Sound & Host Lead owns experiment question、fixtures、A/B 和 rubric inputs |
| `EXP-W-003` dual-mode validation record | Sound & Host Lead | Engineering feasibility gate；algorithm adoption = Joint Gate |
| `ADR-W-001` technical record | Engineering Lead | Joint Gate |
| `WATER-001/002/003/004/005/007/008` | Engineering Lead | Sound & Host Lead |
| `WATER-006` | Sound & Host Lead | Engineering Lead evidence review；final acceptance = Joint Gate |

### M2 Exit gate

Water-only 的 Fluid 与 Resonant 在 `LISTENING-001` 代表性素材上各自具有一致 Water identity 且可明确
区分；正常产品设置及代表性的 `global.mix=100%` Water-only 评估中输入仍具音乐可辨识性；Size/Motion
在两模式保持一致高层语义且 Motion 不主要表现为增益。工程诊断在 `TESTDATA-001` 上通过；所有正式
宏符合 `AUTO-001` 且无明显 zipper；mode/enable transition、state/seed/render/property/performance/
pluginval 通过；Water listening rubric 和 Reject Criteria 通过；
WaterProcessor 未依赖 Host、UI 或 RoutingMode。

## 7. M3 — Ice Vertical Slice

### 目标

产出可被输入驱动、可辨识为 Ice、与 Water 听感和算法机制足够区分的 production vertical slice。

### 研究与实现

| ID | P | 工作 | 具体要求/验收 |
|---|---:|---|---|
| EXP-I-001 | P0 | Ice perceptual brief | 冰晶/摩擦/脆裂/硬度等属性、反例、参考与评价表 |
| EXP-I-002 | P0 | 候选机制实验 | 工程 measurement/regression 使用 `TESTDATA-001` diagnostic corpus；至少两候选；固定测试 seed；A/B；CPU 和极端参数；不把 diagnostic WAV 当作 musical acceptance |
| EXP-I-003 | P0 | vertical slice selection | 使用 `LISTENING-001` representative listening corpus，按 Ice listening rubric 双人听测；最多 2 个首批 macro；风险和弃选记录 |
| ADR-I-001 | P0 | Ice 算法 ADR | 结构、transient/random、latency/tail、mapping、预算 |
| ICE-001 | P0 | `IceProcessor` lifecycle | 与 Water 接口习惯一致但不强求内部对称；无 routing/APVTS |
| ICE-002 | P0 | Ice core | 选定的摩擦/晶体/裂纹机制最小组合；finite/repeatable |
| ICE-003 | P0 | product macros | 用户语义、mapping、smoothing、automation/state |
| ICE-004 | P0 | enable transition | pass-through、value retention、click-free |
| ICE-005 | P0 | performance/tail | 相对 `PERF-BASE-001` 的 Reference baseline 报告 mean/P95/P99/worst；不引入非零 Host-reported processing latency；intentional effect delay/tail 由 Ice ADR/tests 描述 |
| ICE-006 | P0 | listening pack | 使用 `LISTENING-001`，跨代表性素材且与 Water 可区分；工程诊断另由 `TESTDATA-001` 提供 measurement/regression evidence；按 Ice rubric 记录 |
| ICE-007 | P0 | integration | Ice-only AudioEngine 路径 + pluginval/DAW smoke |

### M3 pipeline ownership

| Work | Implementation DRI | Acceptance / decision |
|---|---|---|
| `EXP-I-001` | Sound & Host Lead | Engineering Lead feasibility review |
| `EXP-I-002` candidate implementation/render/measurement | Engineering Lead | Sound & Host Lead owns experiment question、fixtures、A/B 和 rubric inputs |
| `EXP-I-003` selection record | Sound & Host Lead | Engineering feasibility gate；algorithm adoption = Joint Gate |
| `ADR-I-001` technical record | Engineering Lead | Joint Gate |
| `ICE-001/002/003/004/005/007` | Engineering Lead | Sound & Host Lead |
| `ICE-006` | Sound & Host Lead | Engineering Lead evidence review；final acceptance = Joint Gate |

候选可探索 FrictionTexture、CrackTransientGenerator、ModalResonator、CrystalExciter 或 SpectralShaper。随机裂纹事件必须有可控密度/幅度上界，不得造成不可预测爆峰。

### M3 Exit gate

与 M2 同级质量门槛并通过 Ice listening rubric 和 Reject Criteria；额外要求 Water/Ice 对照盲听能稳定区分，两个模块不因“复用”而被迫共享不合适的算法抽象。

### M2/M3 后的 v1 Host contract gate

| ID | P | 工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| PARAM-FREEZE-001 | P0 | Freeze v1 host parameter contract | Water/Ice product macros、所有 Parameter ID、order、choice index、range、default、unit、smoothing、inactive-mode behavior 全部定稿；automation tests 与 state compatibility fixtures 齐备 | M2/M3 exit gate 已通过；contract review/ADR accepted |

M1 只负责 `core contract stabilization`：建立静态参数、Snapshot、mapping、state 和 automation 的基础合同。`PARAM-FREEZE-001` 才是 `v1 host API freeze`。完成后 M4/M5 不得随意修改 Host Parameter ID；确需修改时必须通过 ADR 与 migration/compatibility review。

## 8. M4 — Routing & Material Engine Integration

### 目标

在 `PARAM-FREEZE-001` 和 `ADR-R-001` Accepted 后，实现 ADR-0001 的完整信号合同，建立 click-free routing 与全模式 render/automation/state 证据。

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| ADR-R-001 | P0 | Define routing transition and DSP state ownership | 决定 old/new topology 是否同时运行、两套 DSP state 如何管理、是否复制 Processor state、是否双 graph、random state 推进、delay/resonator tail、最大 transition CPU 成本及两阶段 transition 是否可用 | ADR Accepted 后才可开始 ROUTE-006 |
| ROUTE-001 | P0 | scratch buffer plan | prepare 期为 Parallel 双支路和 transition 预分配；支持 max block/channels | allocation/size tests |
| ROUTE-002 | P0 | StageMixer | stage dry/processed endpoints；amount 平滑；可选 equal-power 仅经 ADR | unit + energy tests |
| ROUTE-003 | P0 | Parallel | 输入复制到 Water/Ice；balance；enable 四组合；两关为 pass-through | complete matrix renders |
| ROUTE-004 | P0 | Water -> Ice | Water stage mix 后喂 Ice；amount 语义独立 | 0/0..100/100 matrix |
| ROUTE-005 | P0 | Ice -> Water | 顺序反转但 ID/amount 语义不变 | 同上 |
| ROUTE-006 | P0 | mode transition | `ADR-R-001` Accepted 后实现；路由切换 click-free；不 reset 参数/算法状态；CPU 峰值有界；不得默认在一个 block 内复用同一有状态 Processor 推进 old/new 两次 | automation stress/render |
| ROUTE-007 | P0 | enable transition | 单模块 bypass 不硬切；inactive 值保留 | transient cases |
| ROUTE-008 | P0 | Global integration | Input Gain -> Routing -> Global Mix -> Output Gain 顺序固定 | impulse/reference test |
| ROUTE-009 | P0 | Host/state matrix | mode 切换、inactive automation、save/reopen、多实例 | integration + DAW |
| ROUTE-010 | P0 | loudness review | Parallel law、Serial stage law、routing 切换的响度与偏好 | A/B notes + ADR update |
| ROUTE-011 | P0 | verify routing infrastructure latency and reporting | 验证 RoutingEngine、branch copies、scratch buffers、Parallel/Serial topology 和 Global Mix infrastructure 不额外引入非预期时间偏移或未声明 processing latency；bypass/identity impulse 不被无故平移；v1 report 0 samples；Water/Ice intentional delay/tail 由算法 ADR/tests 单独描述 | infrastructure impulse/render + plugin metadata/Host check |

`ADR-R-001` 不预设最终实现。允许比较完整 old/new topology 双运行、复制/双 graph、以及 `old routing -> neutral/dry -> switch topology -> new routing` 的两阶段 transition；必须用测量和确定性测试说明 state、random、tail 和 CPU 后再选择。

### 关键不变量

- Water/Ice 不知道自己处于 Parallel 还是 Serial；
- `parallel.balance` 永不充当 serial amount；
- amount 属于 StageMixer，不进入 Water/Ice 内部算法；
- Input Gain 同时改变 dry reference 和 wet input；Output Gain 只在末端；
- `ARCH-LAT-001` 的 v1 Host-reported latency=0 与 `ROUTE-011` 的 routing infrastructure latency 检查必须成立；intentional Water/Ice delay/tail 不被误判为 plugin processing latency；
- routing/enable 改变不重新注册 Host 参数、不清除 inactive values；
- v1 不增加内部 Water->Ice 时间线。

### M4 Exit gate

`docs/TESTING.md` 的完整 routing render matrix、automation stress、mode retention、state reopen、mono/stereo、sample-rate/block-size 子矩阵和 pluginval 全通过；`ADR-R-001` 与 `ROUTE-011` 证据齐备；切换无明显 click；性能数据相对 `PERF-BASE-001` baseline 记录，且无未解释的严重 realtime regression；正式阈值由 M6 `PERF-001` 基于实测锁定；听测确认 crossfade/stage law。

## 9. M5 — Product UI & Edit History

### 目标

让全部 P0 产品能力可快速理解和操作，同时保持 Host 参数与插件内部历史的边界。

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| UI-001 | P0 | editor 移至 `src/ui/` | PluginProcessor 只创建 editor；组件不持 DSP 引用 | dependency review/build |
| UI-002 | P0 | layout system | Header、Input、Water/Ice、Routing、Mix、Output；明确最小尺寸/resize | size matrix screenshots |
| UI-003 | P0 | routing selector | 三模式、信号流顺序清晰；Host attachment | automation + keyboard test |
| UI-004 | P0 | mode controls | Parallel 只显示 balance；Serial 显示两个 amount 且视觉顺序随 flow | retention/visibility test |
| UI-005 | P0 | module/gain controls | enabled、Water Mode（Fluid/Resonant）、共享 Size/Motion、Ice macros、Input/Global Mix/Output；Water mode 不替换完整 Water panel；0 dB reset；仅显示已正式采纳并静态注册的参数 | attachment/default/semantic consistency tests |
| HIST-001 | P0 | EditHistoryManager skeleton | bounded transaction store/API；message thread only；由 UI transaction 驱动；不记录 Host automation/restore | unit tests for capacity/clear/source isolation |
| HIST-002 | P0 | gesture transactions | mouseDown/begin -> changes -> mouseUp/end 为一步；离散操作一步 | undo boundary tests |
| HIST-003 | P0 | Undo/Redo controls | disabled state、redo branch、action label；不注册参数 | interaction tests |
| HIST-004 | P0 | source isolation | Host automation/restore/init/smoothing 不入栈；restore 清栈 | integration tests |
| UI-006 | P0 | tooltip/value formatting | 百分比/dB/choice 一致；说明 inactive mode | snapshot/manual review |
| UI-007 | P1 | keyboard/accessibility | Ctrl/Cmd+Z、Redo、focus order、可读对比度/label | platform manual test |
| UI-008 | P1 | meters/flow hint | 不阻塞音频线程；数据桥接无锁/有界 | performance + visual review |

History 实现顺序固定为 `HIST-001 -> HIST-002 -> HIST-003 -> HIST-004`。M1 只定义 State/Host automation/restore 与 plugin history 的边界合同，不实现 EditHistoryManager。

### UI 状态原则

UI 只根据 Host-visible state 决定 visible/enabled，不保存一套平行参数真相。关闭 editor、打开多个 editor 或 Host automation 变化后，控件必须从参数状态恢复一致。UI 动画不得成为 DSP 正确性的依赖。

### M5 Exit gate

全部 P0 参数可操作、可自动化、可正确格式化；mode 显隐和值保留正确；一次 drag 一步 Undo；Host automation 不污染历史；state restore 清历史；最小/目标尺寸无裁切；idle/animated UI 不导致不可接受音频 callback spike。

## 10. M6 — Hardening & Beta

### 目标

冻结功能范围，系统性消除崩溃、爆音、兼容性、性能和状态风险。

| ID | P | 工作 | 验收/输出 |
|---|---:|---|---|
| QA-001 | P0 | Debug/Release/ASAN clean matrix | clean checkout 全 PASS；无 sanitizer finding |
| QA-002 | P0 | sample-rate/block/channel matrix | 44.1/48/96 x 32..1024 x mono/stereo 关键组合 |
| QA-003 | P0 | parameter/state fuzz | 合法/边界/损坏 state；无 crash/NaN；可复现 seed |
| QA-004 | P0 | automation stress | 所有连续/离散参数；高速、并发、mode inactive 写入 |
| QA-005 | P0 | long-run/multi-instance | 长时间播放、editor 开关、多实例、sample-rate/device restart |
| QA-006 | P0 | validator | pluginval 高严格度；Steinberg validator 若已接入 |
| QA-007 | P0 | DAW matrix | 按 `HOST-000` 的正式支持/验证/best-effort 分类执行 scan/load/save/reopen/automation/render |
| PERF-001 | P0 | performance budget | 继承 `PERF-BASE-001` Reference Machine，比较各 routing mean/P95/P99/worst/CPU/memory；基于测量锁定阈值 |
| AUDIO-001 | P0 | listening regression | fixed pack 对比最后 accepted baseline；两人签核 |
| DOC-002 | P0 | user/dev docs freeze | install、controls、automation、known limitations、license |
| BUG-TRIAGE | P0 | blocker burn-down | P0=0；P1 有 owner/decision；P2 明确 deferred |

M6 开始后禁止新增 P0 产品功能。只允许 bug、compatibility、performance、test 和 documentation changes；声音算法大改返回 experiment 流程。

### Beta exit gate

所有 QA/PERF/AUDIO P0 通过；目标 DAW matrix acceptable；clean machine 安装/扫描成功；known release blockers=0；参数/state compatibility fixtures 固定；版本和 changelog 可生成。

## 11. M7 — v1.0 Release

| ID | P | 工作 | 验收 |
|---|---:|---|---|
| REL-001 | P0 | version/changelog | 单一版本源；用户可读 changelog；breaking/migration 清晰 |
| REL-002 | P0 | clean signed build | clean tag 构建；VST3/Standalone artifact；如签名则验证签名 |
| REL-003 | P0 | final validation | CI、pluginval、DAW、state、performance、listening 全签核 |
| REL-004 | P0 | packaging/install docs | 安装/卸载位置、系统要求、known limitations |
| REL-005 | P0 | GitHub Release | immutable tag、artifact/hash、release notes、license/notice |
| REL-006 | P0 | rollback/support | 保留上一 candidate、crash/bug 模板、hotfix 分支规则 |

Release gate：`CI PASS && validators PASS && DAW matrix acceptable && state compatibility PASS && performance PASS && listening PASS && blockers == 0`。

## 12. 两人并行执行建议

详细的角色职责、决策权、Issue 交接字段和 milestone 协作方式见 [COLLABORATION_ROLES.md](COLLABORATION_ROLES.md)。本节只保留稳定的阶段级分工；具体成员和实时任务状态由 GitHub Issues/Project 管理。

只在接口冻结后并行，团队 WIP <= 2；同一时间最多一个高风险 production DSP implementation：

| 阶段/波次 | Engineering Lead | Sound & Host Lead | 合流点 |
|---|---|---|---|
| M0 | Git/CI/dependency/test framework、GH-001/002 | compatibility/support/licensing/workflow usability | fresh clone + governance gate |
| M1 Engineering lane | RENDER/TEST/PERF/latency infrastructure | workload、risk scenarios、product latency/tail input | Engineering Evidence |
| M1 Host lane | handed-back engineering findings | HOST-001 Ableton -> FL Studio -> REAPER | Sound / Host Evidence |
| M2/M3 Wave A | Water candidate engineering | Water rubric + Ice perceptual brief | Water experiment review |
| M2/M3 Wave B | Water fixes + Ice experiment prototype | Water listening/DAW acceptance + Ice selection inputs | Water gate + Ice experiment review |
| M2/M3 Wave C | Ice production implementation | Water final acceptance + Ice listening preparation | Ice gate |
| M4 | RoutingEngine/state/automation integration | routing sound/loudness/DAW acceptance | full matrix gate |
| M5 | attachments/history/layout | material-control UX/audio feedback | UI acceptance |
| M6 | validators/state/fuzz/performance measurement | DAW/listening regression/product workload | beta sign-off |

M2/M3 的并行单位是 pipeline stage，不是“Developer A owns Water / Developer B owns Ice”。生产 Water/Ice DSP
均由 Engineering Lead 实现，Sound & Host Lead 保持独立声音和 Host acceptance。参数 ID、routing、算法采纳、
声音方向与 formal performance budget 均为 Joint Gate。

## 13. 明确推迟到 P2

- 插件内部 Water -> Ice morph timeline、sequencer、automation recorder；
- AAX、AU（除非 v1 目标正式变更）；
- preset cloud、用户账户、在线服务；
- 通用 modulation matrix、多频段、MIDI modulation；Water 可在真实用户证据支持后另行研究仅固定目标为 Motion 的受限 `Motion Mod` foldout，但不属于当前 v1 parameter/state contract；
- generic DSP graph / scripting；
- GPU 粒子系统、复杂 skin、多主题；
- 为假设未来提前建立的大型继承层次。

P2 只有在用户研究/真实声音问题证明价值、且通过新的 ADR 和 milestone 重排后才能进入 v1。

## 14. 风险登记与触发动作

| 风险 | 早期信号 | 缓解/触发动作 |
|---|---|---|
| 本地可构建但 CI 不可复现 | F: 绝对路径、external/JUCE 缺失 | M0 阻断功能开发；Accept ADR-0004 |
| 参数兼容性提前冻结错误 | `.enable`/`.enabled` 并存 | M1 首个 PR 锁定 registry 与测试 |
| Water/Ice 变成加样本拟音 | dry input 被掩盖、算法只触发 one-shot | perceptual brief 强制“输入驱动/可辨识”；Water 在正常设置及代表性 `global.mix=100%` 下单独验收 source recognizability |
| Water residual ownership 模糊 | Resonant 已含 direct feedthrough，上层仍执行 `x + output` | 每个 sub-engine 声明 residual/complete-signal semantics；ADR 和 unit/render 证据阻断重复 carrier、增益抬升及 comb artifact |
| Water macro 语义交叉 | Size 改变事件密度、Motion 主要改变 loudness/Amount | 强制完整 perceptual -> Mapper -> engine -> DSP -> audible -> validation chain；单调性、响度补偿和 mode consistency 由实验验证 |
| routing 侵入模块 | Processor 出现 setParallelMode | review 阻断，移至 RoutingEngine |
| automation click | 快速 ramp/离散切换爆峰 | 专门 stress render + transition policy |
| 随机 DSP 无法回归 | 相同配置每次输出不可比较 | injectable fixed seed |
| UI history 污染 Host automation | parameterChanged 全部 push | 只从 UI gesture/command 创建 transaction |
| 性能预算过晚 | vertical slice 已复杂且无法降级 | M2/M3 每 slice 报 avg/peak，再集成 |
| reference 音频版权/仓库膨胀 | 来源不明、大 WAV 入 Git | manifest/许可审计，artifact/LFS 决策 |
| routing state ownership 不明确 | transition 同 block 推进同一 Processor 两次、random/tail 不一致 | `ROUTE-006` 阻断至 `ADR-R-001` Accepted；比较双 graph 与两阶段 transition |
| routing infrastructure 引入未预期 latency | identity impulse 被平移、Parallel/Serial branch 错位、Global Mix 隐藏 delay | `ARCH-LAT-001` 要求 Host-reported latency=0；`ROUTE-011` 验收 infrastructure；intentional effect delay/tail 由算法 ADR/tests 描述 |
| production random 同步 | 多实例输出完全相同或 save/reopen 语义不明 | 区分测试 fixed seed 与 production instance seed；由 Water/Ice ADR 定义 persistent/offline 语义 |

## 15. Bootstrap Issue Map

本节是稳定 ID 到 GitHub Issue 的映射规则，不是实时 todo list。Issue 的实际状态、assignee、board column 和是否已创建只在 GitHub 维护；如果某个 ID 已有 issue，不得重复创建。

标题格式统一为：`[<milestone>][<stable-id>] <short action>`。初次初始化时至少应按依赖创建以下入口；它们的完成状态不在本文件重复维护：

| Milestone | Stable ID | Issue title 示例 | 依赖 |
|---|---|---|---|
| M0 | REPO-001 / ENG-STD-001 / LEGAL-001 / DEP-001 | `[M0][REPO-001] Initialize repository and audit first tracked tree` | owner authorization |
| M0 | BUILD-001 / BUILD-002 / TEST-001 | `[M0][BUILD-001] Add portable Windows CI presets without F-drive paths` | DEP-001 |
| M0 | HOST-000 / CI-001 | `[M0][HOST-000] Freeze initial platform and DAW compatibility matrix` | REPO-001 |
| M0 | GH-001 / GH-002 | `[M0][GH-001] Create repository labels, milestones and project board` | REPO-001 / CI-001 |
| M1 | ARCH-001 / PARAM-001 / PARAM-002 / PARAM-003 | `[M1][PARAM-001] Extract ParameterLayout and lock core parameter IDs` | HOST-000 |
| M1 | AUTO-001 / TESTDATA-001 / PERF-BASE-001 / ARCH-LAT-001 | `[M1][PERF-BASE-001] Establish realtime performance baseline` | M1 contract types |
| M1 | APP-001 / RENDER-001 / HOST-001 | `[M1][APP-001] Route EngineParameters through AudioEngine` | PARAM-002/003 |
| M2/M3 | EXP-W-* / EXP-I-* / WATER-* / ICE-* | `[M2][EXP-W-001] Define Water dual-mode perceptual brief` | TESTDATA-001 |
| M4 | PARAM-FREEZE-001 / ADR-R-001 | `[M4][PARAM-FREEZE-001] Freeze v1 host parameter contract` | M2 + M3 exit gates |
| M4 | ROUTE-001..011 | `[M4][ROUTE-006] Implement routing transition policy` | ADR-R-001 |

M0 项未达到 exit gate 前，Water/Ice 只允许在 `experiments/` 探索，不进入 production target。
