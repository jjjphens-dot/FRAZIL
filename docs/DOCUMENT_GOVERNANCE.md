# FRAZIL Documentation Governance

## 1. 目的

本规则防止计划、决策、实现和验证证据相互冒充。每份文档必须有明确的事实层级、owner 和修改方式；规划中的模块、历史结果和本地工具不得写成当前实现。

## 2. 文档等级

### LEVEL 1 — LOCKED CONTRACT

产品语义、Accepted ADR Decision、`PARAM-FREEZE-001` 之后的参数 ID/range/default/choice/state 兼容性属于 LOCKED。变更需要明确 owner approval、issue、new/superseding ADR where applicable、兼容性/迁移分析、测试和 review；不得直接改文字来迁就实现。

### LEVEL 2 — CONTROLLED

架构接口、实时/状态/测试合同、代码质量规范、routing transition、公共模块接口和 Accepted performance budget 属于 CONTROLLED。普通修改需要 issue、review、受影响测试和文档；只有改变既有架构决策、依赖边界、公共职责、参数/状态兼容性、routing、realtime、latency、random-state 或正式性能合同时才补 ADR。

### LEVEL 3 — MAINTAINED / EDITABLE

模块 README、`MODULE_INDEX.md`、实现概览和解释性文档属于可维护内容。它们可以随实现更新，但不能改变 Level 1/2 的事实；发现冲突时回到合同文档和 issue 处理。

### LEVEL 4 — STATUS / EVIDENCE

`PROJECT_STATUS.md`、benchmark、listening、test、CI 和 compatibility evidence 只记录真实结果。每条结果应能追溯到命令、环境、时间、commit、artifact 或 reviewer；planned/未验证必须显式标注。

## 3. 文档地图与责任

| 文档 | 主要等级 | 修改边界 |
|---|---|---|
| `FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` | Level 1/2/3 | 产品语义和 Accepted 决策受保护；candidate/experiment 可先自由验证，采纳为 production decision 或改变合同时按 ADR trigger；解释性文字可维护 |
| `CORE_IMPLEMENTATION_GUIDE.md` | Level 3 | 解释已接受合同、候选 implementation methods、数学和测试参考；不得冻结新参数合同、改写 Accepted ADR、伪造 production/status/evidence 或承诺未知 state preservation |
| `CODING_PLAN.md` | Level 2 | 只维护工作项定义、依赖、交付物、验收和 exit gate；不写实时 issue 状态 |
| `PARAMETERS.md` | Level 1/2/3 | registry/语义按 freeze 保护；解释性文字可维护；ID/schema 变化必须迁移与测试 |
| `TESTING.md` | Level 2/4 | 测试合同和 gate 受控；实际结果进入 status/evidence |
| `GITHUB_WORKFLOW.md` | Level 2 | 只维护稳定流程；不写当前远端 metadata 或 Actions 状态 |
| `COLLABORATION_ROLES.md` | Level 3 | 维护角色分工、交接和协作解释；live assignee/status 留在 GitHub；decision rights/workflow 变化触发 Level 2 同步 |
| `PROJECT_STATUS.md` | Level 4 | 只写已核验的当前事实、证据和明确的未验证项 |
| `docs/adr/*` | Level 1/2 | Accepted Decision 不原地改写；新决策通过 supersedes 链接演进 |
| `CODE_STANDARDS.md` / 本文件 | Level 2 | 质量、治理和修改流程变更需同步 Agent/计划/模块文档 |
| `MODULE_INDEX.md` / `src/*/README.md` | Level 3 | 随实现维护；公共接口事实必须回链 Level 2 合同 |

## 4. 修改流程

1. 先确定文档等级、owner、受影响的 issue/milestone 和是否改变合同；
2. 实现已接受合同时，使用 issue + code/docs + tests + review；只有触发 ADR 条件时才先更新或新增 ADR；
3. 同步测试合同、模块 README、`MODULE_INDEX`、AGENTS 和计划中的受影响字段；
4. 做链接/路径/状态一致性检查，并明确实际验证与未执行验证；
5. PR 必须完成 Code Quality Review 和 Comment & Documentation Pass；Accepted ADR 不删除历史。

### ADR trigger

实现既有 accepted contract、添加已有测试合同要求的测试、修复 bug，或不改变公共行为和 dependency boundary 的 refactor，只需要 issue、代码/文档、测试和 review。只有改变 architecture decision、dependency boundary、public module responsibility、parameter semantics/ID/range/choice compatibility、state schema/compatibility strategy、routing semantics、realtime boundary、latency contract、random-state persistence semantics、formal performance contract/budget 或 major DSP algorithm decision 时才需要 ADR。

