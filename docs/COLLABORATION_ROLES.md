# FRAZIL 双人协作、Ownership 与交接规则

> 状态：Maintained collaboration guide<br>
> 适用阶段：M0 -> v1.0<br>
> 当前角色映射：Engineering Lead = `Aspartameqwq`；Sound & Host Lead = `jjjphens-dot`<br>
> 单个工作项的 assignee、状态和截止信息：仅在 GitHub Issues/Project 中维护

## 1. 协作模型

FRAZIL 使用两条互补的责任链：

> Engineering Lead owns how production software is engineered correctly.
>
> Sound & Host Lead owns whether it works correctly as a musical product and inside real DAWs.

角色映射描述长期职责领域，不等同于 GitHub 登录账号、commit author、PR creator 或 reviewer。
每个工作项仍必须单独指定 Implementation DRI 和 Acceptance DRI，不能用“谁有空谁继续做”替代稳定
ownership，也不能把 Water 和 Ice 长期切成两个互不审查的 production 技术孤岛。

每个 Ready 工作项必须具备：

- Implementation DRI：对约定 scope 内的实现、测试、文档和修复闭环负责；
- Acceptance DRI：提供输入并独立验收，不默认进入对方的 production implementation；
- 明确的 problem/goal、scope/non-goals 和 acceptance criteria。

Write ownership、Allowed/Forbidden paths、双方 inputs/outputs、handoff condition 和 Joint Gate 是风险触发
字段：cross-module、production DSP、contract/ownership-sensitive、Host acceptance 或 milestone gate 工作必须
填写；普通 bounded task 不需要为这些字段承担固定管理成本。

## 2. Ownership 术语与防重合规则

### 2.1 Implementation DRI

Implementation DRI 可以在约定 scope 内编写实现、测试和同 PR 文档；敏感任务还须遵守明确的 Allowed
paths。DRI 负责回应 finding，并说明未执行验证。
DRI 是工作责任，不要求与 PR creator、push account、commit author/committer 或 reviewer 使用同一身份。

### 2.2 Acceptance DRI

Acceptance DRI 负责确认验收输入、复现证据、记录 finding 和作出 acceptance decision。正常闭环为：

```text
Implementation DRI
  -> implementation and self-validation
  -> Acceptance DRI review / validation
  -> finding
  -> Implementation DRI fix
  -> Acceptance DRI re-validation
```

Acceptance DRI 默认是 owner 的实质 production implementation 的 `Review only`，不得为了快速修复而接管
对方 feature。`Review only` 允许 read、comment、run tests、reproduce、create findings、提出概念性 patch
建议和提交 acceptance evidence。

Primary ownership 不是 filesystem ACL。另一角色可以完成当前 scope 明确包含的 typo、one-line label fix、
小型 test-only correction、documentation sync 或 trivial integration correction，只要它不创建/替换 production
implementation、不改变 Joint Gate contract，且 primary owner 可以 review。此类 bounded fix 不需要 DRI
Transfer。

### 2.3 Implementation DRI transfer

只有 substantial implementation responsibility 确实换人时，才在 Issue/PR 记录以下内容：

```text
Implementation DRI Transfer

Previous DRI:
New DRI:
Reason:
Scope transferred:
Allowed / forbidden paths after transfer:
Effective point:
Open findings and evidence state:
```

DRI transfer 不自动改变已有 PR 的 creator 或 push-account rule。若新 DRI 不能通过现有 PR creator
继续 push，应显式决定由原 PR creator 继续管理 push，或关闭旧 PR 后以新 ownership 建立 branch/PR；不得
静默跨账号 push、伪造 author 或 rewrite history。

typo、小型 bounded fix、test-only correction 或 documentation correction 不触发 DRI Transfer。

## 3. 长期角色职责

### 3.1 Engineering Lead

- C++20、JUCE、CMake、CI、测试框架、build portability 和开发工具；
- `plugin -> app -> dsp` 边界、公共接口、对象所有权和生命周期；
- audio-thread realtime safety、预分配、smoothing、transition 和 finite output；
- ParameterLayout、ParameterSnapshot、ParameterMapper、StateModel 与 AudioEngine；
- production Water/Ice/Routing DSP 的工程实现；
- unit/property/integration/render harness、ASAN、pluginval 和性能测量；
- `DEV-UI-001` Developer Control Surface、realtime-safe diagnostics bridge 和 experiment-config export；
- production code quality、公共 API 注释、module README 与 MODULE_INDEX 同步。

