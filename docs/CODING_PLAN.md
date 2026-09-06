# FRAZIL 分阶段 Coding Plan

> 版本：1.0-draft  
> 日期：2026-09-06  
> 输入：`FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` + 当前源码/构建/远端审计  
> 目标：把产品语义转化为可排序、可分工、可验收、可在 GitHub 追踪的工程工作。

## 1. 计划使用方式

本计划中的每一行工作项都应成为一个 GitHub issue，稳定 ID 写入 issue title、branch、PR 和 changelog。状态只允许：Backlog、Ready、In Progress、Review、Validation、Done、Blocked。

一个工作项只有在代码/文档、自动测试、必要听测/DAW 证据和 review 全部满足后才是 Done。`[x]` 只表示本地审计确认，不表示已在 GitHub 关闭。

### 文档职责边界

`CODING_PLAN.md` 是稳定的工程计划合同，只定义 milestone、工作项、依赖、优先级、交付物、验收标准、exit gate 以及架构/流程约束。它不维护某个 issue 的实时状态，也不替代 GitHub Issues 或 Project。

`docs/PROJECT_STATUS.md` 记录当前 milestone、已验证能力、blocker、验证结果和下一步建议；GitHub Issues/Project 是单个工作项实时状态的唯一来源。计划中的工作项即使已经在本地或远端完成，也保留在这里作为稳定定义，不在本文件维护动态 todo 清单。

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

M2 Water --------+
                 |
M3 Ice ----------+
                 |
                 v

PARAM-FREEZE-001
v1 Host Parameter Contract Freeze
                 |
                 v

ADR-R-001
Routing Transition / DSP State Strategy
                 |
                 v

