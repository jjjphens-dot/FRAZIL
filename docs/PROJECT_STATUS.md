# FRAZIL 当前实现与差距

> 快照日期：2026-09-11<br>
> 依据：最新 `origin/main` 的仓库文档/源码审计、TESTDATA-001 当前 revision 的本地 generator/build/CTest evidence，以及 GitHub PR/Issue live query；PR、CI 和合并状态以 GitHub live state 为准。<br>
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## Modification Policy

本文件属于 STATUS/EVIDENCE 文档，只能记录实际审计、测试、CI、兼容性或听测结果。计划、假设和未执行工作必须明确标为 planned/未验证，不得把本文件当作稳定计划合同。

## 1. 结论

项目已有可构建的 JUCE M0 骨架；M1-A/M1-B 的参数、Snapshot/Mapper、Application-DSP 接口和基础 gain signal path 已随 PR #2 / PR #3 合入 `main`。M1-C STATE-001 已通过 PR #5 squash merge 合入 `main`，versioned StateModel/Host State Adapter foundation、XML restore regression 和相关 fallback/migration evidence 已完成；PluginProcessor automation/state integration evidence 已建立；TESTDATA-001 reproducibility/provenance infrastructure 已随 PR #12 合入，`RENDER-001` pass-through offline smoke 已随 PR #13 合入。TESTDATA-001 diagnostic corpus 当前 revision 已完成本地实现与验证；其 PR、CI 和合并状态以 GitHub live state 为准。PR #18 的 TEST-002 processor property、PERF-BASE-001 baseline 和 ARCH-LAT-001 latency evidence 已随 squash merge 进入当前 `main`；M1 仍在进行中，Water/Ice、Routing、完整 render regression matrix 和正式 UI 仍未完成。

仓库文档已记录 HOST-000 兼容性矩阵和正式的 FRAZIL 产品身份；HOST-000 的 support intent 与实际 evidence status 分别由矩阵中的对应字段表示，PR、CI 和合并状态以 GitHub 为准。

`CODING_PLAN.md` v1.1 / Approved Development Baseline 已作为 M0→M7 的正式工程执行基线；这不代表 FRAZIL plugin v1.0 release，也不改变 M0/M1、Water/Ice/Routing 的实际完成状态。

本文件的第 1 节和第 2 节资产表是当前状态速览；第 2.1 至 2.5 节保留带日期的历史 evidence-time snapshots，供追溯但不作为当前 HEAD 或当前 CI 状态。第 2.6 节链接 PR #18 合入前后的 M1 engineering evidence；新的验证结果应优先写入 `docs/evidence/`，并在此处只保留当前结论和链接。

当前阻塞性差距：

