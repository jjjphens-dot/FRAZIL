# FRAZIL 当前实现与差距

## A1/B1/D1 Preview bridge

IMPLEMENTED / VALIDATION PENDING on the bridge branch. Default Legacy retained; Reworked typed-default
A1/B1/D1 integrates with existing runtime A/B and transport. No session schema upgrade.
HUMAN LISTENING NOT ASSESSED; PRODUCT NOT ADOPTED; PRODUCTION WATER NOT IMPLEMENTED.
D1 C6 pending, C7 Joint Gate NOT RECORDED; ADR-0007 Proposed. Not a main/merge claim.
[Commands, results and limitations](evidence/WATER_PREVIEW_A1_B1_D1_BRIDGE.md).


Current next-stage work is [FD-003](../experiments/water/EXP-W-FD-003.md) research
convergence: S0/S1/S2, fixed-character filters and event-aware cross-rate evidence.
C0–C5 engineering evidence is complete: 24 filter configurations, three S1 variants
plus S0/S2 controls, 32 native comparisons and 64 analytic cross-rate comparisons.
Independent [Hosted validation](https://github.com/jjjphens-dot/FRAZIL/actions/runs/36242885134)
passed on code `299badb` (Debug 38/38, Release 37/37 with Preview disabled).
Local Release, ASAN and bounded Debug repeat passed 38/38. Initial Debug was 37/38
after a source-probe access violation (D1-VAL-003, root cause unknown); failure
evidence remains preserved and open. C6 listening protocol is ready; audio generation,
human cross-rate interpretation and C7 ADR-0007 acceptance remain pending. See the
[convergence evidence](evidence/WATER_FLOW_D1_CONVERGENCE.md). No runtime replacement
or final filter/latency adoption is claimed.

The preceding EXP-W-FD-002 feasibility study is archived as engineering evidence;
FD-003 now governs convergence. ADR-0007 remains Proposed, not Accepted; the current
plugin remains zero-latency. FD-002 local Debug/Release/ASAN full repeats passed 37/37;
its independent Hosted validation passed 220 numerical comparisons and 1540 resource
rows, with historical preservation 1707/1707. These are historical results, not
FD-003 validation. Local export-fault causes remain open. No runtime replacement.
See the [latency study](evidence/WATER_FLOW_D1_LATENCY_STUDY.md).

Flow D1 is implemented as a separate offline reduced-transfer research candidate;
validation status and open numerical/listening limits are in
[D1 execution](evidence/WATER_FLOW_D1_EXECUTION.md). The separate Preview bridge above consumes its existing defaults; it does not change
Host/state or production readiness. The older PR40 “conditional Flow not triggered”
statements below retain their historical campaign scope; the new user plan independently
authorizes EXP-W-FD-001. Human acceptance remains NOT ASSESSED.
The original limited-band kernel PASS is superseded: D1-NUM-001 remains OPEN /
BLOCKED. Source-aware remediation adds independent numerical research and explicit
reduction limits; it does not establish an accepted replacement. See the
[remediation finding](evidence/WATER_FLOW_D1_REMEDIATION.md). Representative musical
pad listening is deferred to later human review by user instruction.
Current remediation validation also records D1-VAL-001: an ASAN full-suite native
access violation on the unchanged A1 baseline render. It remains unresolved;
baseline/hosted success does not supersede that local failure.
The first preservation harness also terminated with Python/VCRUNTIME access
violation after1152 exact pairs (D1-VAL-002); repeat evidence is kept separate.

A1+B1 closeout fixes B1 pool bounds, mixed amplitude provenance and captured-event diagnostics. Local Debug/Release/ASAN and current/historical decoded preservation pass; combined freeze also requires hosted CI on the published exact head. Scope and evidence: [B1 execution](evidence/WATER_DROPLET_B1_EXECUTION.md). Human NOT ASSESSED; product NOT ADOPTED; production Water NOT IMPLEMENTED.

A1 governance remediation on B1 base `8f3b7ca`: classifications, typed v2 authority,
strict descriptor snapshot and independent lifetime/seed properties are implemented
for review. Final validation is recorded in [A1 execution](evidence/WATER_BUBBLE_A1_EXECUTION.md);
do not infer a PASS from this status entry. No sonic tuning or new B1 behavior is intended.


B1 branch work: [Droplet B1](../experiments/water/EXP-W-DB-001.md) is a separate offline research candidate. Validation and limitations are recorded only in [B1 execution](evidence/WATER_DROPLET_B1_EXECUTION.md). Human listening and product adoption remain NOT ASSESSED; Legacy Preview/session v5 uses A0/B0/D0; Reworked runtime now uses A1/B1/D1. Historical B0 evidence below is not B1 evidence.

Current Water distinction: EXP-W-001 Revision B is ACCEPTED for perceptual definition.
A1 v2 is an engineering review candidate, HUMAN NOT ASSESSED, production NOT IMPLEMENTED.
PHYS-REF and MACRO-NEUTRAL differ; Legacy Preview uses A0 and Reworked uses A1 typed defaults.
Legacy B/D/C and Protect behavior is unchanged. See [Water index](../experiments/water/README.md) for current versus historical evidence.

2026-09-24 bounded research addition: [Bubble A1](../experiments/water/EXP-W-BA-001.md)
implements an explicit offline independent-bubble candidate; validation state is in
its [execution record](evidence/WATER_BUBBLE_A1_EXECUTION.md). Preview runtime integration is recorded above; human
listening acceptance and production adoption remain pending. This does not reopen
or close a milestone, change Host support, or replace A0/B/D/C evidence below.

> 快照日期：2026-09-18（仅追加 Protect review remediation；Decay baseline 与其余 evidence 保留原适用范围）<br>
> 依据：最新 `origin/main` 的仓库文档/源码审计、TESTDATA-001 当前 revision 的本地 generator/build/CTest evidence，以及 GitHub PR/Issue live query；PR、CI 和合并状态以 GitHub live state 为准。<br>
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## Modification Policy

本文件属于 STATUS/EVIDENCE 文档，只能记录实际审计、测试、CI、兼容性或听测结果。计划、假设和未执行工作必须明确标为 planned/未验证，不得把本文件当作稳定计划合同。

## 1. 结论

项目已完成 **M1 Audio Skeleton & Parameter Contract Exit**，当前进入 **Water pre-M2 preparation + Developer Sound/Debug Tooling acceptance follow-up**。
M1-A/M1-B foundation、STATE-001/002、AUTO-001、TESTDATA-001、RENDER-001、TEST-002、PERF-BASE-001 和
ARCH-LAT-001 engineering evidence 已进入 `main`；current-artifact pluginval 已完成 strictness-5 本地
验证，Ableton 与 FL Studio 的 primary HOST-001 matrix 已由 Sound & Host Lead 签认为 `Passed`。Engineering
Lead 已正式批准精确 PR #27 HEAD `01d590f`，Hosted CI 成功，PR #27 已合入 `main@3438593`；HOST-001
关闭、两个 primary host 达到本记录范围内的 `Development Validated`，M1 Joint Exit 已批准。
Water/Ice、Routing、完整 render regression matrix 和正式 UI 仍未实现。

仓库文档已记录 HOST-000 兼容性矩阵和正式的 FRAZIL 产品身份；HOST-000 的 support intent 与实际 evidence status 分别由矩阵中的对应字段表示，PR、CI 和合并状态以 GitHub 为准。

`CODING_PLAN.md` v1.4 的 candidate contract / engineering boundaries 已获
[PR #35 proposal HEAD 765f42a 的独立 APPROVE](https://github.com/jjjphens-dot/FRAZIL/pull/35#pullrequestreview-5223609754)，
该 HEAD 的 [Hosted Windows Debug / CMake / CTest](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35103827055) 已通过。
2026-09-17 live query 确认 [PR #35](https://github.com/jjjphens-dot/FRAZIL/pull/35) 已于
2026-09-16 14:32:33 UTC 合入 `main@fc20370`，v1.4 Approved Development Baseline 已生效。
上述 proposal HEAD 的历史 approval/CI 不冒充 merge commit 的新验证；v1.3 /
[PR #23](https://github.com/jjjphens-dot/FRAZIL/pull/23) 是 previous approved baseline。
这不代表 FRAZIL plugin v1.0 release，
也不改变 M1、Water/Ice/Routing 的实际完成状态：Water production DSP、production candidate controls
和 model transition 均未实现或注册；Debug/ASAN 开发面板中的 experiment-only controls 不属于该
production scope。

[DOC-W-PROTECT-001 / #36](planning/WATER_PROTECT_CANDIDATE_REVISION.md) 是历史 Wave 1 提案。用户后续授权
逐波工程自审并最终统一上传；[PROTECT-EXP-001 execution](planning/WATER_PROTECT_EXECUTION.md) 记录现有
独立 experiment 中的 detector、residual gain、Fluid placement、renderer 与数值验证，及每项实际结果。
没有 production Water/Developer/Host Protect control 或第五个已接受 macro。Issue #17 的 owner brief/
Decay Revision B、精确版本 re-review 和 whole-contract acceptance 仍待完成。该 objective follow-up 不等于
formal EXP-W-002 或 product adoption；用户将后续提供音频/结论，人类听测尚未执行。PR #37 remediation
将 fixed-source-gain 主听测与 RMS-matched preference evidence 分开，加入显式 D0/D1 盲测条件；
detector selection 仍未完成，Wave 7 产品决定在 selection 和人类听测完成前保持 BLOCKED。

PR #37 CI interpreter remediation：历史 head `90b7f2c` 的
[run 35310887776](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35310887776) 为 FAIL（18/19；pip 使用
Python 3.12.10，CMake/CTest 使用 3.14.7，听测测试缺 NumPy）。修复 head `82f6960` 的
[run 35313675800](https://github.com/jjjphens-dot/FRAZIL/actions/runs/35313675800) 已 SUCCESS，19/19 PASS，
`frazil_water_protect_listening` PASS；日志核实 pip/CMake/CTest 同为 Python 3.12.10 的同一个 executable。
此结果只适用于该 implementation head，后续文档提交仍需自己的 exact-head CI 和最终独立 review。
本次不诊断或宣称修复此前本地 `python312.dll` 崩溃，也不形成 Hosted Release/ASAN、听测或产品采纳证据。

当前阻塞性差距：

1. PR #5 squash merge commit 为 `12d36a40d03944f9c69cd273d1cc3dca3e0b6ee7`；STATE-001 = MERGED / validated；`feat/m1-parameter-engine-contract` 与 `feat/m1-state-contract` 保留为历史 feature 分支。本文件记录 verified merge/evidence snapshots；current `main` HEAD should be read from GitHub，不把本文件中的 snapshot 当作永久 current HEAD；
2. PR #5 合并后 main 的 [Hosted CI run 34091515810](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34091515810) 已完成 Windows Debug configure/build/test 并通过；PR #4、PR #3 和 PR #2 的既有 Hosted CI 证据也已通过；当前 GitHub branch listing 报告 `main` 为 `protected:false`；fine-grained repository ruleset / admin-level branch-protection configuration 尚未以充分的管理员证据独立验证；不据此推断不存在其它规则集；
3. `water.enable`/`ice.enable` 与架构目标的 ID 冲突已在合入 `main` 的集中式 ParameterLayout 中修正为 `water.enabled`/`ice.enabled`；STATE-001 已建立已知 pre-v1 ID migration fixture，公开版本兼容性仍需后续 freeze/evidence；
4. APVTS 参数已通过一次 block Snapshot 和 ParameterMapper 进入 AudioEngine；当前 wet path 仍为 post-input pass-through；
5. CTest 已覆盖参数枚举、Snapshot、Mapper、mix、smoothing、RandomSource、gain staging、first-block priming、reset、zero-length、runtime buffer invariant，以及 STATE-001 的 schema round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、默认/非法输入 fallback（含 duplicate known ID、nonnumeric schemaVersion/value 和 malformed bool）、legacy ID migration、三个 routing choice 和 inactive retention；新增 plugin integration test 覆盖实际 `FRAZILAudioProcessor` 的参数写入→audio path、连续 gain automation smoothing、三种 routing mode 切换、inactive value retention、prepareToPlay -> setStateInformation -> processBlock 生命周期 restore 和 XML state reopen；`frazil_render` + `tools/render_testdata.py` 已提供 M1 pass-through offline smoke，固定 input/config/seed 两次运行并检查 finite output、输出字节一致和完整当前配置/input/output hash manifest；`frazil_render_cli` 覆盖 help 成功路径、enable/routing/balance/amount 非法值拒绝、非默认完整配置 manifest 转发，以及同一 output path 重复渲染覆盖而非追加 WAV；CTest render artifacts 已隔离到 preset build tree；closeout candidate Debug VST3 使用 pluginval 1.0.4、strictness 5、seed 12345 验证并以 `SUCCESS` 结束，Steinberg validator 因未配置而跳过；Ableton Live 12.4.2 与 FL Studio 2025 25.1.4.4951 的 HOST-001 primary matrix 已记录为 Sound & Host `Passed`，详见 [HOST-001 primary DAW evidence](evidence/HOST-001-DAW-SMOKE-2026-09-13.md)；完整未来 render regression matrix 仍待后续阶段执行，REAPER secondary evidence 延期；
6. `DEV-UI-001` 已进入当前 `main@b595a47` 的 Debug/ASAN 工程实现：它绑定当前 9 个 Host 参数，保持
   Water experiment-only controls，在 override active 时让 effective developer state 拥有可见 controls，
   并提供显式 Return Host；它还覆盖不触碰 APVTS 的临时 A/B/Reset override、developer-only Dry/Processed
   comparison、完整 draft experiment-config export 和 coherent prepared/latest block diagnostics。该实现
   不是 M5 Production UI，也不改变 Host registry 或 plugin state schema；interactive usability、真实
   Host/DAW、listening 和 Offline Sound Lab handoff 仍待完成。Water、Ice、Routing、EditHistoryManager、
   完整 render regression matrix、离散 enable/routing transition 和正式 UI 仍未实现；
   `PARAM-FREEZE-001` 保持为 M2/M3 -> M4 gate，不是 M1 Exit blocker。
7. PR #2 与 PR #3 均已合入 `main`；`87fd69b docs: add two-person collaboration roles` 作为协作基线保留在历史中，未为追求历史美观而重写 feature 分支；
8. PR #3 collaborator review was not preserved as a formal GitHub Review submission；这是 process evidence gap，不是 production implementation bug。PR #9 的人工 review 进一步暴露了角色 ownership、PR creator、commit authorship 和 reviewer evidence 被混为同一账号 gate 的问题；当时提出的“必须由 Implementation DRI account 创建 PR/预绑定不同 reviewer account”属于历史纠正方案，现已由更简单的规则取代：职责和 review evidence 分别真实记录，需要第二人 review 时保留独立 evidence；已有 PR 的后续 push account 必须等于 PR creator；
9. MIT `LICENSE` 已加入；第三方 notice 策略仍待收口；
10. GitHub 当前已有治理 issue [#11](https://github.com/jjjphens-dot/FRAZIL/issues/11) 用于跟踪 portable GitHub workflow 文档同步，以及 TESTDATA-001 rationale issue [#15](https://github.com/jjjphens-dot/FRAZIL/issues/15)；Milestones 为 0，Projects 为 0；Labels 页面仅见 GitHub 默认标签，项目自定义 labels 未建立；branch listing 当前报告 `main` 为 `protected:false`，更细粒度 ruleset / admin-level branch-protection configuration 尚未独立验证；HOST-000 产品目标已由 Sound & Host Lead 冻结，HOST-001 primary Live/FL evidence 已由该角色签认为 `Passed` 并由 Engineering Lead 在精确 PR #27 HEAD 正式批准；REAPER secondary matrix 延期；当前 PR、review 和 merge 状态只以 GitHub 为准，不据此推断不存在其它规则集。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.25+、C++20、Ninja presets | 本机可用；portable preset 已由 Hosted CI 验证 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、versioned state XML adapter | M1-C state boundary 已接入；XML createXml/fromXml restore path 已验证；Live/FL 的真实 DAW save/reopen、lane/inactive-value restore 已通过并获 Engineering Lead review；HOST-001 closed |
| Parameters | 9 个集中式 APVTS 参数静态注册；enabled ID 已使用 `.enabled` | M1 参数路径已接入；`PARAM-FREEZE-001` 是后续 M2/M3 -> M4 gate，不是 M1 Exit blocker |
| App | `ProcessSpec`、`EngineParameters`、`ParameterSnapshot`、`ParameterMapper`、`StateModel`、`AudioEngine::prepare/reset/process` | M1 gain/mix skeleton；STATE-001 versioned value/schema、known migration、invalid fallback、inactive retention；STATE-002 mode-value-retention integration verified；M5 EditHistoryManager remains planned；wet pass-through；runtime buffer invariant fallback |
| UI | `main@b595a47` 中的 Debug/ASAN DEV-UI-001 测试用控制面板；Release 保留静态占位界面 | 非产品 UI；interactive usability、DAW 和 M5 Production UI 未完成 |
| Developer sound tools | RENDER-001、TESTDATA-001、manual performance harness，以及 Debug/ASAN DEV-UI-001 的 effective controls、developer override、Dry/Processed、coherent bounded diagnostics、A/B slots 和 draft config export | implementation 已进入 `main`；正式 acceptance、debug bundle、Offline Sound Lab handoff 仍为 PLANNED |
| Product identity | `docs/PRODUCT_IDENTITY.md` | FRAZIL adopted working/product name；命名词汇不改变参数合同；法律/商标 clearance 不属于当前工程范围 |
| Tests | `frazil_smoke` + `frazil_unit` + `frazil_plugin_integration` + `frazil_processor_property` + `frazil_latency_contract` + `frazil_render` + `frazil_render_cli` CTest；`frazil_performance` manual benchmark | Debug/Release/ASAN 各 7/7 PASS；canonical PERF-BASE-001 已记录；Ableton/FL Studio primary HOST-001 matrix 已通过并获 Engineering Lead review |
| Local validation | PR #18 follow-up 的 Release、ASAN fresh configure、6-job safe build、7/7 CTest 和 Release manual benchmark 结果见 2.6，并按该节记录 exact tested implementation commit；当前 branch HEAD 与最新 CI 以 GitHub live query 为准 | 已验证；本机绝对路径仅在 ignored local configuration/build output，仓库 preset 保持可移植 |
| pluginval | `main@b595a47` 当前 Debug VST3 在配置的 Host 扫描副本上以 pluginval 1.0.4、strictness 5、seed 12345 运行并以 `SUCCESS` 结束；Steinberg validator 未配置而跳过 | 当前 artifact 已验证；不等于完整 DAW compatibility |
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

当前 TEST-002 与 ARCH-LAT-001 的 implementation commit 为 `e56664c8775f614e77004ac891d63a5c2fd2fa7a`；PERF-BASE-001 harness follow-up implementation commit 为 `98ea8c6a70edef917fc42cc48a25801fc613716b`。结果见 [`docs/evidence/M1_ENGINEERING_EVIDENCE.md`](evidence/M1_ENGINEERING_EVIDENCE.md) 与 [`docs/evidence/PERF-BASE-001.md`](evidence/PERF-BASE-001.md)。当前分支的 Release、Debug、ASAN 均通过 fresh configure、safe build 和 7/7 CTest；canonical `frazil_performance` manual Release benchmark 的 `configured_commit`/`configured_source_state` 与 `runtime_commit`/`runtime_source_state` 均为 clean matching，`formal_provenance_status=PASS`，并记录 steady-state/parameter-retarget、CPU、memory、allocation、finite-output 和 denormal observations；focused mutation verification 也证明 active lifecycle corruption、active valid-but-different output 和 neutral exact-repeatability corruption 能被分别验证。早期实现/证据 head `c1aea641688397f80bec0996e8eb3375b5f60def` 的 Hosted Windows Debug run `34481193756` 已通过 Configure、Build 和 7/7 Test；此前 docs-only validation head `c3bfeab4d55c47e322cad6ef7cfc259f6542b6dd` 的 run `34482138484`，以及 provenance-fix head `b8918ec93fe696867e16712e67837173b6cfceb4` 的 run `34490710722`，也均已通过 Configure、Build 和 7/7 Test。pluginval、真实 DAW、listening 和 M1 Joint Exit 不在本地/Hosted Debug 证据范围内，需后续 acceptance。

## 2.7 DEV-UI-001 engineering validation

2026-09-13 DEV-UI-001 closeout follow-up 本地工作树 evidence：Debug、Release 和 ASAN 均通过仓库 `tools/vscode_msvc_env.cmd` 初始化
MSVC 环境、`tools/vscode_build_safe.cmd --preset <name> --jobs 6` safe wrapper build，以及对应的
`ctest --preset <name> --output-on-failure` 7/7 CTest。ASAN 的默认 6-job preflight 在内存被清理前曾因
可用物理内存 2.76 GiB 而拒绝；清理内存后 6-job preflight/build 通过，未绕过安全检查。

初次增量测试运行触发 Debug CRT 的 `midi` 栈破坏提示；ASAN 随后定位为测试 translation unit 使用旧的
`FRAZILAudioProcessor` 类布局，而 `PluginProcessor.cpp` 已包含新增 `DeveloperDiagnostics` 成员。该构建
依赖失配在三个 processor 集成目标中强制重编后消失；重新配置并让 Debug/ASAN 测试目标显式使用开发宏后，
Debug、Release、ASAN 均 7/7 通过；新增的 `frazil_plugin_integration` targeted regression 在 Debug 连续 20 次、ASAN 连续 10 次运行均通过。当前证据支持“无复现的栈越界”，不把这次结果写成完整 UI usability 或
Host/DAW acceptance。

Final engineering follow-up 还通过 effective-state ownership、Host-change-under-override、state-restore、Editor
attachment/comparison-button decision-helper reconciliation、coherent diagnostics publication/readback、并发
publish/read、conditional active-publication token、确定性 stale-edit/Host-clear rejection、continuous set-only
no-Host-fallback、clear/session cache invalidation、reader-ready handshake、set/clear coherence pressure 和 complete
temporary A/B slot regression；其中 Editor attachment 回归是纯决策 helper 覆盖，不是实际 JUCE Editor/attachment
生命周期自动化。`DeveloperParameterOverride` 的 control-side set/clear/conditional commit 使用非实时 mutex 串行化，
audio read/apply 保持 lock-free、bounded，并使用 Processor-owned same-session last coherent fallback；clear/session
epoch 不允许旧缓存跨越 Host boundary。并通过 `python tools/check_markdown_links.py`、`python tools/check_portability.py`
与 `git diff --check`。Release isolation audit 确认 Debug/ASAN 使用 `FRAZIL_ENABLE_DEVELOPER_UI=1`，Release 使用 `0`，
且 `ParameterLayout` 不包含 Water candidate IDs。Dry/Processed 的代码路径和 finite/serialization boundary 已有
CTest evidence，但未执行当前变更后的 pluginval、真实 DAW/Host matrix、interactive usability review、实际 GUI
attachment lifecycle、Dry/Processed listening comparison 或完整 debug bundle。
该 section 只记录 DEV-UI-001 engineering evidence；它不等同于产品 UI、interactive usability 或完整 Host/DAW
acceptance。

## 2.8 HOST-001 primary DAW evidence

2026-09-14，Sound & Host Lead / 用户确认在 Ableton Live 12 Suite 12.4.2 与 FL Studio 2025
25.1.4.4951 中完成 acceptance index 的完整 primary-host scope：Group A、gain/continuous/discrete automation、
save/reopen、DAW render、H1-H7 及 Developer UI/Host restore boundary 均无异常并标为 `Passed`。
截图、DAW 工程和输出 WAV 未留存；这是 evidence-form limitation，不是测试失败。详见
[HOST-001 primary DAW evidence](evidence/HOST-001-DAW-SMOKE-2026-09-13.md)。

收尾 artifact 的工程、产物和逐 Host 结果集中记录在
[`HOST-001 / M1 acceptance index`](evidence/HOST-001_ACCEPTANCE_INDEX.md)；M1 gate-by-gate 结论见
[`M1 Joint Exit record`](evidence/M1_JOINT_EXIT.md)。Engineering Lead 已于 2026-09-14 正式批准精确 PR #27
HEAD `01d590f`；Hosted CI run 34864859407 通过，PR #27 合入 `main@3438593`。HOST-001 已关闭，Live/FL
达到本次记录范围内的 `Development Validated`，M1 Joint Exit 已批准。

当前 Debug artifact 另有 pluginval 1.0.4 strictness 5 / seed 12345 `SUCCESS` 观察；这不替代
RENDER-001 确定性回归或 Water/Ice 声音产品验收。REAPER 作为 secondary host 延期，
仍为 `Not run` 且不形成支持声明。

## 2.9 Diagnostics GUI candidate

2026-09-16，DEV-UI-001 diagnostics GUI follow-up 已形成本地 **implementation candidate / human usability
acceptance pending**。呈现职责移至 `src/ui/DeveloperDiagnosticsView.*` 和 `DeveloperLevelMeter.*`：aggregate
INPUT/OUTPUT 使用 RMS 填充、当前 Peak 竖线和 dBFS 数字，图形范围 -60 至 0 dBFS；保留 runtime metadata 和
finite 与 effective `Route:`，10 Hz 刷新，无 smoothing、history 或 Peak Hold。Diagnostics 位于左侧参数
网格下方；Water Size/Motion 保持既有 main 基线。人工 usability review 要求稍增 workflow 按钮：区域
由 96 增至 108 px，4x3 排列、间距及行为不变；整体仍为 implementation candidate，human usability review ongoing。

本次验证（不替代 §2.8 的历史 Host evidence）：

| Check | Actual result / boundary |
|---|---|
| Debug / Release / ASAN | 每个 preset 均执行 `cmake --preset <preset>`、`python tools/build_safe.py --preset <preset>`、`ctest --preset <preset>`；安全构建 PASS，CTest 各 7/7 PASS，全部 pipeline 串行 |
| Developer/Release isolation | compile commands 确认 Debug/ASAN macro=1 并编译呈现组件；Release macro=0 且无呈现组件源文件；实际 Release Standalone 保留 `Production editor pending` |
| Debug / ASAN Standalone | 实际启动并观察 runtime、两个 meter、finite；静音 Peak/RMS 均显示 `-inf dBFS`、无 RMS 填充；ASAN 启动/观察期间未报告 sanitizer error。ASAN GUI 运行所需 runtime DLL 仅放在 ignored artifact 目录 |
| Layout | 1000x720 默认、820x680 最小和 1200x800 较大 editor 图像已观察；最小尺寸各区域无重叠；既有 `Processed` 标签在最小尺寸仍显示省略号，本轮仅调整高度，文字适配留待单独处理；Water geometry、diagnostics 布局和窗口限制不变，workflow 按钮实际高度由 28 增至 32 px |
| Synthetic signal | 临时本地预览入口使用实际 Processor/Editor，48 kHz、prepared 512/latest 480、双通道 1 kHz 正弦；输入 peak 0.25、RMS 0.176777 对应 -12.0/-15.1 dBFS；output gain -12 dB 后 meter 显示 -24.0/-27.1 dBFS；over-range 保留 +12.0 dBFS Peak，并饱和图形/显示警示 |
| Effective routing | Host `Water -> Ice` / `Ice -> Water` 后读取正确；Capture A / Apply A 后底层 Host 保持 `Ice -> Water`，`Route:` 显示 effective `Water -> Ice`；Return Host 恢复 `Ice -> Water` |
| Editor reopen | 同一预览 Processor 关闭/重建 Editor，序列化 state 内容相同，重开后的输出 peak/RMS 不变；这是本地 fixture observation，不是 DAW save/reopen acceptance |
| Warning fixture | 仅向独立 view 注入 `finite=false` / invalid amplitude，观察 `FINITE NO` / `Peak INVALID` / `RMS INVALID`（无 dBFS 后缀）；不宣称真实 audio fault capture |
| pluginval | 1.0.4，Debug VST3，`--strictness-level 5 --random-seed 12345 --validate <Debug VST3>`，`SUCCESS`；包含 Editor / Open editor whilst processing / Editor Automation；Steinberg validator 未配置 |
| Scope audit | Processor、diagnostics transport、ParameterLayout、state adapter、app/DSP、existing tests 无 diff；9 个 Host 参数、state/automation、Dry/Processed 与 export 实现未改 |

临时预览入口在常规回归前已移除，未提交额外 GUI framework；日志保留在 ignored 本地
`build/dev-ui-review/`；review 截图附于 [PR #31](https://github.com/jjjphens-dot/FRAZIL/pull/31)，不提交到仓库。
截图和合成信号观察不等于独立 human acceptance、实时峰值完整性或 DAW evidence。
本次未重跑真实 DAW matrix、听测或 CPU benchmark，也未实现 L/R、true peak、LUFS、FFT/waveform、可靠跨 block
Peak Hold、clip history 或 Production UI。正式 Engineering / Sound & Host review 和最终 usability acceptance
仍待完成。

## 3. 当前源码映射

```text
PluginProcessor
  ├─ owns APVTS
  ├─ uses src/plugin/ParameterLayout for static parameters
  ├─ caches APVTS raw parameter atomics
  ├─ captures one ParameterSnapshot per processBlock
  ├─ maps to EngineParameters
  ├─ saves/restores versioned state through Host State Adapter -> StateModel
  └─ Current-main Debug/ASAN implementation may apply a developer-only override and Dry/Processed mode before AudioEngine
     `process(buffer, engineParameters, dryReferenceOnly)`

AudioEngine
  ├─ input gain -> dry reference/wet skeleton -> global mix -> output gain
  └─ owns prepare-time dry scratch and continuous smoothers

PluginEditor
  ├─ Current-main Debug/ASAN implementation: Host/Water experiment controls, non-APVTS A/B/reset, Dry/Processed and draft config export
  ├─ Diagnostics GUI follow-up candidate: value-fed src/ui/DeveloperDiagnosticsView -> DeveloperLevelMeter
  └─ Release: static non-developer placeholder
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

- 正向事实：`src/plugin`、`src/app`、`src/dsp`、`src/ui` 目录边界已经存在；当前未发现 mutable global runtime state；AudioEngine 的运行状态由实例成员持有；`JuceHeader.h` 目前局限在插件适配层；Debug/ASAN 的开发 editor 通过 bounded atomic diagnostics snapshot 读取 runtime 信息，Release 编译掉 callback diagnostics publication path。
- 已确认技术债：`PluginProcessor` 仍公开 APVTS，后续需要收窄 Host parameter interface；公开兼容性 freeze 与 history integration 尚未完成；wet path 仍为 pass-through，Water/Ice/Routing 尚未实现。
- 有意保留的未实现项：Water、Ice、Routing、EditHistoryManager、完整 render regression matrix、DEV-UI usability
  acceptance、正式 UI 和离散 transition 均仍按 Coding Plan 处于计划阶段；current-main Dry/Processed 仅是
  developer workflow infrastructure，不是 production semantics；当前 RENDER-001 只覆盖 M1 pass-through offline
  smoke；本次状态工作不提前创建声音算法或 history 生产依赖。
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
| Input trim | `input.gain` | `input.gain` | 保留；基础 DSP/continuous smoothing 已接入；Live/FL automation/state/Host matrix 已通过并获 Engineering exact-HEAD review；不形成 official-support claim |
| Global dry/wet | `global.mix` | `global.mix` | 保留；基础 DSP/continuous smoothing 已接入；Live/FL automation/state/Host matrix 已通过并获 Engineering exact-HEAD review；不形成 official-support claim |
| Output trim | `output.gain` | `output.gain` | 保留；基础 DSP/continuous smoothing 已接入；Live/FL automation/state/Host matrix 已通过并获 Engineering exact-HEAD review；不形成 official-support claim |

虽然 M1 参数合同已随 PR #3 合入 `main`，但在正式参数 freeze、state migration 和 compatibility evidence 前，不得创建公开 preset/session 兼容性承诺。若已有外部用户使用过当前占位构建，应先确认是否需要兼容别名/迁移。

## 5. 现状对应 milestone

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证、首次 push 和 Hosted CI success 已验证；HOST-000 产品目标矩阵与产品身份文档已记录，但 official-support gate、完整 release compatibility、GitHub metadata 与 branch protection 尚未收口。HOST-001 已关闭，不再阻塞 M1 Exit。
- M1 Audio Skeleton & Parameter Contract：**Exit approved / 完成**。M1 engineering foundation/evidence、closeout artifact 的 Debug/Release/ASAN、exact Debug artifact strictness-5 pluginval、Sound & Host `Passed` 的 Ableton/FL primary matrix、Engineering Lead exact-HEAD approval、Hosted CI 与 PR #27 merge 均已完成。REAPER 为延期的 secondary host，不阻塞 M1 且不形成支持声明。完整未来 render matrix 仍按原计划推进；`PARAM-FREEZE-001` 仍是 M2/M3 后续 gate；M5 EditHistoryManager 仍未开始。
- Developer Sound/Debug Tooling：**DEV-UI-001 implementation merged / acceptance 进行中**。当前 `main` 的
  Debug/ASAN 控制面板已覆盖当前 9 参数控制、Water experiment-only controls、non-APVTS A/B/reset、
  developer Dry/Processed path、prepared/latest diagnostics 和完整 draft config representation。它不是
  M5 Production UI；usability、DAW/listening evidence 和 Offline Sound Lab handoff 仍未完成。它是大规模
  `EXP-W-002` 前的 Water M2 effective-development-readiness prerequisite，但不是 M1 Exit hard gate。
- M2 Water：本次 EXP-W-001 收口未启动 EXP-W-002 或 Water production DSP；后续启动仍须核验适用 Developer workflow readiness。
- Perceptual Contract framework/template：**CURRENT / CONTROLLED**。2026-09-18，[EXP-W-001 Revision B 四 macro 感知合同](../experiments/water/EXP-W-001_PERCEPTUAL_BRIEF.md) 已记录 **ACCEPTED（definition only）**：Human ACCEPT at b616533、[工程初审 PASS at a5a0d99](https://github.com/jjjphens-dot/FRAZIL/issues/17#issuecomment-5701188940)，以及用户转述 Engineering Lead 口头认可并要求完成收口。后者是 manual evidence，不是 formal GitHub APPROVE；口头审阅的精确 revision/date 未提供。brief 第 11/12 节保留证据限制及未执行听测的下游归属；实际 PR 合入/Issue 关闭以 [Issue #17](https://github.com/jjjphens-dot/FRAZIL/issues/17) 的 GitHub 记录为准。
- M3 Ice：**DEFERRED**；长期 milestone 保留，在 `M2 Exit + Explicit Joint Gate` 确认 Water workflow 可复用于 Ice 前，不启动 Ice experiment、perceptual/parameter redesign 或 production implementation。
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
  -> maintain merged M1 evidence only for regressions/findings

Sound / Host lane
  -> HOST-001 Ableton + FL Studio primary evidence complete
  -> REAPER secondary validation deferred; no support claim
  -> EXP-W-001 Perceptual Contract; no candidate or production DSP
  -> DEV-UI-001 workflow/product usability acceptance
  -> LISTENING-001 representative corpus preparation (parallel; does not block M1 Joint Exit)

Post-M1 Water readiness
  -> M1 Joint Exit complete
  -> DEV-UI-001 usable before large-scale EXP-W-002
  -> EXP-W-002 engineering experiments using TESTDATA-001
  -> EXP-W-003 Water dual-mode validation/refinement after LISTENING-001 is ready
  -> WATER-006 listening evidence
  -> Water production only after experiment and Joint Gate
  -> after M2 Exit, use an Explicit Joint Gate before resuming M3 Ice
```

HOST-000 产品目标已冻结；Live/FL 已满足本轮 `Development Validated` 条件，但 `Officially Supported`
仍需后续 release compatibility/support gate，REAPER 无支持声明。
已进入 `main` 的 M1 foundation/evidence 只按 regression/finding ownership 维护，不得建立平行实现。
Debug/ASAN Developer UI 与 diagnostics implementation 已进入 `main`，但不得从本状态文档推断为产品
UI 或已完成 usability/DAW acceptance；Water-specific Perceptual Contract instance 仍不得推断为已实现；
Perceptual Contract framework/template 已在 v1.3 approval + merge 后成为 CURRENT/CONTROLLED。
Water/Ice/Routing 仍未开始 production。

## Water objective engineering feasibility

A standalone bounded `SPIKE-W-DSP-001` exists under `experiments/water/SPIKE-W-DSP-001/`.
It contains A/B/D/C research mechanisms, deterministic Fluid ablation, engineering JSON configs,
offline rendering and preliminary performance measurement. Its controlled scope is objective
feasibility before the accepted brief, not production Water DSP or perceptual acceptance.
It does not close EXP-W-002. Brief integration and human listening remain outstanding with
Sound & Host Lead. Current validation and limitations are recorded in the
[research checkpoint](../experiments/water/SPIKE-W-DSP-001/README.md).

## Standalone Water preview implementation candidate

An opt-in research executable connects WAV playback and explicit engineering controls to SPIKE-W-DSP-001
A/B/D/C. It reuses the developer diagnostic view without changing the FRAZIL plugin, nine Host parameters,
M1 wet path or plugin state. Draft/apply/restart preserves prepare-time DSP configuration; temporary A/B,
monitor comparisons and renderer-compatible module export support Sound Lead engineering inspection.
Production Water integration, accepted product macro mappings and final workflow/perceptual acceptance
remain pending. Actual validation and limits are recorded in
[WATER_PREVIEW_VALIDATION](evidence/WATER_PREVIEW_VALIDATION.md); usage in the
[debugging guide](DEV_UI_WATER_DEBUG_GUIDE.md).

The staged [Water UI control bridge](evidence/WATER_UI_CONTROL_BRIDGE_EXECUTION.md) has reconciled
Preview and Protect on a separate integration branch and added isolated time-value tooling tests.
Shared session state and Sound Lead/Engineering views are implemented with Model/composition sync,
research Size/Fluid Motion/Decay mappings and Modal excitation-weight Motion (legacy imports stay unmapped), edit provenance, retained
inactive controls and applied A/B state. The listening-ready follow-up is tracked in the
[new execution record](evidence/WATER_LISTENING_UI_EXECUTION.md). Decay
uses provisional `0.5` and participates in reset and separate session copy/export/import. Strict exact
entry, adaptive time units, collapsible module cards and draft details are implemented. Session source/build
provenance, module import and module-specific validation feedback are implemented and regression-tested.
Protect is connected only in the research preview: Depth/Enable are live, configuration edits require
Apply, D0/D1 retain separate calibration, and C permits only Whole. Existing Protect DSP is reused;
no new production or human acceptance is claimed. Sound Lead Auto Audition and bounded operation history are implemented; supplied-input handoff and human mapping/audibility decisions are tracked separately in the listening UI execution record.

The research listening follow-up completed phases A-I and generated the Phase J fixed-source pack: 72 cases across four supplied inputs pass finite/repeat/partition/isolation checks. [Handoff](evidence/WATER_LISTENING_HANDOFF_V02.md#historical-v01-handoff) explicitly retains low-level Resonant Motion as a candidate risk; human ACCEPT/REVISE/REJECT, independent code review and production adoption are not complete.

### PR #40 listening remediation in progress

The [remediation record](evidence/WATER_LISTENING_REMEDIATION.md) tracks the no-op interaction fix,
Motion v0.2 endpoint / legacy-session migration, monitor over-range warning and normalization
study. C1/C2 are not adopted: the supplied Stop A requires numerical/design review. This work
must not be read as completed Resonant audibility, accepted mapping or product DSP adoption.

The next-stage [EXP-W-RX-001](../experiments/water/EXP-W-RX-001.md) compares bounded modal
excitation separately from the raw preview default and C0 normalization. An actual-driver
Engineering audition is implemented; validation and candidate selection remain in the phase
record. No human acceptance, C3 adoption or completed Water listening readiness is implied.


### Next-stage candidate engineering handoff (PR #40 branch)

The [phase ledger](evidence/WATER_DSP_LISTENING_EXECUTION.md) records bounded-carrier comparison,
C3 induced-response-capped normalization, structured Modal Motion, Fluid balance comparison,
separate continuous Droplet activity and monitor-only actual component/driver diagnostics.
These are explicit research candidates; raw/C0/independent and the v0.2 macro defaults remain.
Session v5 stores28 engineering targets with conservative v1-v4 import. Two90-case packs
(Hard/C3 and Feature/C3) passed finite/repeat/partition/isolation, with separate RMS support and
[blank independent review forms](evidence/WATER_CANDIDATE_LISTENING_REVIEW.md). The fifth input
is a generated engineering pad; representative musical testing is deferred by the user.

Latest local Debug/Release/ASAN each pass25/25; native GUI covers candidate Apply, diagnostics
and descriptor-group visibility. Two earlier intermittent renderer failures remain unresolved
observations, despite later passing suites; no root-cause fix is claimed. Human audibility,
macro semantics, baseline acceptance/revision and subsequent Protect listening are NOT ASSESSED /
NOT RUN. Conditional Flow D1/D2 and Droplet B2 were not triggered. This is unmerged PR-branch
engineering evidence, not current-main adoption, formal Host validation or Water readiness.
