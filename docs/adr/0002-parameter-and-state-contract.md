# ADR-0002: Host 参数与状态合同

- Status: Accepted
- Date: 2026-09-06

## Modification Policy

本 ADR 的 Accepted Decision 是 LOCKED 历史合同，不原地改变参数/state 语义。任何变化必须新增 ADR、更新 `PARAMETERS.md`、补兼容性/迁移测试，并保留历史链。

## Context

VST3 Parameter ID、choice index 与 state 一旦出现在用户 session/preset 中就是兼容性接口。DSP 直接读取 APVTS 会造成同一 block 参数不一致并耦合 Host 层。

## Decision

- 参数静态注册，定义集中在 `ParameterLayout.*`；
- 正式 ID 以 `docs/PARAMETERS.md` 为 registry；enabled ID 使用 `water.enabled`/`ice.enabled`；
- 每个 block 开始创建一次值类型 `ParameterSnapshot`，经 `ParameterMapper` 生成 EngineParameters；
- DSP 不直接读取 APVTS；
- state 带 `schemaVersion`，保存全部参数含 inactive values；
- Host restore 不进入插件 Undo history，restore 后清空 history；
- 第一个公开 alpha 后 ID/choice index 冻结，变更需迁移和 regression fixture。

## Consequences

M0 占位 ID `water.enable`/`ice.enable` 必须在公开版本前一次性修正。参数 PR 需要枚举、state round-trip、automation 和 inactive retention 测试。

## Verification

ParameterLayout 精确合同测试、state fixtures、pluginval 参数测试及目标 DAW save/reopen 通过。