1. PR #5 squash merge commit 为 `12d36a40d03944f9c69cd273d1cc3dca3e0b6ee7`；STATE-001 = MERGED / validated；`feat/m1-parameter-engine-contract` 与 `feat/m1-state-contract` 保留为历史 feature 分支。本文件记录 verified merge/evidence snapshots；current `main` HEAD should be read from GitHub，不把本文件中的 snapshot 当作永久 current HEAD；
2. PR #5 合并后 main 的 [Hosted CI run 34091515810](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34091515810) 已完成 Windows Debug configure/build/test 并通过；PR #4、PR #3 和 PR #2 的既有 Hosted CI 证据也已通过；当前 GitHub branch listing 报告 `main` 为 `protected:false`；fine-grained repository ruleset / admin-level branch-protection configuration 尚未以充分的管理员证据独立验证；不据此推断不存在其它规则集；
3. `water.enable`/`ice.enable` 与架构目标的 ID 冲突已在合入 `main` 的集中式 ParameterLayout 中修正为 `water.enabled`/`ice.enabled`；STATE-001 已建立已知 pre-v1 ID migration fixture，公开版本兼容性仍需后续 freeze/evidence；
4. APVTS 参数已通过一次 block Snapshot 和 ParameterMapper 进入 AudioEngine；当前 wet path 仍为 post-input pass-through；
5. CTest 已覆盖参数枚举、Snapshot、Mapper、mix、smoothing、RandomSource、gain staging、first-block priming、reset、zero-length、runtime buffer invariant，以及 STATE-001 的 schema round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、默认/非法输入 fallback（含 duplicate known ID、nonnumeric schemaVersion/value 和 malformed bool）、legacy ID migration、三个 routing choice 和 inactive retention；新增 plugin integration test 覆盖实际 `FRAZILAudioProcessor` 的参数写入→audio path、连续 gain automation smoothing、三种 routing mode 切换、inactive value retention、prepareToPlay -> setStateInformation -> processBlock 生命周期 restore 和 XML state reopen；`frazil_render` + `tools/render_testdata.py` 已提供 M1 pass-through offline smoke，固定 input/config/seed 两次运行并检查 finite output、输出字节一致和完整当前配置/input/output hash manifest；`frazil_render_cli` 覆盖 help 成功路径、enable/routing/balance/amount 非法值拒绝、非默认完整配置 manifest 转发，以及同一 output path 重复渲染覆盖而非追加 WAV；CTest render artifacts 已隔离到 preset build tree；历史 Debug VST3 artifact 曾使用 pluginval 1.0.4、strictness 5、seed 12345 验证并以 `SUCCESS` 结束，Steinberg validator 因未配置而跳过；current artifact 的 pluginval validation 仍为 NOT RUN；完整 render matrix 和真实 DAW automation 仍待执行；
6. Water、Ice、Routing、EditHistoryManager、完整 render regression matrix、离散 enable/routing transition 和正式 UI 仍未实现；真实 Host/DAW restore matrix、M5 EditHistoryManager integration、pluginval current-artifact validation 和公开兼容性仍待执行；`PARAM-FREEZE-001` 保持为 M2/M3 -> M4 gate，不是 M1 Exit blocker。
7. PR #2 与 PR #3 均已合入 `main`；`87fd69b docs: add two-person collaboration roles` 作为协作基线保留在历史中，未为追求历史美观而重写 feature 分支；
8. PR #3 collaborator review was not preserved as a formal GitHub Review submission；这是 process evidence gap，不是 production implementation bug。PR #9 的人工 review 进一步暴露了角色 ownership、PR creator、commit authorship 和 reviewer evidence 被混为同一账号 gate 的问题；当时提出的“必须由 Implementation DRI account 创建 PR/预绑定不同 reviewer account”属于历史纠正方案，现已由更简单的规则取代：职责和 review evidence 分别真实记录，需要第二人 review 时保留独立 evidence；已有 PR 的后续 push account 必须等于 PR creator；
9. MIT `LICENSE` 已加入；第三方 notice 策略仍待收口；
10. GitHub 当前已有治理 issue [#11](https://github.com/jjjphens-dot/FRAZIL/issues/11) 用于跟踪 portable GitHub workflow 文档同步，以及 TESTDATA-001 rationale issue [#15](https://github.com/jjjphens-dot/FRAZIL/issues/15)；Milestones 为 0，Projects 为 0；Labels 页面仅见 GitHub 默认标签，项目自定义 labels 未建立；branch listing 当前报告 `main` 为 `protected:false`，更细粒度 ruleset / admin-level branch-protection configuration 尚未独立验证；HOST-000 产品目标已由 Sound & Host Lead 冻结，HOST-001 实际 DAW evidence 和 REAPER exact version 仍未完成；当前 PR、review 和 merge 状态只以 GitHub 为准，不在本文件重复记录；不据此推断不存在其它规则集。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.25+、C++20、Ninja presets | 本机可用；portable preset 已由 Hosted CI 验证 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、versioned state XML adapter | M1-C state boundary 已接入；XML createXml/fromXml restore path 已验证；真实 Host/DAW restore 证据仍待执行 |
| Parameters | 9 个集中式 APVTS 参数静态注册；enabled ID 已使用 `.enabled` | M1 参数路径已接入；`PARAM-FREEZE-001` 是后续 M2/M3 -> M4 gate，不是 M1 Exit blocker |
| App | `ProcessSpec`、`EngineParameters`、`ParameterSnapshot`、`ParameterMapper`、`StateModel`、`AudioEngine::prepare/reset/process` | M1 gain/mix skeleton；STATE-001 versioned value/schema、known migration、invalid fallback、inactive retention；STATE-002 mode-value-retention integration verified；M5 EditHistoryManager remains planned；wet pass-through；runtime buffer invariant fallback |
| UI | 640x360 M0 占位界面 | 非产品 UI |
| Product identity | `docs/PRODUCT_IDENTITY.md` | FRAZIL adopted working/product name；命名词汇不改变参数合同；法律/商标 clearance 不属于当前工程范围 |
| Tests | `frazil_smoke` + `frazil_unit` + `frazil_plugin_integration` + `frazil_processor_property` + `frazil_latency_contract` + `frazil_render` + `frazil_render_cli` CTest；`frazil_performance` manual benchmark | Release/ASAN 各 7/7 PASS；canonical PERF-BASE-001 的 steady-state、parameter-retarget、CPU/memory/allocation、finite-output 和 denormal evidence 已记录；真实 DAW 测试未完成 |
| Local validation | PR #18 follow-up 的 Release、ASAN fresh configure、6-job safe build、7/7 CTest 和 Release manual benchmark 结果见 2.6，并按该节记录 exact tested implementation commit；当前 `main` HEAD 与最新 CI 以 GitHub live query 为准 | 已验证；本机绝对路径仅在 ignored local configuration/build output，仓库 preset 保持可移植 |
| pluginval | 当前机器缺少 `tools/bin/pluginval.exe`，current-artifact validation `NOT RUN`；历史 Debug artifact pluginval 记录保留为历史 evidence | 当前变更未验证（不等于独立 VST3 validator） |
| Remote | `jjjphens-dot/FRAZIL` public repository；`main` 和审查分支的当前 SHA、mergeability 与 CI 状态以 GitHub live query 为准；`9955cdb` 仅为历史 safety follow-up baseline，不是当前审查分支 head；HOST-000 frozen-target push 已有记录；PR #5 已将 STATE-001 合入 main；TESTDATA-001 rationale 记录见 Issue #15 | GitHub live state；Milestones/Projects metadata 未建立 |

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

## 2.3 Plugin integration validation evidence（historical 3/3 snapshot）

本节保留较早的 plugin-integration validation snapshot，不代表当前 TESTDATA revision 的完整 CTest 数量。本审查分支在合入当时的 `main` 基线后重新执行了 repository portability scan、scanner regression、build-safety regression、portable `ci-windows-debug` configure、6-job safe build 和 CTest。`frazil_smoke`、`frazil_unit` 与 `frazil_plugin_integration` 共 3/3 PASS。integration target 覆盖实际 PluginProcessor 参数写入进入 audio path、连续 gain automation smoothing、routing mode 切换后的 inactive value retention、prepareToPlay -> setStateInformation -> processBlock 生命周期 restore，以及 XML state reopen。

本机开发者工具链和构建目录只存在于 ignored local configuration/build output；tracked CMake preset 不包含个人安装路径。干净 clone 的依赖 bootstrap 仍需在 GitHub 网络可用时单独复验。

## 2.4 PR #9 人工 review 与身份流程 evidence

以下状态按 2026-09-08 的 GitHub live query 和协作者提供的人工 review 截图分别记录，不把截图中的批准文字改写成 GitHub formal review：

- PR：`#9`，当前审查快照 head 为 `f016f13`，base 为 `main`；PR author 为 `jjjphens-dot`。
- 当前 head 的最近实现/修复提交由 `Aspartameqwq` authored/committed；PR 历史同时包含 `jjjphens-dot` 与 `Aspartameqwq` 的 commit authors。commit 署名不改变 PR author，也不能替代 reviewer 身份。
- 人工 review：协作者提供的 review 结论为 `APPROVE`，未发现 P0/P1 阻塞问题；复核范围包括 mode-retention、ASAN runtime discovery/copy、CTest PATH、VS Code/MSVC portability、Markdown/task/build-safety 和 `git diff --check`，截图明确未重新执行 pluginval、真实 DAW、render、performance benchmark。
- GitHub formal review：当前 API `reviews=[]`、`reviewDecision` 为空；因此 PR #9 的协作者批准目前只能记录为人工 screenshot/comment evidence，不能记录为正式 GitHub `APPROVE` submission。
- 流程状态：`Manual collaborator review = APPROVE (screenshot evidence)`；`Formal GitHub review = NOT RECORDED`。PR #9 后续已合入 `main`，该缺口保留为 historical process evidence gap，不通过回滚或历史重写补造。
- 当前纠正规则：Implementation/Acceptance DRI、PR creator 和 reviewer 分别记录，不要求相同或预绑定；commit author/committer 由 Git history 记录，push account 只在 existing-PR push 时运行时核对，不在普通 PR body 重复抄写。需要第二人 review 的工作仍保留真实独立 evidence。向已有 PR 分支继续 push 时，只有当前 authenticated account 与 PR creator 相同才允许；同一 repository + branch + PR + auth session 的成功检查可以复用，context/account/permission 变化时重新检查。该规则取代本节早期快照中的严格账号映射方案。

## 2.5 TESTDATA-001 validation evidence

TESTDATA-001 diagnostic refinement exists to provide stable mathematical engineering inputs for future
Water/Ice/Routing development。Representative musical acceptance is intentionally separated into
`LISTENING-001`，while real DAW behavior remains `HOST-001` evidence。当前 closeout 的 GitHub rationale
记录见 [Issue #15](https://github.com/jjjphens-dot/FRAZIL/issues/15)。

PR #12 合入的基线以 `tools/generate_testdata.py` 建立了固定 seed、provenance、license、
storage、WAV metadata 和 SHA-256 审计基础。本 follow-up 将 `testdata/input/` 重构为十个按
DSP 目标命名的双声道、48 kHz、PCM24 diagnostic signals；manifest `schemaVersion=2` 保持
为 testdata schema，插件 state `schemaVersion=1` 未改变。当前仓库原 generator 已是 version 2，
本次语义/格式重构将 generator metadata 更新为 version 3，并采用稳定 per-signal seed。
`tools/verify_testdata.py` 已验证新的 signal objective/class/parameters/properties/windows/targets、
provenance/license、PCM24 WAV metadata、SHA-256 和 input/manifest 双向集合；
`tools/test_testdata.py` 已验证 deterministic regeneration、manifest 语义、WAV 字节、SHA-256、
tamper/storage/provenance checks，以及十个信号的标准库 semantic checks。44100/48000/96000
临时 generation 均通过；`RENDER-001` 已改用 `zero_state_response__impulse.wav`，没有建立第二套
render harness。`testdata/rendered/` 继续保持 ignored；该 evidence 不等同于 Water/Ice DSP、
真实 DAW 或听测验收，真实音乐素材仍应由独立 `LISTENING-001` 放入 `testdata/listening/`。

2026-09-09 本地 evidence：`python tools/generate_testdata.py --sample-rate 48000` PASS（10 fixtures）；
`python tools/verify_testdata.py` PASS；`python tools/test_testdata.py` PASS；
`python tools/check_markdown_links.py`、`python tools/test_check_markdown_links.py`、
`python tools/check_portability.py`、`python tools/test_check_portability.py` 和 `git diff --check`
均 PASS；`cmake --preset windows-debug` 首次因旧 cache/linker 环境失败，随后通过仓库
`tools/vscode_msvc_env.cmd` 与 `cmake --fresh --preset windows-debug` 成功 configure；
`cmd /c tools\vscode_build_safe.cmd --preset windows-debug` 通过 safe wrapper build，
`ctest --preset windows-debug --output-on-failure` 通过 5/5（含 frazil_unit、
frazil_plugin_integration、frazil_render、frazil_render_cli）；`tools/analyze_testdata.py`
对 `frequency_response__log_sweep.wav` 与 `harmonic_response__stepped_sine_1khz.wav`
的代表性 FFT/Welch PSD/STFT 分析均 PASS。未执行 Release/ASAN、pluginval、完整
FFT/THD/IMD measurement、性能 benchmark、真实 DAW 或 manual listening；按本 task
non-goals 标记为 NOT RUN。

## 2.6 M1 engineering follow-up evidence

PR #18 中的 tested implementation commit `e56664c8775f614e77004ac891d63a5c2fd2fa7a` 的 TEST-002、PERF-BASE-001 和 ARCH-LAT-001 结果见 [`docs/evidence/M1_ENGINEERING_EVIDENCE.md`](evidence/M1_ENGINEERING_EVIDENCE.md) 与 [`docs/evidence/PERF-BASE-001.md`](evidence/PERF-BASE-001.md)。Release、ASAN 均通过 fresh configure、safe build 和 7/7 CTest；canonical `frazil_performance` manual Release benchmark 的 `configuredCommit` 与 implementation commit 一致、`sourceState=clean`，并记录 steady-state/parameter-retarget、CPU、memory、allocation、finite-output 和 denormal observations；focused mutation verification 也证明 active lifecycle corruption、active valid-but-different output 和 neutral exact-repeatability corruption 能被分别验证。Hosted Windows Debug run `34491770873` 由 PR #18 的 `pull_request` 事件验证 PR head `1eef0c675061fe01e8948cf05c6fd1baf4226e90`，并通过 Configure、Build 和 7/7 Test；当前 main squash commit `49e87256ddf60116fb72caf29f6c5a511ac6993a` 与该 PR head 包含相同 Git tree `b93c7982de12d97474be8867d0b8f1576b392eee`，但该 run 不是直接挂在 merge commit 上。更早的 evidence-time heads `c1aea641688397f80bec0996e8eb3375b5f60def`、`c3bfeab4d55c47e322cad6ef7cfc259f6542b6dd` 和 `b8918ec93fe696867e16712e67837173b6cfceb4` 及其 Hosted CI runs 保留为历史 evidence。pluginval、真实 DAW、listening 和 M1 Joint Exit 不在本地/Hosted Debug 证据范围内，需后续 acceptance。

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
- 有意保留的未实现项：Water、Ice、Routing、EditHistoryManager、完整 render regression matrix、正式 UI 和离散 transition 均仍按 Coding Plan 处于计划阶段；当前 RENDER-001 只覆盖 M1 pass-through offline smoke；本次状态工作不提前创建声音算法或 history 生产依赖。
- 当前验证边界：CTest 已提供当前列出的 unit/lifecycle/invariant、processor property、latency 和 render smoke 证据；PERF-BASE-001 是独立 manual benchmark，不是 CTest gate。以上不能据此宣称完整 realtime safety、真实 DAW、完整 render/listening 或公开兼容性已完成。

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

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证、首次 push 和两次 Hosted CI success 已验证；HOST-000 产品目标矩阵与产品身份文档已记录，但 official-support gate、实际 Host smoke、GitHub metadata 与 branch protection 尚未收口。HOST-001 evidence 不阻塞 HOST-000 定义目标，但阻塞 M1 Exit Gate。
- M1 Audio Skeleton & Parameter Contract：**进行中**。M1-A/M1-B foundation 与 PR #5 中的 STATE-001 versioned StateModel/Host State Adapter foundation 已合入 `main`；STATE-002 mode-value-retention、AUTO-001 plugin integration、TESTDATA-001 reproducibility infrastructure、RENDER-001 pass-through offline smoke、TEST-002、PERF-BASE-001 和 ARCH-LAT-001 已建立；PR #18 tested implementation 的 Debug/Release/ASAN engineering evidence 见 `docs/evidence/`，不把本整改 branch 的局部验证写成 main 或 milestone evidence；仍缺完整 render regression matrix、真实 DAW 验证、current-artifact pluginval、正式参数 freeze 和 M1 Joint Exit；M5 EditHistoryManager 仍未开始。
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
Engineering lane
  -> RENDER-001 acceptance/finding follow-up
  -> fix findings handed back from HOST-001

Sound / Host lane
  -> HOST-001 Ableton -> FL Studio -> REAPER
  -> parameter enumeration / automation / save-reopen / DAW render evidence
  -> representative workload for PERF-BASE-001
  -> EXP-W-001 perceptual brief only; no production DSP
  -> LISTENING-001 representative corpus preparation (parallel; does not block M1 Joint Exit)

Both evidence lanes
  -> M1 Joint Exit Review
  -> EXP-W-002 / EXP-I-002 engineering experiments using TESTDATA-001
  -> EXP-W-003 / EXP-I-003 selection only after LISTENING-001 is ready
  -> WATER-006 / ICE-006 listening evidence only after LISTENING-001 is ready
  -> Water production only after experiment and Joint Gate
```

HOST-000 产品目标已冻结，support classification 仍需满足 Engineering Lead review 与 HOST-001 evidence 条件。STATE-001/002、AUTO-001 foundation、TESTDATA-001 original reproducibility/provenance infrastructure（PR #12）和 RENDER-001 pass-through smoke（PR #13）已进入 `main` 并转为 regression/finding ownership；当前 TESTDATA-001 diagnostic semantic refinement 仍是同一 work item 的 finding-driven follow-up，其 PR/CI/merge 状态只以 GitHub live state 为准。不得建立平行实现。Water/Ice/Routing 仍未开始 production。
