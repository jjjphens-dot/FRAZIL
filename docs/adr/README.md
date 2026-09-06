# Architecture Decision Records

ADR 记录会影响多个模块或长期兼容性的决策。状态使用 `Proposed`、`Accepted`、`Superseded`、`Rejected`。

新增 ADR 使用四位序号，至少包含：Context、Decision、Consequences、Verification。已 Accepted 的 ADR 不原地改写历史结论；若决策改变，新增 ADR 并标记 supersedes/superseded by。

| ADR | 状态 | 主题 |
|---|---|---|
| [0001](0001-routing-and-control-model.md) | Accepted | Parallel/Serial 控制模型与 DAW timeline 边界 |
| [0002](0002-parameter-and-state-contract.md) | Accepted | 静态 Host 参数、Snapshot 和 state 兼容性 |
| [0003](0003-realtime-processing-boundary.md) | Accepted | 实时线程与层级依赖边界 |
| [0004](0004-juce-and-ci-dependency-strategy.md) | Proposed | JUCE 固定、补丁和可移植 CI 获取方式 |