M4
Routing / Latency / Gain Integration
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
```

M2 Water 与 M3 Ice 仍可并行开发。`PARAM-FREEZE-001` 必须在 M2/M3 完成后、M4 开始前完成；`ADR-R-001` 必须在任何 `ROUTE-006` 实现前 Accepted。任何 Water/Ice 生产实现都依赖 M1 的 ProcessSpec、EngineParameters、Snapshot、统一测试素材和性能 baseline。

## 3. 跨模块完成合同

| 模块 | 必须提供 | 明确不负责 | 核心验收 |
|---|---|---|---|
| PluginProcessor | JUCE lifecycle、bus、Snapshot 入口、state adapter | Water/Ice 算法、routing math、UI layout | pluginval、mono/stereo、state/parameter tests |
| ParameterLayout | 静态 ID/type/range/default/label | DSP mapping、显隐 | 精确枚举 regression |
| ParameterSnapshot | 每 block 一致的 POD 值 | smoothing、APVTS ownership | 无分配、一致性测试 |
| ParameterMapper | user -> engine 语义、clamp、enum | buffer、Host timeline | table-driven unit tests |
| AudioEngine | gain/routing/global mix 编排与生命周期 | 算法细节、UI/history | signal-chain/integration tests |
| WaterProcessor | 完整 Water transform 与少量 macro | stage amount、routing、Host | property/render/listening/perf |
| IceProcessor | 完整 Ice transform 与少量 macro | stage amount、routing、Host | property/render/listening/perf |
| RoutingEngine | Parallel/两个 Serial、enable、transition | Water/Ice 内部算法 | routing matrix、click-free、retention |
| StageMixer | dry/processed mix law | Host/UI | endpoint/monotonicity/energy tests |
| EditHistoryManager | UI transaction、bounded undo/redo | Host automation、audio thread | gesture/source/state tests |
| UI | 产品参数表达、attachment、gesture | DSP 执行、动态参数注册 | interaction/resize/automation tests |

## 4. M0 — Repository, Governance & Reproducible Build

### 目标

从“本机可构建目录”变成“fresh clone 可复现、可以安全协作、CI 可作为合并门槛”的仓库。此阶段不开发声音功能。

### 工作包

| ID | P | 工作内容 | 交付物 | 验收标准/证据 | 依赖 |
|---|---:|---|---|---|---|
| REPO-001 | P0 | 初始化 `main`、关联 `origin`、审计首次提交 | Git history + remote main | `git status` clean；remote 正确；忽略项未入库；GitHub 可 fresh clone | owner 授权 push |
| DOC-001 | P0 | 纳入架构总纲、plan、status、parameters、testing、workflow、ADR、贡献规范 | `docs/` + AGENTS/README/templates | 文档链接有效；现状与目标分离；review 通过 | 无 |
| LEGAL-001 | P0 | 由 owner 选择并加入开源许可证 | `LICENSE` + README 声明 | 许可证与第三方 notices 一致；提交音频许可可追溯 | owner 产品决定 |
| DEP-001 | P0 | 决定 JUCE 固定与补丁策略 | Accepted ADR-0004 | fresh directory 获取精确 JUCE revision；补丁可审计；不依赖 C: 或临时文件 | REPO-001 |
| BUILD-001 | P0 | 分离本地 F: preset 与 portable CI preset | presets/toolchain/CMake | 本地三 preset 不退化；GitHub runner 不引用 F: 绝对路径 | DEP-001 |
| BUILD-002 | P0 | 建立依赖 bootstrap/checksum | script + docs | 空 `external/` 可按固定版本恢复；重复执行幂等；失败信息清晰 | DEP-001 |
| TEST-001 | P0 | 接入可扩展单元测试框架 | `frazil_tests` target | CTest 能发现多个 case；失败返回非零；不依赖 plugin GUI | BUILD-001 |
| CI-001 | P0 | Windows PR workflow | `.github/workflows/ci.yml` | clean checkout Debug configure/build/test PASS；最小权限；缓存失效规则正确 | BUILD-001/2, TEST-001 |
| CI-002 | P1 | Release/ASAN/pluginval scheduled jobs | workflow jobs | 可手动触发；artifact/log 可追踪 commit；失败可诊断 | CI-001 |
| HOST-000 | P0 | Freeze initial platform and DAW compatibility matrix | 支持矩阵 ADR/文档 | 记录 v1 正式支持、开发验证、best-effort 的 OS/架构/格式/DAW/Standalone 角色；目标 DAW 可用于后续 M1 smoke | REPO-001 |
| GH-001 | P0 | 创建 labels、M0-M7 milestones、Project board | GitHub metadata | issue 可按 type/area/priority/milestone 查询 | REPO-001 |
| GH-002 | P0 | 配置 main protection | ruleset | PR、1 approval、CI、resolved conversations、no force push | CI-001 稳定 |
| TOOL-001 | P1 | 增加 formatting check | clang-format target/job | 与 `.clang-format` 一致；不自动改 vendor/generated | CI-001 |

### 实施顺序

1. DOC-001、LEGAL-001 与首次提交审计；
2. REPO-001；
3. DEP-001 -> BUILD-001/002；
4. HOST-000；
5. TEST-001 -> CI-001；
6. GH-001/002；
7. CI-002/TOOL-001。

### Exit gate

```text
fresh clone
-> obtain pinned dependencies
-> configure portable Debug
-> build FRAZIL_All + tests
-> CTest PASS
-> CI required check PASS
-> HOST-000 matrix recorded
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
| PARAM-001 | P0 | 提取 `ParameterLayout.*` | 集中注册 9 个核心参数；enabled ID 改为 `.enabled`；固定顺序/choice | 精确 contract test |
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
| DSP-005 | P0 | RandomSource contract | 测试支持 deterministic fixed seed；生产实例必须 decorrelated；无全局可变状态；未来 ADR 决定 save/reopen、offline render、实时播放和 transition 时的 random state 语义 | exact sequence/reseed/instance-isolation tests |

M1 的 wet path 可暂时等于 post-input pass-through，以单独验证 gain/global mix。此时 `global.mix` 不应改变声音，因为 dry/wet 相同；测试需解释这一点，不能误判为参数未接入。