文档出现冲突时，停止扩大实现范围，列出冲突事实和建议 owner，由 issue/ADR 决定，不擅自选择一方作为真相。

## 5. Documentation Synchronization Gate

这是仓库级 Documentation Impact Analysis、同步和一致性审查的 canonical 规则。Documentation review 与
change risk/scope 成比例；Targeted Check 是普通 bounded task 的默认路径，只有明确命中高影响 trigger 才进入
Full Documentation Synchronization Gate。

#### Level A — Targeted Documentation Impact Check

bug fix、unit regression、small implementation correction、bounded refactor、narrow tooling 和 ordinary docs
correction 只执行：

```text
Identify directly affected contract/module/evidence
  -> Check directly relevant docs
  -> Update if needed in the same PR
  -> If no documented contract/status changes: stop
```

add regression coverage、fix unit test、refactor test helper、add one property case 或修复 deterministic fixture，
如果不改变 documented support/status、milestone exit evidence、public compatibility claim、canonical testing
contract，或下列任一产品/工程合同，不触发 Full Gate。

#### Level B — Full Documentation Synchronization Gate

只有改变 architecture、public interface、module existence/responsibility、ownership model、parameter/state/routing
contract、thread/realtime boundary、latency/random semantics、documented Host/UI behavior、build/dependency/CI
contract、formal performance contract、milestone/status/support claim 或 release compatibility claim 时执行 Full
Gate。test evidence 只有在改变 documented project status、support claim、milestone exit evidence、public
compatibility claim 或 canonical testing contract 时才属于 Full Gate trigger。

```text
Read affected canonical contract
  -> Identify changed contracts and documented evidence/status
  -> Use relevant Documentation Impact Matrix rows
  -> Implementation + documentation synchronization in the same PR
  -> Consistency review of affected documents
  -> Proportional tests / CI
  -> Risk-appropriate review and final report
```

### 5.1 Documentation Impact Matrix

Full Gate 只使用实际变化命中的矩阵行；Targeted Check 只检查直接相关文档，不机械遍历整张矩阵。

| 变化领域 | 相关文档 | 同步触发 |
|---|---|---|
| Architecture / dependency / ownership | `FRAZIL_PROJECT_ARCHITECTURE_v0.3.md`、`MODULE_INDEX.md`、受影响 module README、相关 ADR | 边界、职责、所有权或依赖方向改变时同 PR 更新 |
| Parameter / Host contract | `PARAMETERS.md`、`MODULE_INDEX.md`、plugin/app README、相关 tests、`PROJECT_STATUS.md` | ID、range、default、choice、smoothing、documented Host behavior/support 或 compatibility claim 改变时同 PR 更新 |
| State | `PARAMETERS.md`、ADR-0002、`MODULE_INDEX.md`、app/plugin README、`TESTING.md`、`PROJECT_STATUS.md` | schema、migration、fallback、inactive value retention 或 restore behavior 改变时同 PR 更新 |
| Realtime / DSP processing | 受影响 module README、`CODE_STANDARDS.md`、`TESTING.md`、相关 ADR、`MODULE_INDEX.md` | realtime boundary、buffer、random、latency、DSP contract 或 documented support/performance claim 改变时同 PR 更新 |
| Routing | Architecture、ADR-0001、`PARAMETERS.md`、`MODULE_INDEX.md`、routing README、`TESTING.md` | topology、mix law、transition、routing contract 或 documented compatibility/milestone evidence 改变时同 PR 更新 |
| UI / Undo / interaction | `PARAMETERS.md`、UI/app README、`MODULE_INDEX.md`、`TESTING.md`、相关 UX 文档 | user interaction、gesture/history 或 UI contract 改变时同 PR 更新 |
| Build / dependency / CI | `ENVIRONMENT.md`、README、`TESTING.md`、`GITHUB_WORKFLOW.md`、`PROJECT_STATUS.md` | toolchain、preset、dependency、workflow contract 或 documented CI support/status 改变时同 PR 更新 |
| Milestone / implementation/support status | `PROJECT_STATUS.md`、`MODULE_INDEX.md` | Planned/Partial/Implemented、support claim、milestone 或 exit-gate status 改变时同 PR 更新 |

