# FRAZIL Documentation Governance

## 1. 目的

本规则防止计划、决策、实现和验证证据相互冒充。每份文档必须有明确的事实层级、owner 和修改方式；规划中的模块、历史结果和本地工具不得写成当前实现。

## 2. 文档等级

### LEVEL 1 — LOCKED CONTRACT

产品语义、Accepted ADR Decision、`PARAM-FREEZE-001` 之后的参数 ID/range/default/choice/state 兼容性属于 LOCKED。变更需要明确 owner 请求、issue、ADR、兼容性/迁移策略和测试；不得直接改文字来迁就实现。

### LEVEL 2 — CONTROLLED

架构接口、实时/状态/测试合同、代码质量规范、routing transition、公共模块接口和 Accepted performance budget 属于 CONTROLLED。修改需要 issue、review、受影响测试和文档；跨边界或合同变化时补 ADR。

### LEVEL 3 — MAINTAINED / EDITABLE

模块 README、`MODULE_INDEX.md`、实现概览和解释性文档属于可维护内容。它们可以随实现更新，但不能改变 Level 1/2 的事实；发现冲突时回到合同文档和 issue 处理。

### LEVEL 4 — STATUS / EVIDENCE

`PROJECT_STATUS.md`、benchmark、listening、test、CI 和 compatibility evidence 只记录真实结果。每条结果应能追溯到命令、环境、时间、commit、artifact 或 reviewer；planned/未验证必须显式标注。

## 3. 文档地图与责任

| 文档 | 主要等级 | 修改边界 |
|---|---|---|
| `FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` | Level 1/2/3 | 产品语义和 Accepted 决策受保护；候选方案需 ADR；解释性文字可维护 |
| `CODING_PLAN.md` | Level 2 | 只维护工作项定义、依赖、交付物、验收和 exit gate；不写实时 issue 状态 |
| `PARAMETERS.md` | Level 1/2/3 | registry/语义按 freeze 保护；解释性文字可维护；ID/schema 变化必须迁移与测试 |
| `TESTING.md` | Level 2/4 | 测试合同和 gate 受控；实际结果进入 status/evidence |
| `GITHUB_WORKFLOW.md` | Level 2 | 只维护稳定流程；不写当前远端 metadata 或 Actions 状态 |
| `PROJECT_STATUS.md` | Level 4 | 只写已核验的当前事实、证据和明确的未验证项 |
| `docs/adr/*` | Level 1/2 | Accepted Decision 不原地改写；新决策通过 supersedes 链接演进 |
| `CODE_STANDARDS.md` / 本文件 | Level 2 | 质量、治理和修改流程变更需同步 Agent/计划/模块文档 |
| `MODULE_INDEX.md` / `src/*/README.md` | Level 3 | 随实现维护；公共接口事实必须回链 Level 2 合同 |

## 4. 修改流程

1. 先确定文档等级、owner、受影响的 issue/milestone 和是否改变合同；
2. 若为 Level 1/2，先更新或新增 ADR，再修改实现/文档；
3. 同步测试合同、模块 README、`MODULE_INDEX`、AGENTS 和计划中的受影响字段；
4. 做链接/路径/状态一致性检查，并明确实际验证与未执行验证；
5. PR 必须完成 Code Quality Review 和 Comment & Documentation Pass；Accepted ADR 不删除历史。

文档出现冲突时，停止扩大实现范围，列出冲突事实和建议 owner，由 issue/ADR 决定，不擅自选择一方作为真相。

## Modification Policy

本文件属于 CONTROLLED 治理合同。修改必须有治理 issue/review；等级、文档地图、owner 或修改流程发生变化时，必须同步 `AGENTS.md`、`CODE_STANDARDS.md` 和 `CODING_PLAN.md`。