Engineering Lead 不单独决定最终听感、macro 产品语义、DAW 工作流或 UI 产品优先级，也不能为实现方便
改变参数、state、routing、latency 或 random persistence 合同。

### 3.2 Sound & Host Lead

- Water/Ice perceptual brief、声音参考、反例、reject criteria 和 listening rubric；
- listening/reference 素材、使用场景、许可输入和 representative workload；
- candidate A/B、loudness match、独立评分与 accept/revise/reject；
- 参数名称、默认值、范围、macro 语义和极端行为的产品验收；
- Ableton、FL Studio、REAPER 中的 scan/load、参数枚举、automation、save/reopen 和 offline render；
- routing、enable、Global Mix、Input/Output Gain 的真实制作体验；
- UI 信息层级、操作速度、视觉信号流和产品验收；
- `DEV-UI-001` 的 workflow/product usability acceptance，包括参数可发现性、A/B、Reset、diagnostics 和
  offline experiment handoff；
- 用户文档、已知限制和音乐制作示例。

Sound & Host Lead 不单独决定 production audio-thread 安全、跨模块依赖、对象生命周期或 state migration
技术方案，也不能让未通过 property/performance gate 的算法进入 `src/dsp/`。

## 4. File / Path write ownership

下表描述 default responsibility，用于防止两人平行或实质性接管 production work，不是 strict filesystem
ACL。高风险 Issue 可以用 Allowed/Forbidden paths 收窄写入边界；普通 bounded task 以明确 scope 为准。

| Path / domain | Primary writer | Other developer |
|---|---|---|
| `src/app/**` | Engineering Lead | Review only |
| `src/plugin/**` | Engineering Lead | Host finding / review only |
| `src/dsp/**` | Engineering Lead | Sound acceptance / review only |
| `tests/unit/**` | Engineering Lead | Review / scenario input |
| `tests/dsp/**` | Engineering Lead | Review / listening-risk input |
| `tests/integration/**` | Engineering Lead | Host scenario input / review |
| `tests/render/**` | Engineering Lead | Consume / review |
| `tools/*render*`, `tools/*perf*` | Engineering Lead | Consume / usability finding |
| Developer Control Surface、diagnostics bridge、experiment-config export | Engineering Lead | Sound & Host Lead owns workflow/product usability acceptance |
| CMake、build scripts、`.github/workflows/**` | Engineering Lead | Reproduction review |
| `testdata/input/**`、`testdata/manifest.json`、`tools/*testdata*` engineering corpus | Engineering Lead；accepted 后冻结 | 修改需显式 Issue；Sound & Host Lead 提供需求/许可 review |
| `testdata/listening/**` | Sound & Host Lead | Engineering validation |
| Host/DAW evidence | Sound & Host Lead | Engineering reproducibility review |
| perceptual briefs、listening rubric/notes | Sound & Host Lead | Engineering feasibility review |
| production Water/Ice/Routing DSP | Engineering Lead | Sound / Host acceptance |
| Parameter/state/routing contract | Joint Gate | Joint Gate |
| Accepted ADR decision | Joint Gate | Joint Gate |
| formal performance budget | Joint Gate | Joint Gate |

`AGENTS.md`、治理文档和 PR template 的修改按其 Modification Policy 执行。Primary ownership 不能覆盖
LOCKED/CONTROLLED 文档要求，也不允许 parallel production implementation、静默接管另一 owner 的 feature、
跨 ownership boundary 的 substantial refactor，或未经批准改变 Joint Gate decision。

## 5. M0 与 M1 ownership

### 5.1 M0 剩余职责

| Work | Implementation DRI | Acceptance DRI | Scope boundary |
|---|---|---|---|
| `GH-001` labels/milestones/Project | Engineering Lead | Sound & Host Lead | 只处理协作 metadata，不改产品合同 |
| `GH-002` ruleset/branch protection | Engineering Lead | Sound & Host Lead | 只处理 repository governance |
| CI/build/portability/dependency regression | Engineering Lead | Sound & Host Lead | 复现入口 review；不重做 Accepted JUCE dependency strategy |
| HOST compatibility content/support classification | Sound & Host Lead | Engineering Lead | 不把未执行 DAW evidence 写成支持事实 |
| audio/reference licensing product review | Sound & Host Lead | Engineering Lead | 不修改 frozen engineering corpus，除非显式 Issue |

### 5.2 M1 已建立、只做 regression/finding follow-up

