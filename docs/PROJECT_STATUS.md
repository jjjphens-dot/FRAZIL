# FRAZIL 当前实现与差距

> 快照日期：2026-09-07<br>
> 依据：本地源码/构建目录审计、当前 Debug/portable build/CTest、GitHub 仓库页面与 Actions 页面。<br>
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## Modification Policy

本文件属于 STATUS/EVIDENCE 文档，只能记录实际审计、测试、CI、兼容性或听测结果。计划、假设和未执行工作必须明确标为 planned/未验证，不得把本文件当作稳定计划合同。

## 1. 结论

项目已有可构建的 JUCE M0 骨架；M1-A/M1-B 的参数、Snapshot/Mapper、Application-DSP 接口和基础 gain signal path 已随 PR #2 / PR #3 合入 `main`。M1 仍在进行中；Water/Ice、Routing、版本化 state、render 和正式 UI 仍未实现。

`CODING_PLAN.md` v1.0 / Approved Development Baseline 已作为 M0→M7 的正式工程执行基线；这不代表 FRAZIL plugin v1.0 release，也不改变 M0/M1、Water/Ice/Routing 的实际完成状态。

当前阻塞性差距：

1. 本次状态快照记录的 observed `main` HEAD 为 `db02d1c`；PR #3 merge commit 为 `229ca19`，M1-A/M1-B foundation 已合入 `main`；`feat/m1-parameter-engine-contract` 保留为历史 feature 分支，下一开发入口为独立的 M1-C state 分支；本状态文件后续提交只表达 snapshot 与 merge/current-state relationship，不把其自身之前的提交长期称为 current HEAD；
2. PR #3 最新 HEAD 的 [Hosted CI run 34044332388](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34044332388) 已完成 Windows Debug configure/build/test 并通过；PR #2 的 [Hosted CI run 34022773758](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34022773758) 也已通过；workflow/API 的更细粒度权限与 branch protection 未验证；
3. `water.enable`/`ice.enable` 与架构目标的 ID 冲突已在合入 `main` 的集中式 ParameterLayout 中修正为 `water.enabled`/`ice.enabled`；兼容迁移策略仍属于 STATE/公开版本前置工作；
4. APVTS 参数已通过一次 block Snapshot 和 ParameterMapper 进入 AudioEngine；当前 wet path 仍为 post-input pass-through；
5. CTest 已覆盖参数枚举、Snapshot、Mapper、mix、smoothing、RandomSource、gain staging、first-block priming、reset、zero-length 和 runtime buffer invariant；M1-A/M1-B 合并前的 Debug VST3 pluginval strictness 5 已通过，尚未覆盖版本化 state、render 和 DAW automation；
6. Water、Ice、Routing、版本化 StateModel、EditHistoryManager、离线 render、DSP property/performance harness、离散 enable/routing transition 和正式 UI 仍未实现。
7. PR #2 与 PR #3 均已合入 `main`；`87fd69b docs: add two-person collaboration roles` 作为协作基线保留在历史中，未为追求历史美观而重写 feature 分支；
8. PR #3 collaborator review was not preserved as a formal GitHub Review submission；这是 process evidence gap，不是 production implementation bug；从下一条需要双人 review 的核心 PR 开始，必须留下 formal review 或满足治理规则的第二位开发者 comment evidence；
9. MIT `LICENSE` 已加入；第三方 notice 策略仍待收口；
10. GitHub Issues 全状态筛选无结果，Milestones 为 0，Projects 为 0；Labels 页面仅见 GitHub 默认标签，项目自定义 labels 未建立；branch protection 未验证。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.22+、C++20、Ninja presets | 本机可用；portable preset 已由 Hosted CI 验证 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、state XML round-trip | 骨架 |
| Parameters | 9 个集中式 APVTS 参数静态注册；enabled ID 已使用 `.enabled` | M1 参数路径已接入；合同仍待 freeze |
| App | `ProcessSpec`、`EngineParameters`、`ParameterSnapshot`、`ParameterMapper`、`AudioEngine::prepare/reset/process` | M1 gain/mix skeleton；first-block priming；wet pass-through；runtime buffer invariant fallback |
| UI | 640x360 M0 占位界面 | 非产品 UI |
| Tests | `frazil_smoke` + `frazil_tests` CTest | M1 contract/gain/smoothing/priming/invariant unit 覆盖；state/DSP property/Host 测试未完成 |
| Local validation | `feat/m1-parameter-engine-contract` merge-candidate 的 Debug、Release、ASAN 均 configure/build；三个 preset 的 CTest 均 2/2 PASS | 已验证 |
| pluginval | M1-A/M1-B merge-candidate Debug VST3 strictness 5 `SUCCESS`；Steinberg validator 因未配置而跳过 | 已验证（不等于独立 VST3 validator） |
| Remote | `jjjphens-dot/FRAZIL` public repository；`origin` 已绑定；snapshot observed `main` HEAD 为 `db02d1c`，PR #3 merge commit 为 `229ca19` | PR #2 / PR #3 Hosted CI success；Issues/Milestones/Projects metadata 未建立 |

## 3. 当前源码映射

