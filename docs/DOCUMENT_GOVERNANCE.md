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

这是仓库级 Documentation Impact Analysis、同步和一致性审查的 canonical 规则。凡任务改变 architecture、public interface、module existence/responsibility、ownership、thread/realtime behavior、parameter/state/routing contract、random/latency semantics、test evidence、Host/UI behavior、milestone、build/CI 或 performance，必须在实现前执行以下流程：

```text
Read contract
  -> Identify changed contracts and evidence
  -> Documentation Impact Analysis
  -> Implementation
  -> Documentation Synchronization in the same PR
  -> Documentation Consistency Review
  -> Tests / CI
  -> PR review and final report
```

### 5.1 Documentation Impact Matrix

| 变化领域 | 必须检查的文档 | 同步触发 |
|---|---|---|
| Architecture / dependency / ownership | `FRAZIL_PROJECT_ARCHITECTURE_v0.3.md`、`MODULE_INDEX.md`、受影响 module README、相关 ADR | 边界、职责、所有权或依赖方向改变时同 PR 更新 |
| Parameter / Host contract | `PARAMETERS.md`、`MODULE_INDEX.md`、plugin/app README、相关 tests、`PROJECT_STATUS.md` | ID、range、default、choice、smoothing、Host behavior 或 evidence 改变时同 PR 更新 |
| State | `PARAMETERS.md`、ADR-0002、`MODULE_INDEX.md`、app/plugin README、`TESTING.md`、`PROJECT_STATUS.md` | schema、migration、fallback、inactive value retention 或 restore behavior 改变时同 PR 更新 |
| Realtime / DSP processing | 受影响 module README、`CODE_STANDARDS.md`、`TESTING.md`、相关 ADR、`MODULE_INDEX.md` | realtime boundary、buffer、random、latency、DSP contract 或 evidence 改变时同 PR 更新 |
| Routing | Architecture、ADR-0001、`PARAMETERS.md`、`MODULE_INDEX.md`、routing README、`TESTING.md` | topology、mix law、transition 或 routing evidence 改变时同 PR 更新 |
| UI / Undo / interaction | `PARAMETERS.md`、UI/app README、`MODULE_INDEX.md`、`TESTING.md`、相关 UX 文档 | user interaction、gesture/history 或 UI contract 改变时同 PR 更新 |
| Build / dependency / CI | `ENVIRONMENT.md`、README、`TESTING.md`、`GITHUB_WORKFLOW.md`、`PROJECT_STATUS.md` | toolchain、preset、dependency、workflow 或 CI evidence 改变时同 PR 更新 |
| Milestone / implementation status | `PROJECT_STATUS.md`、`MODULE_INDEX.md` | Planned/Partial/Implemented、milestone 或 exit-gate status 改变时同 PR 更新 |

“Reviewed, no update required” 是有效结果，但必须在 PR checklist 或最终报告中记录理由。不得仅因为改动看起来是局部代码或文档而跳过矩阵审查；`CODING_PLAN.md` 的稳定 milestone 合同只有在计划真正改变时才修改。

### 5.2 Documentation Consistency Review

实现和文档同步后，必须交叉检查 `PROJECT_STATUS.md`、`MODULE_INDEX.md`、受影响 module README、`CODING_PLAN.md`、`PARAMETERS.md`、`TESTING.md`、`COLLABORATION_ROLES.md`、Architecture 和相关 ADR。重点排查 Planned/Implemented、StateModel existence、parameter freeze、main/feature、CI result/SHA 和 module ownership 的冲突。

如果 source、contract、module index、status 或 evidence 互相矛盾，Documentation Gate = FAIL，PR 不得标记 Ready，直到事实层级被明确并完成同步。`PROJECT_STATUS.md` 只记录已验证事实，不承载长期计划正文；长期协作指南只保留职责、合同、边界、流程和阶段重心，不记录 current HEAD、reviewer、临时 branch 或单次 CI 状态。

### 5.3 Human Review Evidence

Parameter contract、State contract、Routing、production Water/Ice DSP、latency/tail/random semantics、
realtime architecture、performance budget 和 release PR 必须留下可验证的 reviewer evidence，优先使用
GitHub formal `APPROVE`、`COMMENT` 或 `REQUEST_CHANGES` submission。需要另一位开发者 review 的工作仍需
真实的第二人 evidence；PR creator 自审不能冒充独立 review。

Implementation DRI、Acceptance DRI、PR creator、push account、commit author/committer 和 reviewer 必须
分别、真实记录，但不要求相同或预先映射到固定账号。`PR creator != Implementation DRI`、commit authorship
混合或 reviewer 曾参与其它相关 commit，本身不触发 PR recreation。reviewer evidence 明确：

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

创建 PR、首次向 branch push 或向已有 PR 分支继续 push 前，preflight 至少核对
`git branch --show-current`、`gh pr list --head <branch> --state open --json number,author,url` 和
`gh api user --jq .login`。如果 branch 没有 open PR，当前准备管理该 PR lifecycle 的账号可以创建 PR，
无需等于 Implementation DRI。若已有 PR，唯一严格的账号一致性规则是：执行后续 push 的 authenticated
account 必须等于 existing PR creator。不同则停止 push 并优先检查错误登录；不得跨账号 push、改写
commit author、force push、rewrite history 或新建不必要的 PR 规避。文档 PR 与 code PR 使用同一规则。

### 5.4 Final Report

Agent 完成任务时，最终反馈必须包含：

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

不得只写“docs updated”；未执行的检查必须明确标为 `NOT RUN`。

## Modification Policy

本文件属于 CONTROLLED 治理合同。修改必须有治理 issue/review；等级、文档地图、owner 或修改流程发生变化时，必须同步 `AGENTS.md`、`CODE_STANDARDS.md` 和 `CODING_PLAN.md`。
