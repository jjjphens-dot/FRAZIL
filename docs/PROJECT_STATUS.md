# FRAZIL 当前实现与差距

> 快照日期：2026-09-06  
> 依据：本地源码/构建目录审计、当前 Debug/portable build/CTest、GitHub 仓库页面与 Actions 页面。<br>
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## Modification Policy

本文件属于 STATUS/EVIDENCE 文档，只能记录实际审计、测试、CI、兼容性或听测结果。计划、假设和未执行工作必须明确标为 planned/未验证，不得把本文件当作稳定计划合同。

## 1. 结论

项目已有可构建的 JUCE M0 骨架；本开发分支已开始收口 M1 的参数、Snapshot/Mapper、Application-DSP 接口和基础 gain signal path。首次仓库接入和 Hosted CI 已验证；Water/Ice、Routing、版本化 state、render 和正式 UI 仍未实现。

`CODING_PLAN.md` v1.0 / Approved Development Baseline 已作为 M0→M7 的正式工程执行基线；这不代表 FRAZIL plugin v1.0 release，也不改变 M0/M1、Water/Ice/Routing 的实际完成状态。

当前阻塞性差距：

1. 本次状态快照在 `feat/m1-parameter-engine-contract` 开发分支上；该分支从已审计的文档基线提交继续开发，提交与远端发布结果以本次 PR 记录为准；
2. GitHub Actions 页面显示 `FRAZIL CI` 的 run `34014018189`（`96b3659`）和 run `34014110580`（`1f4bb67`）均 completed successfully；workflow/API 的更细粒度权限与 branch protection 未验证；
3. `water.enable`/`ice.enable` 与架构目标的 ID 冲突已在本分支的集中式 ParameterLayout 中修正为 `water.enabled`/`ice.enabled`；兼容迁移策略仍属于 STATE/公开版本前置工作；
4. APVTS 参数已通过一次 block Snapshot 和 ParameterMapper 进入 AudioEngine；当前 wet path 仍为 post-input pass-through；
5. CTest 已覆盖参数枚举、Snapshot、Mapper、mix、smoothing、RandomSource、gain staging 和 lifecycle；尚未覆盖版本化 state、render、DAW automation 和本次变更后的 pluginval；
6. Water、Ice、Routing、gain processing、smoothing、Undo/Redo 和正式 UI 均未实现。
7. MIT `LICENSE` 已加入；第三方 notice 策略仍待收口；
8. GitHub Issues 全状态筛选无结果，Milestones 为 0，Projects 为 0；Labels 页面仅见 GitHub 默认标签，项目自定义 labels 未建立；branch protection 未验证。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.22+、C++20、Ninja presets | 本机可用；portable preset 已由 Hosted CI 验证 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、state XML round-trip | 骨架 |
| Parameters | 9 个集中式 APVTS 参数静态注册；enabled ID 已使用 `.enabled` | M1 参数路径已接入；合同仍待 freeze |
| App | `ProcessSpec`、`ParameterSnapshot`、`ParameterMapper`、`AudioEngine::prepare/reset/process` | M1 gain/mix skeleton；wet pass-through |
| UI | 640x360 M0 占位界面 | 非产品 UI |
| Tests | `frazil_smoke` + `frazil_tests` CTest | M1 contract/gain primitive unit 覆盖；state/DSP property/Host 测试未完成 |
| Local validation | 本分支 Debug、Release、ASAN 均 configure/build；三个 preset 的 CTest 均 2/2 PASS | 已验证 |
| pluginval | 本分支 Debug VST3 strictness 5 `SUCCESS`；Steinberg validator 因未配置而跳过 | 已验证（不等于独立 VST3 validator） |
| Remote | `jjjphens-dot/FRAZIL` public repository；`origin` 已绑定，本次审计的 `main` 基线为 `1f4bb67` | GitHub Actions 两次 run success；Issues/Milestones/Projects metadata 未建立 |

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
- 有意保留的未实现项：Water、Ice、Routing、真实 gain/mix、Snapshot/Mapper、正式 UI 和 EditHistoryManager 均仍按 Coding Plan 处于计划阶段；本次治理任务不提前创建生产依赖。
- 当前验证边界：已有 smoke/lifecycle 和 AudioEngine unit 证据，不能据此宣称参数 automation、DSP property/render、DAW 或完整 realtime safety 已完成。

## 4. 参数差异审计

| 语义 | 当前代码 | 目标合同 | 动作 |
|---|---|---|---|
| Water enable | M0 历史 `water.enable` | `water.enabled` | 本分支已迁移；首个公开版本前仍需 state compatibility/migration 证据 |
| Ice enable | M0 历史 `ice.enable` | `ice.enabled` | 同上 |
| Routing | `routing.mode` | `routing.mode` | 保留 |
| Parallel balance | `parallel.balance` | `parallel.balance` | 保留 |
| Water stage amount | `water.amount` | `water.amount` | 保留 |
| Ice stage amount | `ice.amount` | `ice.amount` | 保留 |
| Input trim | `input.gain` | `input.gain` | 保留，补 DSP 与 smoothing |
| Global dry/wet | `global.mix` | `global.mix` | 保留，补 DSP 与 smoothing |
| Output trim | `output.gain` | `output.gain` | 保留，补 DSP 与 smoothing |

在 M1 参数合同 PR 合并前，不得创建公开 preset/session 兼容性承诺。若已有外部用户使用过当前占位构建，应先确认是否需要兼容别名/迁移。

## 5. 现状对应 milestone

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证、首次 push 和两次 Hosted CI success 已验证；HOST-000、GitHub metadata 与 branch protection 尚未收口。
- M1 Audio Skeleton & Parameter Contract：**进行中早期**。有 AudioEngine、静态参数和 state 骨架，缺 Snapshot/Mapper、真实 gain、自动化验收与离线渲染。
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

按 `docs/CODING_PLAN.md` 收口 M0：完成 HOST-000、GitHub metadata 和 branch protection 的实际决策/验证，再进入 M1 参数合同和 Application/DSP 接口。不要先写 Water/Ice 生产算法。