```text
PluginProcessor
  ├─ owns APVTS
  ├─ uses src/plugin/ParameterLayout for static parameters
  ├─ caches APVTS raw parameter atomics
  ├─ captures one ParameterSnapshot per processBlock
  ├─ maps to EngineParameters
  ├─ saves/restores APVTS XML
  └─ calls AudioEngine::process(buffer, engineParameters)

AudioEngine
  ├─ input gain -> dry reference/wet skeleton -> global mix -> output gain
  └─ owns prepare-time dry scratch and continuous smoothers

PluginEditor
  └─ static M0 label
```

目标映射应在 M1 后变为：

```text
PluginProcessor
  ├─ setup: `src/plugin/ParameterLayout.*` -> APVTS / Host Parameter Registry
  ├─ state: Host State Adapter -> StateModel
  └─ audio runtime: Host Parameter Atomics
                       ↓
                 ParameterSnapshot
                       ↓
                 ParameterMapper
                       ↓
                 EngineParameters
                       ↓
                   AudioEngine
  ├─ Input Gain
  ├─ RoutingEngine -> Water / Ice / StageMixer
  ├─ Global Mix
  └─ Output Gain
```

## 3.1 M0 code quality audit

本次代码审计只记录当前源码事实，不把计划模块当成实现：

- 正向事实：`src/plugin`、`src/app`、`src/dsp`、`src/ui` 目录边界已经存在；当前未发现 mutable global runtime state；AudioEngine 的运行状态由实例成员持有；`JuceHeader.h` 目前局限在插件适配层。
- 已确认技术债：`PluginProcessor` 仍公开 APVTS，后续需要收窄 Host parameter interface；state 仍是未版本化的 XML skeleton；wet path 仍为 pass-through，Water/Ice/Routing 尚未实现。
- 有意保留的未实现项：Water、Ice、Routing、版本化 StateModel、EditHistoryManager、正式 UI、render/property/performance harness 和离散 transition 均仍按 Coding Plan 处于计划阶段；本次修复不提前创建生产依赖。
- 当前验证边界：CTest 仅提供当前列出的 unit/lifecycle/invariant 证据，不能据此宣称参数 automation、state compatibility、DSP property/render、DAW 或完整 realtime safety 已完成。

## 4. 参数差异审计

| 语义 | 当前代码 | 目标合同 | 动作 |
|---|---|---|---|
| Water enable | M0 历史 `water.enable` | `water.enabled` | 合入 `main` 的集中式 ParameterLayout 已迁移；首个公开版本前仍需 state compatibility/migration 证据 |
| Ice enable | M0 历史 `ice.enable` | `ice.enabled` | 同上 |
| Routing | `routing.mode` | `routing.mode` | 保留 |
| Parallel balance | `parallel.balance` | `parallel.balance` | 保留 |
| Water stage amount | `water.amount` | `water.amount` | 保留 |
| Ice stage amount | `ice.amount` | `ice.amount` | 保留 |
| Input trim | `input.gain` | `input.gain` | 保留；基础 DSP/continuous smoothing 已接入；仍待 automation/state/Host validation |
| Global dry/wet | `global.mix` | `global.mix` | 保留；基础 DSP/continuous smoothing 已接入；仍待 automation/state/Host validation |
| Output trim | `output.gain` | `output.gain` | 保留；基础 DSP/continuous smoothing 已接入；仍待 automation/state/Host validation |

虽然 M1 参数合同已随 PR #3 合入 `main`，但在正式参数 freeze、state migration 和 compatibility evidence 前，不得创建公开 preset/session 兼容性承诺。若已有外部用户使用过当前占位构建，应先确认是否需要兼容别名/迁移。

## 5. 现状对应 milestone

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证、首次 push 和两次 Hosted CI success 已验证；HOST-000、GitHub metadata 与 branch protection 尚未收口。
- M1 Audio Skeleton & Parameter Contract：**进行中**。M1-A/M1-B foundation 已合入 `main`：ParameterLayout、Snapshot、Mapper、ProcessSpec、EngineParameters、Input/Output gain skeleton、Global DryWet primitive、continuous smoothing、RandomSource 和 Host -> Snapshot -> Mapper -> AudioEngine 路径已实现；仍缺版本化 state、automation integration、render/property/performance、DAW 验证和正式参数 freeze，下一入口为 M1-C。
- M2 Water：**未开始**。
- M3 Ice：**未开始**。
- M4 Routing：**未开始**。
- M5 UI & Edit History：**未开始**。
- M6 Beta Hardening：**未开始**。
- M7 v1.0 Release：**未开始**。

## 6. 不应从现状推断的结论

- 参数“能被 APVTS 注册”不等于 automation click-free 或 DSP 已消费参数；
- state XML 能 round-trip 不等于跨版本兼容策略已建立；
- pluginval 历史通过不等于真实 DAW matrix 已通过；
- pass-through 能构建不等于 routing/gain/dry-wet 的数学和增益结构正确；
- 目录 README 存在不等于对应模块已经实现；
- 本地 `external/JUCE` 存在不等于 fresh clone 可复现。

## 7. 下一步唯一推荐入口

按 `docs/CODING_PLAN.md` 继续收口 M1-C：完成版本化 StateModel、automation integration、TESTDATA/RENDER/PERF harness 和 HOST-001 证据；在这些基础合同与验证就绪前，不进入 Water/Ice/Routing 生产实现。
