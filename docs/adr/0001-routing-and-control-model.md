# ADR-0001: Routing 与控制模型

- Status: Accepted
- Date: 2026-09-06

## Context

FRAZIL 需要并联与两个串联方向。Parallel 的“两个分支相对比例”和 Serial 的“每个 stage 处理量”是不同产品语义；时间上的 Water -> Ice 变化又属于 DAW timeline。

## Decision

- Parallel 使用唯一参数 `parallel.balance`，0 为 Water、1 为 Ice；
- Serial 使用 `water.amount` 和 `ice.amount` 分别控制对应 stage dry/wet；
- Water -> Ice 与 Ice -> Water 只改变 stage 顺序，不改变 amount 的语义；
- inactive-mode 参数保持注册、值和 state，但不参与当前 DSP；
- v1 不实现内部 morph timeline，动态过渡由 Host automation 组合两个 amount；
- WaterProcessor/IceProcessor 不感知 routing，组合由 RoutingEngine/StageMixer 负责。

## Consequences

Host 始终枚举三类独立参数；UI 按 mode 显隐但不重置；routing 测试必须覆盖端点、stage 组合、enable 组合、值保留和 click-free mode switch。

## Verification

`docs/TESTING.md` 的 routing render matrix、mode retention 和 DAW automation acceptance 全部通过。
