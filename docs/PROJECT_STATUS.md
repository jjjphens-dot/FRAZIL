# FRAZIL 当前实现与差距

> 快照日期：2026-09-07<br>
> 依据：本地源码/构建目录审计、当前 Debug/portable build/CTest、GitHub 仓库页面与 Actions 页面。<br>
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## Modification Policy

本文件属于 STATUS/EVIDENCE 文档，只能记录实际审计、测试、CI、兼容性或听测结果。计划、假设和未执行工作必须明确标为 planned/未验证，不得把本文件当作稳定计划合同。

## 1. 结论

项目已有可构建的 JUCE M0 骨架；M1-A/M1-B 的参数、Snapshot/Mapper、Application-DSP 接口和基础 gain signal path 已随 PR #2 / PR #3 合入 `main`。M1-C STATE-001 已通过 PR #5 squash merge 合入 `main`，versioned StateModel/Host State Adapter foundation、XML restore regression 和相关 fallback/migration evidence 已完成；本分支新增实际 PluginProcessor automation/state integration evidence；M1 仍在进行中；Water/Ice、Routing、render 和正式 UI 仍未实现。

`CODING_PLAN.md` v1.0 / Approved Development Baseline 已作为 M0→M7 的正式工程执行基线；这不代表 FRAZIL plugin v1.0 release，也不改变 M0/M1、Water/Ice/Routing 的实际完成状态。

当前阻塞性差距：

1. PR #5 squash merge commit 为 `12d36a40d03944f9c69cd273d1cc3dca3e0b6ee7`；STATE-001 = MERGED / validated；`feat/m1-parameter-engine-contract` 与 `feat/m1-state-contract` 保留为历史 feature 分支。本文件记录 verified merge/evidence snapshots；current `main` HEAD should be read from GitHub，不把本文件中的 snapshot 当作永久 current HEAD；
2. PR #5 合并后 main 的 [Hosted CI run 34091515810](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34091515810) 已完成 Windows Debug configure/build/test 并通过；PR #4、PR #3 和 PR #2 的既有 Hosted CI 证据也已通过；当前 GitHub branch listing 报告 `main` 为 `protected:false`；fine-grained repository ruleset / admin-level branch-protection configuration 尚未以充分的管理员证据独立验证；不据此推断不存在其它规则集；
3. `water.enable`/`ice.enable` 与架构目标的 ID 冲突已在合入 `main` 的集中式 ParameterLayout 中修正为 `water.enabled`/`ice.enabled`；STATE-001 已建立已知 pre-v1 ID migration fixture，公开版本兼容性仍需后续 freeze/evidence；
4. APVTS 参数已通过一次 block Snapshot 和 ParameterMapper 进入 AudioEngine；当前 wet path 仍为 post-input pass-through；
5. CTest 已覆盖参数枚举、Snapshot、Mapper、mix、smoothing、RandomSource、gain staging、first-block priming、reset、zero-length、runtime buffer invariant，以及 STATE-001 的 schema round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、默认/非法输入 fallback（含 duplicate known ID、nonnumeric schemaVersion/value 和 malformed bool）、legacy ID migration、三个 routing choice 和 inactive retention；新增 plugin integration test 覆盖实际 `FRAZILAudioProcessor` 的参数写入→audio path、连续 gain automation smoothing、三种 routing mode 切换、inactive value retention 和 XML state reopen；本分支 Debug VST3 artifact 使用 pluginval 1.0.4、strictness 5、seed 12345 验证并以 `SUCCESS` 结束，Steinberg validator 因未配置而跳过；尚未覆盖 render 和真实 DAW automation；
6. Water、Ice、Routing、EditHistoryManager、离线 render、DSP property/performance harness、离散 enable/routing transition 和正式 UI 仍未实现；真实 Host/DAW restore matrix、M5 EditHistoryManager integration 和公开兼容性仍待执行。
7. PR #2 与 PR #3 均已合入 `main`；`87fd69b docs: add two-person collaboration roles` 作为协作基线保留在历史中，未为追求历史美观而重写 feature 分支；
8. PR #3 collaborator review was not preserved as a formal GitHub Review submission；这是 process evidence gap，不是 production implementation bug；从下一条需要双人 review 的核心 PR 开始，必须留下 formal review 或满足治理规则的第二位开发者 comment evidence；
9. MIT `LICENSE` 已加入；第三方 notice 策略仍待收口；
10. GitHub Issues 全状态筛选无结果，Milestones 为 0，Projects 为 0；Labels 页面仅见 GitHub 默认标签，项目自定义 labels 未建立；branch listing 当前报告 `main` 为 `protected:false`，更细粒度 ruleset / admin-level branch-protection configuration 尚未独立验证；HOST-000 产品目标已由 Sound & Host Lead 冻结并推送至 `origin/experiment/music-dsp`；Engineering Lead review、PR/CI 和 merge pending，HOST-001 实际 DAW evidence pending，REAPER exact version pending；不据此推断不存在其它规则集。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.22+、C++20、Ninja presets | 本机可用；portable preset 已由 Hosted CI 验证 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、versioned state XML adapter | M1-C state boundary 已接入；XML createXml/fromXml restore path 已验证；真实 Host/DAW restore 证据仍待执行 |
| Parameters | 9 个集中式 APVTS 参数静态注册；enabled ID 已使用 `.enabled` | M1 参数路径已接入；合同仍待 freeze |
| App | `ProcessSpec`、`EngineParameters`、`ParameterSnapshot`、`ParameterMapper`、`StateModel`、`AudioEngine::prepare/reset/process` | M1 gain/mix skeleton；STATE-001 versioned value/schema、known migration、invalid fallback、inactive retention；STATE-002 mode-value-retention integration verified；M5 EditHistoryManager remains planned；wet pass-through；runtime buffer invariant fallback |
| UI | 640x360 M0 占位界面 | 非产品 UI |
| Tests | `frazil_smoke` + `frazil_tests` + `frazil_plugin_integration` CTest | M1 contract/gain/smoothing/priming/invariant + STATE-001 state/XML restore + PluginProcessor automation/state integration 覆盖；DSP property/真实 DAW 测试未完成 |
| Local validation | 本轮 `ci-windows-debug` configure、6-job safe build 和 CTest 均通过；smoke、unit、plugin integration 共 3/3 PASS | 已验证；本机绝对路径仅在 ignored `CMakeUserPresets.json`，仓库 preset 保持可移植 |
| pluginval | 本分支已有 Debug VST3 artifact 的 pluginval 1.0.4、strictness 5、seed 12345 `SUCCESS` 记录；Steinberg validator 因未配置而跳过 | 已验证（不等于独立 VST3 validator） |
| Remote | `jjjphens-dot/FRAZIL` public repository；`main` 当前为 `12ef3fb`，审查分支为 `9955cdb`；HOST-000 frozen-target push 已有记录；PR #5 已将 STATE-001 合入 main | 本分支等待当前 PR/CI/review；Issues/Milestones/Projects metadata 未建立 |