#### M1-C：State、history 骨架与工具

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| STATE-001 | P0 | versioned StateModel/adapter | `schemaVersion`、全部参数、invalid input fallback；非音频线程迁移 | fixtures + round-trip |
| STATE-002 | P0 | mode value retention | 切换三种 mode、保存、恢复，不重置 inactive values | integration scenario |
| HIST-001 | P0 | EditHistoryManager skeleton | bounded transaction store/API；message thread only；暂不要求完整 UI | unit tests for capacity/clear |
| TESTDATA-001 | P0 | Establish licensed reference audio corpus | impulse、noise、drums、vocal、piano、guitar、pad、bass；每项记录 source/license/hash/sample rate/channels/storage/repository-artifact-LFS 策略；Water/Ice 共用 corpus | manifest review + hash/licence audit |
| PERF-BASE-001 | P0 | Establish realtime performance baseline | Reference Machine、OS、compiler、build type、48 kHz/128、测量方法；记录 mean/P95/P99/worst callback、deadline、memory、allocation observation | reproducible baseline report；不预设百分比阈值 |
| ARCH-LAT-001 | P0 | Define v1 latency model and dry/wet alignment | v1 Host-reported processing latency 为 0 samples；不依赖 lookahead、FFT block latency、linear-phase、convolution 或 Host PDC；Water/Ice 允许属于声音设计的 intentional effect delay/tail，但不得依赖 Host latency compensation | ADR + latency metadata/infrastructure acceptance |
| RENDER-001 | P0 | offline WAV harness | 固定 `TESTDATA-001` input/config/seed -> WAV + manifest；不依赖实时设备；candidate A/B 可复现 | deterministic smoke render |
| TEST-002 | P0 | processor property harness | finite/random/extreme/prepare-reset；矩阵参数化 | 44.1/48/96 + block subset |
| HOST-001 | P0 | pluginval + Host smoke protocol | 枚举/state/editor/bus/automation；结果可追踪 SHA；DAW 目标来自 HOST-000 | strictness 5 PASS |

### M1 Exit gate

- Parameter registry 与 `docs/PARAMETERS.md` 完全一致；
- AudioEngine 实际消费 Snapshot/EngineParameters；
- gain/global mix 位置与数学由 tests 证明；
- state version、round-trip、inactive retention 和 invalid state 通过；
- process path 审查无 I/O/lock/allocation/history/UI；
- `AUTO-001` 的 block snapshot、sample-aware smoothing 和离散 transition 语义有测试证据；
- `TESTDATA-001` manifest/hash/license 可追溯，`RENDER-001` 使用统一 corpus；
- `PERF-BASE-001` 已记录 baseline，不以未测量的 CPU 百分比作为门槛；
- `ARCH-LAT-001` 已接受，routing/mixing infrastructure 不引入未声明 processing latency，v1 Host-reported latency 为 0 samples；Water/Ice intentional effect delay/tail 由各自 ADR/tests 描述；
- offline render 可复现，property tests 无 NaN/Inf；
- Debug/Release/ASAN + pluginval PASS；
- `HOST-000` 中定义的 primary target DAW 完成参数枚举与 project save/reopen smoke。

## 6. M2 — Water Vertical Slice

### 目标

产出第一个可被音乐素材驱动、可辨识为 Water、保持输入可辨识度且实时安全的 production vertical slice。不是加入水声采样层。

### 研究波次

| ID | P | 工作 | 交付/验收 |
|---|---:|---|---|
| EXP-W-001 | P0 | Water perceptual brief | 3-5 个可听属性（流动、液体扰动/水滴、共振、平滑度等）、反例、参考素材与评价表 |
| EXP-W-002 | P0 | 候选机制实验 | 使用 `TESTDATA-001`；至少两种低耦合候选；固定测试 seed；baseline/A/B；参数空间与 CPU 初测 |
| EXP-W-003 | P0 | 选择 vertical slice | 使用统一 listening rubric 做双人 loudness-matched review；选择理由、放弃理由、风险；确定最多 2 个首批 macro |
| ADR-W-001 | P0 | Water 算法 ADR | 信号结构、latency/tail、随机性、参数 mapping、性能与失败模式 |

候选可探索 FlowModulator、DropletExciter、LiquidResonator、SpectralShaper 或 MicroDelayNetwork，但名称不是实现要求；以听感、稳定性和预算决定。

### 生产实现