`STATE-001`、`STATE-002`、`AUTO-001` foundation 和 `TESTDATA-001` original
reproducibility/provenance infrastructure 已进入 `main`。`RENDER-001` 的 pass-through offline smoke
也已进入 `main`。TESTDATA-001 diagnostic semantic refinement 仍归同一 Engineering Lead ownership，
在其 GitHub merge state 确认接受前，按 finding-driven follow-up 处理；不得把它描述为平行 TESTDATA
实现，也不得另建平行 corpus 或 render harness。后续只允许由相关 regression、兼容性 finding 或明确
新 work item 驱动的修改；尚未满足的 render matrix/acceptance 继续由同一 ownership 收口。

### 5.3 M1 剩余工作矩阵

| Work item | Implementation DRI | Acceptance DRI / inputs | Allowed paths | Forbidden paths / non-goals |
|---|---|---|---|---|
| `RENDER-001` acceptance/follow-up | Engineering Lead | Sound & Host Lead 提供使用性 review | 既有 render target、`tools/*render*`、`tests/render/**`、相关 docs | Water/Ice/Routing、listening selection、DAW acceptance |
| `TEST-002` | Engineering Lead | Sound & Host Lead 提供风险场景 | property/unit/integration tests 与必要 test support | 新声音算法、UI、routing production |
| `PERF-BASE-001` | Engineering Lead | Sound & Host Lead 提供实际 workload、素材、block size、制作场景 | performance harness、reports、必要 build/test wiring | 正式性能预算、Water/Ice/Routing/UI |
| `ARCH-LAT-001` | Engineering Lead | Sound & Host Lead 验收 intentional delay/tail 产品语义 | latency metadata、impulse/automated evidence、相关 ADR/docs | 引入 lookahead/FFT/convolution 或更改 routing 实现 |
| `HOST-001` | Sound & Host Lead | Engineering Lead 提供 build/plugin 与 finding 修复 | Host evidence、DAW matrix/results、Issue findings | 直接修改 StateModel、Mapper、Snapshot、AudioEngine、PluginProcessor |
| `DEV-UI-001` | Engineering Lead | Sound & Host Lead 提供 workflow/product usability acceptance | developer controls、safe parameter binding、diagnostics bridge、experiment-config export、相关 docs/tests | Production UI、preset system、Host registry/state adoption、Water production DSP |
| M1 Joint Exit Review | Sound & Host Lead 维护 Host evidence；Engineering Lead 维护 engineering evidence | 双方 Joint Gate | exit evidence 和 review record | 用单条证据链替代另一条 |

### 5.4 HOST-001 handoff

Sound & Host Lead 按 Ableton -> FL Studio -> REAPER 推进，每个 DAW 至少执行：

- scan、load、unload、reload；
- 九个 Host-visible parameters 的 name/order/range/default/choice/automation visibility；
- automation lane create/record/edit/playback/save/reopen；
- 非默认参数 project save/close/reopen/value restore；
- DAW offline render 和明确的环境/步骤/结果记录。

M1 wet path 仍为 pass-through 时，`global.mix` 不改变声音不是未接入的充分证据；重点检查 Host state、final
value、lane behavior、retention 和 reopen。发现 engine/state/plugin 问题时创建 finding，包含复现步骤、期望、
实际结果和 evidence，然后 hand back 给 Engineering Lead；验收方不直接进入上述 production implementation。

M1 只有在 Engineering Evidence 与 Sound / Host Evidence 两条链均满足后，才能进入 Joint Exit Review。
工程测试不能替代 DAW acceptance，DAW 中“听起来正常”也不能替代自动化工程证据。

`HOST-001` 不依赖 `DEV-UI-001`。即使 Developer Control Surface 已可用，Host acceptance 仍必须从 DAW
generic parameter interface、automation lane、state restore 和 save/reopen 取得证据。

## 6. Water-first experiment and production ownership

当前采用 Water-first：先稳定 Perceptual Contract、Developer Control Surface、Offline Sound Lab、Water
experiment/evidence/ADR 方法，再恢复 Ice。Ice 的长期 M3 ownership 和 gate 保留，但当前不并行启动
Ice experiment、perceptual/parameter redesign 或 production implementation。production C++ ownership 仍不按
材质拆给两人各自孤立实现。

### 6.0 LISTENING-001 shared preparation

| Work item | Implementation DRI | Acceptance DRI | Allowed paths | Forbidden paths / non-goals |
|---|---|---|---|---|
| `LISTENING-001` | Sound & Host Lead | Engineering Lead | `testdata/listening/**`、listening corpus metadata、license/provenance evidence、directly related listening docs | `testdata/input/**`、TESTDATA generator/verifier、production Water/Ice DSP、HOST-001 DAW evidence、parameter/state contracts |