## 2.1 Clean portability/build-safety PR evidence

Clean portability/build-safety implementation baseline: 03bee6a, based on origin/main 00ebd8e. Hosted PR CI run 34128576335 (pull_request) validated that implementation baseline successfully. Current PR head, current mergeability, and latest CI state are live GitHub state and must be queried from GitHub when needed.

- Portability scanner：PASS。
- Portability regression tests：PASS。
- Build-safety regression tests：PASS。
- Configure：PASS；portable-windows-base provides CMAKE_BUILD_PARALLEL_LEVEL=6。
- Safe ci-windows-debug build：PASS。
- CTest：PASS。
- Evidence-time PR mergeability：MERGEABLE / CLEAN；current live mergeability must be read from GitHub。
- Local 6-job and 8-job check-only：REFUSED by the existing memory gate because only about 2.45-2.48 GiB was available；no local C++ build was started。
- Release：NOT RUN。
- ASAN：NOT RUN。


## 2.2 Repository portability remediation evidence

repository-portability remediation 与 bounded build-safety guard 已在 commit 7d5f9a8 对应版本完成静态和安全 preflight 验证：python tools/check_portability.py、python tools/test_check_portability.py、python tools/test_build_safe.py、cmake --list-presets、CMake/VS Code JSON 解析和 6/8 job preflight 均通过；9 job 与 CMAKE_BUILD_PARALLEL_LEVEL=32 的绕过尝试均被拒绝。事故前 519ede8 的 Debug/Release/ASAN 本地构建记录保持为历史证据。Hosted GitHub Actions 已在 PR #7 的 run 34125302391（workflow_dispatch，Windows Debug / CMake / CTest，4m55s）通过；Fresh clone 与 VS Code GUI task 仍未在本机单独复验。

安全 follow-up commit f598922 的验收状态：
- Python safety tests：PASS（py_compile、portability scanner、scanner regression、build-safety regression）。
- Build preflight：PASS（默认 6 jobs 与显式 8 jobs 的 check-only）。
- Refusal paths：PASS（jobs=9、CMAKE_BUILD_PARALLEL_LEVEL=32、非法环境值 abc 均拒绝）。
- Configure safety：PASS（portable-windows-base 注入 CMAKE_BUILD_PARALLEL_LEVEL=6，Hosted Configure 成功）。
- Debug safe build：PASS（Hosted CI 的 ci-windows-debug safe wrapper；本机 post-incident Debug 未重跑）。
- Debug CTest：PASS（Hosted CI）。
- Release：NOT RUN。
- ASAN：NOT RUN。
- 本机 post-incident full Debug build：NOT RUN；本轮没有启动本机高负载构建。