| ID | P | 模块/工作 | 具体要求 | 验收 |
|---|---:|---|---|---|
| WATER-001 | P0 | `WaterProcessor` lifecycle | `prepare/reset/process(buffer, WaterParameters)`；mono/stereo；无 routing/amount/APVTS | lifecycle/property tests |
| WATER-002 | P0 | Water core | 实现已选最小算法；固定 seed 可复现；处理参数极值 | render + finite tests |
| WATER-003 | P0 | product macros | 名称体现用户听感；mapping 集中；自动化平滑；文档/parameter registry | mapping/automation tests |
| WATER-004 | P0 | click-free enable | disabled=pass-through；重新开启保留 macro；过渡无异常峰值 | transient automation render |
| WATER-005 | P0 | performance/tail | 相对 `PERF-BASE-001` 的 48k/128 baseline 报告 mean/P95/P99/worst；不引入非零 Host-reported processing latency；intentional effect delay/tail 由 Water ADR/tests 描述 | benchmark + plugin metadata |
| WATER-006 | P0 | listening pack | 使用 `TESTDATA-001` 的多类素材 dry/baseline/candidate + manifest；按 Water rubric 记录 | rubric accepted；未触发 Reject Criteria |
| WATER-007 | P0 | integration | AudioEngine 的 Water-only 临时路径，不引入 routing 语义 | pluginval + DAW automation |

### M2 Exit gate

Water-only 在 `TESTDATA-001` 固定素材上具有一致可辨识的材质变化，输入仍可辨识；所有宏符合 `AUTO-001` 且无明显 zipper；通过 Water listening rubric 和 Reject Criteria；bypass/state/seed/render/property/performance/pluginval 通过；WaterProcessor 未依赖 Host、UI 或 RoutingMode。

## 7. M3 — Ice Vertical Slice

### 目标

产出可被输入驱动、可辨识为 Ice、与 Water 听感和算法机制足够区分的 production vertical slice。

### 研究与实现

| ID | P | 工作 | 具体要求/验收 |
|---|---:|---|---|
| EXP-I-001 | P0 | Ice perceptual brief | 冰晶/摩擦/脆裂/硬度等属性、反例、参考与评价表 |
| EXP-I-002 | P0 | 候选机制实验 | 使用 `TESTDATA-001`；至少两候选；固定测试 seed；A/B；CPU 和极端参数 |
| EXP-I-003 | P0 | vertical slice selection | 按 Ice listening rubric 双人听测；最多 2 个首批 macro；风险和弃选记录 |
| ADR-I-001 | P0 | Ice 算法 ADR | 结构、transient/random、latency/tail、mapping、预算 |
| ICE-001 | P0 | `IceProcessor` lifecycle | 与 Water 接口习惯一致但不强求内部对称；无 routing/APVTS |
| ICE-002 | P0 | Ice core | 选定的摩擦/晶体/裂纹机制最小组合；finite/repeatable |
| ICE-003 | P0 | product macros | 用户语义、mapping、smoothing、automation/state |
| ICE-004 | P0 | enable transition | pass-through、value retention、click-free |
| ICE-005 | P0 | performance/tail | 相对 `PERF-BASE-001` 的 Reference baseline 报告 mean/P95/P99/worst；不引入非零 Host-reported processing latency；intentional effect delay/tail 由 Ice ADR/tests 描述 |
| ICE-006 | P0 | listening pack | 使用 `TESTDATA-001`，跨素材且与 Water 可区分；按 Ice rubric 记录 |
| ICE-007 | P0 | integration | Ice-only AudioEngine 路径 + pluginval/DAW smoke |

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
| UI-005 | P0 | module/gain controls | enabled、Water/Ice macros、Input/Global Mix/Output；0 dB reset | attachment/default tests |
| HIST-002 | P0 | gesture transactions | mouseDown/begin -> changes -> mouseUp/end 为一步；离散操作一步 | undo boundary tests |
| HIST-003 | P0 | Undo/Redo controls | disabled state、redo branch、action label；不注册参数 | interaction tests |
| HIST-004 | P0 | source isolation | Host automation/restore/init/smoothing 不入栈；restore 清栈 | integration tests |
| UI-006 | P0 | tooltip/value formatting | 百分比/dB/choice 一致；说明 inactive mode | snapshot/manual review |
| UI-007 | P1 | keyboard/accessibility | Ctrl/Cmd+Z、Redo、focus order、可读对比度/label | platform manual test |
| UI-008 | P1 | meters/flow hint | 不阻塞音频线程；数据桥接无锁/有界 | performance + visual review |

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