`LISTENING-001` 可以在 M1 期间开始准备，但不阻塞 M1 Exit。当前必须在 `EXP-W-003` Water dual-mode
validation/refinement 与 `WATER-006` listening evidence 前 ready；`EXP-W-002` engineering experiments
可以直接使用 `TESTDATA-001`，不需等待 listening corpus 完成。Ice 对应使用要求在 M3 恢复时仍成立。
DAW compatibility/automation/save-reopen evidence 始终归 `HOST-001`。

### 6.1 Experiment pipeline

| Stage / work item | Implementation DRI | Acceptance / Joint Gate | Output boundary |
|---|---|---|---|
| `EXP-W-001` Perceptual Contract | Sound & Host Lead | Engineering Lead feasibility review | intent、positive/negative、preserve/reject、references、objective proxies and listening dimensions |
| `EXP-W-002` candidate experiment | Engineering Lead | Sound & Host Lead owns question/target/fixtures/A-B/rubric inputs | experiment-only DSP、fixed seed、render、engineering measurements |
| `EXP-W-003` Water dual-mode validation | Sound & Host Lead | Engineering Lead realtime/latency/CPU/random/maintainability gate；final adoption = Joint Gate | loudness-matched review、rubric、accept/revise/reject、macro direction |
| Water algorithm ADR | Engineering Lead records technical decision | Joint Gate | production structure、mapping、latency/tail/random/performance/failure modes |
| `EXP-I-*` / Ice ADR | Same role split when resumed | Joint Gate | DEFERRED until the Water method is stable |

禁止两位开发者在没有显式 experiment scope 时，各自实现竞争的 production candidate。实验 code 必须留在
`experiments/`，通过 Joint Gate 和 ADR 后才进入 production work item。

### 6.2 Production ownership

| Work items | Implementation DRI | Acceptance DRI |
|---|---|---|
| `WATER-001/002/003/004/005/007/008` | Engineering Lead | Sound & Host Lead |
| `WATER-006` listening pack | Sound & Host Lead | Engineering Lead evidence review；final algorithm remains Joint Gate |
| `ICE-001/002/003/004/005/007` | Engineering Lead | Sound & Host Lead |
| `ICE-006` listening/differentiation pack | Sound & Host Lead | Engineering Lead evidence review；final algorithm remains Joint Gate |

Sound & Host Lead 默认不修改 WaterProcessor/IceProcessor production implementation。Engineering Lead
不得用工程可行性替代 macro semantic、musical usefulness、Water-vs-Ice differentiation 或 DAW acceptance。
这不禁止 Sound & Host Lead 在 `experiments/**`、small scripts、parameter sweeps、fixtures 和 prototype
exploration 中学习或试验 DSP；只禁止未经 scope/Joint Gate 进入平行 production implementation。

### 6.3 当前执行波次

```text
Current M1 late-stage
  Engineering Lead: DEV-UI-001 + handed-back HOST findings
  Sound & Host Lead: HOST-001 + EXP-W-001 + Developer UI usability acceptance

Water experiment
  Engineering Lead: Water candidates + Offline Sound Lab evidence
  Sound & Host Lead: Water rubric + listening decision

Water production
  Engineering Lead: accepted Water implementation
  Sound & Host Lead: Water final acceptance

Ice resume (deferred now)
  Apply the same contract -> experiment -> evidence -> ADR -> production method
```

团队 WIP <= 2；同一时间最多一个高风险 production DSP implementation。能力交叉通过 review、复现和
experiment 完成，不通过越过 path ownership 同时修改同一 production feature 完成。

## 7. Joint Gate

以下事项必须双方共同决定：

- Parameter IDs、ordering、choice indices、ranges、defaults、macros 和 `PARAM-FREEZE-001`；
- Water 和 Ice production algorithm adoption；
- routing semantics、StageMixer/crossfade law 和 `ADR-R-001`；
- latency/tail/random persistence semantics；
- formal performance budget；
- Beta 和 Release go/no-go。

意见不一致时记录双方证据和最小验证实验，工作项进入 Blocked；任一角色不得覆盖另一领域的阻断意见。

Joint Gate 只由上述合同变化触发。普通 bug fix、test coverage、render harness maintenance、performance
measurement implementation、DAW evidence collection、docs update 和不改变合同的 bounded implementation
detail refactor 不需要 Joint Gate。