“Reviewed, no update required” 是有效结果，但只需对 directly relevant documents 记录理由。普通 task 不需要
为完全无关文档逐项填写 `N/A`；`CODING_PLAN.md` 的稳定 milestone 合同只有在计划真正改变时才修改。

### 5.2 Documentation Consistency Review

Targeted Check 只交叉检查直接受影响的 canonical/module/evidence 文档。Full Gate 根据 Documentation Impact
Matrix 中实际命中的行检查 affected `PROJECT_STATUS.md`、`MODULE_INDEX.md`、module README、`CODING_PLAN.md`、
`PARAMETERS.md`、`TESTING.md`、`COLLABORATION_ROLES.md`、Architecture 或 ADR；不要求每个任务机械打开全部
文档。重点排查受影响范围内的 Planned/Implemented、module existence、parameter freeze、main/feature、
documented CI/support result 和 module ownership 冲突。

如果 source、contract、module index、status 或 evidence 互相矛盾，Documentation Gate = FAIL，PR 不得标记 Ready，直到事实层级被明确并完成同步。`PROJECT_STATUS.md` 只记录已验证事实，不承载长期计划正文；长期协作指南只保留职责、合同、边界、流程和阶段重心，不记录 current HEAD、reviewer、临时 branch 或单次 CI 状态。

### 5.3 Human Review Evidence

Parameter contract、State contract、Routing、production Water/Ice DSP、latency/tail/random semantics、
realtime architecture、performance budget 和 release PR 必须留下可验证的 reviewer evidence，优先使用
GitHub formal `APPROVE`、`COMMENT` 或 `REQUEST_CHANGES` submission。需要另一位开发者 review 的工作仍需
真实的第二人 evidence；PR creator 自审不能冒充独立 review。

Implementation DRI、Acceptance DRI、PR creator 和 reviewer 分别记录，但不要求相同或预先映射到固定
账号。push account 是运行时状态，commit author/committer 已由 Git history 记录，无需在普通 PR body
重复抄写。`PR creator != Implementation DRI`、commit authorship 混合或 reviewer 曾参与其它相关 commit，
本身不触发 PR recreation。核心/高风险 reviewer evidence 明确：

```text
Reviewer
Review scope
Evidence reproduced
Evidence not reproduced
Findings
Decision
Formal GitHub review type or NOT AVAILABLE / NOT RECORDED
Fallback comment/manual evidence or N/A
```

平台权限不允许 formal review 时，第二位开发者可以留下明确标注的 comment/manual evidence；不得把普通
comment 写成 formal `APPROVE`。既有 PR 的 evidence 缺口记录为 historical process gap，不通过回滚、伪造
author 或 history rewrite 补造。

普通 docs、bounded test、typo、narrow tooling 或 low-risk maintenance 的 review 记录可缩减为 Reviewer、
Decision 和 notable limitations/findings；documentation/review evidence 应与风险和 scope 成比例。

仅在准备向 existing PR branch 执行当前 context 的首次 push 时，核对 open PR creator 和 authenticated
account。相同 repository + branch + PR + auth session 的成功检查可以复用；上下文或账号变化、权限异常时
重新检查。authenticated account 必须等于 existing PR creator；不同则停止 push 并检查登录，不得跨账号
push、改写 author、force push、rewrite history 或新建无意义 PR。没有 open PR 时按普通 push/PR 创建流程
处理；文档 PR 与 code PR 使用同一规则。

### 5.4 Final Report

Agent 完成任务时按风险比例报告。所有任务至少说明 changed files、实际 validation、未执行检查和结果；
命中 Level B Full Gate trigger 时，使用完整 Documentation Review：

```text
## Documentation Review

Changed:
- ...

Reviewed, no update required:
- ...

Consistency:
- PROJECT_STATUS <-> GitHub/main
- MODULE_INDEX <-> source
- module README <-> implementation
- PARAMETERS <-> parameter/state contract
- TESTING <-> evidence
- CODING_PLAN <-> milestone scope

Result:
PASS / FAIL
```

Targeted Check 下的 bounded docs/maintenance task 可以合并为简短的 changed/reviewed/consistency/result
结论，只列 directly relevant documents。不得只写“docs updated”；未执行的相关检查必须明确标为
`NOT RUN`。

## Modification Policy

本文件属于 CONTROLLED 治理合同。修改必须有治理 issue/review；等级、文档地图、owner 或修改流程发生变化时，必须同步 `AGENTS.md`、`CODE_STANDARDS.md` 和 `CODING_PLAN.md`。