Branch audit 仅报告未合并分支中的既有基线路径污染，不改写其它 branch；合并或 cherry-pick 本修复后应重新运行 portability scan。

## 2.3 Plugin integration validation evidence

本审查分支在合入当前 `main` 基线后重新执行了 repository portability scan、scanner regression、build-safety regression、portable `ci-windows-debug` configure、6-job safe build 和 CTest。`frazil_smoke`、`frazil_unit` 与 `frazil_plugin_integration` 共 3/3 PASS。integration target 覆盖实际 PluginProcessor 参数写入进入 audio path、连续 gain automation smoothing、routing mode 切换后的 inactive value retention，以及 XML state reopen。

本机开发者工具链和构建目录只存在于 ignored local configuration/build output；tracked CMake preset 不包含个人安装路径。干净 clone 的依赖 bootstrap 仍需在 GitHub 网络可用时单独复验。
## 3. 当前源码映射

```text
PluginProcessor
  ├─ owns APVTS
  ├─ uses src/plugin/ParameterLayout for static parameters
  ├─ caches APVTS raw parameter atomics
  ├─ captures one ParameterSnapshot per processBlock
  ├─ maps to EngineParameters
  ├─ saves/restores versioned state through Host State Adapter -> StateModel
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
- 已确认技术债：`PluginProcessor` 仍公开 APVTS，后续需要收窄 Host parameter interface；完整 Host/DAW state compatibility 与 history integration 尚未完成；wet path 仍为 pass-through，Water/Ice/Routing 尚未实现。
- 有意保留的未实现项：Water、Ice、Routing、EditHistoryManager、正式 UI、render/property/performance harness 和离散 transition 均仍按 Coding Plan 处于计划阶段；本次 STATE-001 不提前创建声音算法或 history 生产依赖。
- 当前验证边界：CTest 仅提供当前列出的 unit/lifecycle/invariant 证据，不能据此宣称参数 automation、state compatibility、DSP property/render、DAW 或完整 realtime safety 已完成。

## 4. 参数差异审计

| 语义 | 当前代码 | 目标合同 | 动作 |
|---|---|---|---|
| Water enable | M0 历史 `water.enable` | `water.enabled` | 合入 `main` 的集中式 ParameterLayout 已迁移；STATE-001 已建立最小 known-ID migration fixture，首个公开版本前仍需完整 compatibility evidence |
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

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证、首次 push 和两次 Hosted CI success 已验证；HOST-000 产品目标已冻结并推送至 `origin/experiment/music-dsp`，但 Engineering Lead review、PR/CI、merge、实际 Host smoke、GitHub metadata 与 branch protection 尚未收口。HOST-001 evidence 不阻塞 HOST-000 定义目标，但阻塞 M1 Exit Gate。
- M1 Audio Skeleton & Parameter Contract：**进行中**。M1-A/M1-B foundation 与 PR #5 中的 STATE-001 versioned StateModel/Host State Adapter foundation 已合入 `main`；本次新增并验证 STATE-002 mode-value-retention 与 AUTO-001 plugin integration；仍缺 render/property/performance、真实 DAW 验证和正式参数 freeze；M5 EditHistoryManager 仍未开始。
- M2 Water：**未开始**。
- M3 Ice：**未开始**。
- M4 Routing：**未开始**。
- M5 UI & Edit History：**未开始**。
- M6 Beta Hardening：**未开始**。
- M7 v1.0 Release：**未开始**。

## 6. 不应从现状推断的结论

- 参数“能被 APVTS 注册”不等于 automation click-free 或 DSP 已消费参数；
- STATE-001 versioned state 能通过当前 XML transport round-trip 不等于完整跨版本兼容、真实 DAW restore 或 M5 EditHistoryManager integration 已建立；
- pluginval 历史通过不等于真实 DAW matrix 已通过；
- pass-through 能构建不等于 routing/gain/dry-wet 的数学和增益结构正确；
- 目录 README 存在不等于对应模块已经实现；
- 本地 `external/JUCE` 存在不等于 fresh clone 可复现。

## 7. 下一步顺序

```text
HOST-000 Engineering review / PR / merge
  -> TESTDATA-001 可并行启动
  -> HOST-001 save/reopen/automation/DAW evidence
  -> RENDER-001 / PERF-BASE-001
  -> M1 Exit Gate
  -> EXP-W-001/002/003
  -> Water production only after experiment gate
```

HOST-000 已完成产品目标冻结并推送至 `origin/experiment/music-dsp`；Engineering Lead review、PR 和 merge 尚未完成。STATE-001 已随 PR #5 合入 `main`，后续继续收口 STATE-002、automation、TESTDATA/RENDER/PERF harness 和 HOST-001 evidence。Water/Ice/Routing 仍未开始 production。