只在接口冻结后并行，团队 WIP <= 2：

| 波次 | Developer A（声音/DSP primary） | Developer B（plugin/architecture/tooling primary） | 合流点 |
|---|---|---|---|
| M0 | reference/listening protocol | Git/CI/dependency/test framework | fresh clone gate |
| M1 | DSP primitive/gain/property cases | Parameter/Snapshot/state/render harness | Engine contract gate |
| M2/M3 | Water vertical slice | Ice vertical slice（或反之） | 双人交叉 review，各自接口一致 |
| M4 | routing sound/loudness A/B | RoutingEngine/state/automation integration | full matrix gate |
| M5 | material control UX/audio feedback | attachments/history/layout | UI acceptance |
| M6 | listening regression/perf tuning | validators/DAW/state/fuzz | beta sign-off |

每个 primary 的 PR 由另一人 review。参数 ID、routing、声音方向与 performance budget 不属于单人私有知识。

## 13. 明确推迟到 P2

- 插件内部 Water -> Ice morph timeline、sequencer、automation recorder；
- AAX、AU（除非 v1 目标正式变更）；
- preset cloud、用户账户、在线服务；
- modulation matrix、多频段、MIDI modulation；
- generic DSP graph / scripting；
- GPU 粒子系统、复杂 skin、多主题；
- 为假设未来提前建立的大型继承层次。

P2 只有在用户研究/真实声音问题证明价值、且通过新的 ADR 和 milestone 重排后才能进入 v1。

## 14. 风险登记与触发动作

| 风险 | 早期信号 | 缓解/触发动作 |
|---|---|---|
| 本地可构建但 CI 不可复现 | F: 绝对路径、external/JUCE 缺失 | M0 阻断功能开发；Accept ADR-0004 |
| 参数兼容性提前冻结错误 | `.enable`/`.enabled` 并存 | M1 首个 PR 锁定 registry 与测试 |
| Water/Ice 变成加样本拟音 | dry input 被掩盖、算法只触发 one-shot | perceptual brief 强制“输入驱动/可辨识” |
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
| M0 | REPO-001 / LEGAL-001 / DEP-001 | `[M0][REPO-001] Initialize repository and audit first tracked tree` | owner authorization |
| M0 | BUILD-001 / BUILD-002 / TEST-001 | `[M0][BUILD-001] Add portable Windows CI presets without F-drive paths` | DEP-001 |
| M0 | HOST-000 / CI-001 | `[M0][HOST-000] Freeze initial platform and DAW compatibility matrix` | REPO-001 |
| M0 | GH-001 / GH-002 | `[M0][GH-001] Create repository labels, milestones and project board` | REPO-001 / CI-001 |
| M1 | ARCH-001 / PARAM-001 / PARAM-002 / PARAM-003 | `[M1][PARAM-001] Extract ParameterLayout and lock core parameter IDs` | HOST-000 |
| M1 | AUTO-001 / TESTDATA-001 / PERF-BASE-001 / ARCH-LAT-001 | `[M1][PERF-BASE-001] Establish realtime performance baseline` | M1 contract types |
| M1 | APP-001 / RENDER-001 / HOST-001 | `[M1][APP-001] Route EngineParameters through AudioEngine` | PARAM-002/003 |
| M2/M3 | EXP-W-* / EXP-I-* / WATER-* / ICE-* | `[M2][EXP-W-001] Define Water perceptual brief` | TESTDATA-001 |
| M4 | PARAM-FREEZE-001 / ADR-R-001 | `[M4][PARAM-FREEZE-001] Freeze v1 host parameter contract` | M2 + M3 exit gates |
| M4 | ROUTE-001..011 | `[M4][ROUTE-006] Implement routing transition policy` | ADR-R-001 |

M0 项未达到 exit gate 前，Water/Ice 只允许在 `experiments/` 探索，不进入 production target。