## 8. Issue / PR task contract

Ready 必填：

```text
Stable ID / Milestone:
Implementation DRI:
Acceptance DRI:
Problem / sound or user goal:
Scope:
Non-goals:
Engineering acceptance:
Sound / Host acceptance:
```

以下字段只在 cross-module、production DSP、contract/ownership-sensitive、Host handoff 或 milestone gate 时
要求；bounded single-module task 可以写 `N/A — bounded scope` 或留空：

```text
Write ownership:
Allowed paths:
Forbidden paths:
Inputs owned by Acceptance DRI:
Outputs owed to Acceptance DRI:
Handoff condition:
Joint-gate decisions:
Related contract / ADR:
Documentation impact:
```

不得让 agent 把模糊目标扩成跨模块重构；也不得把条件字段当作每个小任务的固定 ACL/checklist。

## 9. Review 与 GitHub 身份

以下身份必须真实、分别记录，但不要求相同：

- Implementation DRI / Acceptance DRI：工作 ownership；
- PR creator / author：创建 GitHub PR 的账号；
- push account：执行当前 push 的已认证 GitHub 账号；
- commit author / committer：实际 authorship/commit metadata；
- reviewer：承担 review responsibility 的人和实际提交 review 的账号。

`PR creator == Implementation DRI`、`commit author == PR creator` 或 reviewer 与预设角色账号绑定都不是
项目 gate。唯一严格的账号一致性规则是：向已有 PR 对应分支继续 push 时，当前 authenticated push
account 必须等于该 PR creator；不一致则停止 push 并检查是否登录了错误账号。完整流程见
[GITHUB_WORKFLOW.md](GITHUB_WORKFLOW.md)。

Reviewer 关注 independence、scope、evidence 和 decision。parameter/state contract、core DSP、Water/Ice
algorithm adoption、routing、realtime boundary、latency、random semantics、formal performance budget、
Beta/Release 和 milestone exit 使用完整 Reviewer/scope/reproduced/not-reproduced/findings/decision 记录。普通
docs、bounded test、typo、narrow tooling 或 low-risk maintenance 只需 Reviewer、Decision 和 notable
limitations/findings。平台无法记录 formal review 时，可以使用明确标注的 comment/manual evidence，但不得
把 comment 写成 formal `APPROVE`。

## 10. 标准流程与反模式

1. Contract Review：确认 scope、primary ownership 和与风险相称的第 8 节字段/ADR trigger；
2. Implementation：DRI 在 bounded scope 内实现；敏感任务遵守明确 paths，团队 WIP 不超过 2；
3. Functional Validation：工程侧运行自动测试/性能，声音侧运行 listening/DAW/automation/state/UI；
4. Code Quality Review：检查边界、ownership、realtime 和可维护性；
5. Comment & Documentation Pass：同步注释、module README、MODULE_INDEX 和受影响合同；
6. Final Validation：Acceptance DRI 复现关键证据，finding 回到 Implementation DRI 闭环；
7. GitHub lifecycle：按 PR creator / push-account rule 管理后续 push 和 review evidence。

禁止以下反模式：

- 两人同时修改同一 production feature，却没有明确 DRI transfer 或拆分后的路径边界；
- Acceptance DRI 静默接管或大幅重构对方 production code；
- 以“谁有空谁继续做”替代明确 ownership；
- 按 Water/Ice 切成互不审查的 production 技术孤岛；
- 为并行而在公共合同冻结前创建两套不兼容接口；
- 把 live issue 状态或临时 branch 写成长久协作合同；
- 用 commit author 改写、force push 或新建多余 PR 掩盖 GitHub 登录账号不一致。

## 11. Modification Policy

本文件属于 LEVEL 3 MAINTAINED collaboration guide，用于解释 `CODING_PLAN.md` 已接受的两人协作方式，
不改变 Level 1/2 产品、架构、参数、实时或测试合同。

- 当前长期角色映射可以在这里维护；具体 work-item assignee/status 仍只写 GitHub；
- 修改 decision rights、review requirement、WIP、path ownership 或 production workflow 时，按 CONTROLLED
  变更同步 `AGENTS.md`、`CODE_STANDARDS.md`、`DOCUMENT_GOVERNANCE.md` 和 `CODING_PLAN.md`；
- 更新示例、交接格式或解释性文字可作为普通 docs PR；
- 本文件不得降低 Production Definition of Done 或绕过 LOCKED contract。
