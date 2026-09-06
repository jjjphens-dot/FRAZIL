# src/dsp/

## Purpose

承载 Water、Ice、Routing、StageMixer、DryWetMixer 和可复用的实时 DSP primitives。当前目录只有 README；规划模块尚未进入生产 target。

## Responsibilities

- 纯值类型/音频 buffer 处理的 Water/Ice transform；
- Routing topology、stage/global mix law、smoothing、deterministic random source 等 DSP 责任；
- 在 `prepare/reset/process` 生命周期内保持 finite output、可重复和 realtime-safe。

## Non-responsibilities

不得访问 APVTS、Host、UI、UndoManager 或 app/plugin object；不得决定产品参数注册、state schema 或把 `RoutingMode` 反向注入 Water/Ice。

## Architecture / Data Flow

```text
app EngineParameters -> RoutingEngine -> StageMixer -> Water/Ice processors
```

依赖方向只能由 app 指向 dsp；DSP 不反向依赖 app/plugin/ui。模块边界和计划路径见 [MODULE_INDEX.md](../../docs/MODULE_INDEX.md)。

## Public Interfaces

Water/Ice/Routing 等接口均为 Planned，不能在当前代码中视为已实现。正式接口必须使用小型值类型、明确的 prepare/reset/process 生命周期和可测试的 transition contract。

## Ownership & Lifetime

DSP 状态由 AudioEngine/对应 DSP 实例拥有；delay、FFT、scratch buffer 和随机源在 prepare 阶段预分配。不得使用 mutable global/static runtime state。

## Threading / Realtime Rules

音频线程禁止 I/O、logging、阻塞锁、运行时分配、异常控制流、UI/history 访问和隐藏初始化。连续参数必须 smoothing，离散 routing/enable 必须 click-free transition；有限输入下输出不得 NaN/Inf。

## Implementation Overview

generic `RandomSource` primitive/contract 由 M1 `DSP-005` 建立；M2 Water、M3 Ice 只使用并补 algorithm-specific random semantics；Routing/Gain M4 和其他 primitives 按 [CODING_PLAN.md](../../docs/CODING_PLAN.md) 实现。实验算法只有完成 `AGENTS.md` 的 production gate 后才能移入此目录。

## State / Tail / Latency

DSP 可以拥有声音设计所需的 intentional effect delay/tail，但不得把它伪装为 Host processing latency。v1 Host-reported latency 为 0 samples；routing infrastructure 的 latency 由 `ROUTE-011` 验证。

## Tests

每个生产 DSP 模块需要 unit/property/render/listening/performance 中适用的证据；具体 gate 见 [TESTING.md](../../docs/TESTING.md)。当前没有 Water/Ice/Routing 生产实现或对应通过记录。

## Related ADRs

[ADR-0001](../../docs/adr/0001-routing-and-control-model.md)、[ADR-0003](../../docs/adr/0003-realtime-processing-boundary.md)，以及未来各算法/transition ADR。

## Files

规划路径包括 `src/dsp/water/`、`src/dsp/ice/`、`src/dsp/routing/` 和 `src/dsp/primitives/`；当前仅有本 README。

## Modification Policy

本 README 属于 LEVEL 3 module documentation。生产 DSP 新增或移动时必须同步代码质量审查、测试 evidence、[MODULE_INDEX.md](../../docs/MODULE_INDEX.md)、相关 ADR 和 Coding Plan。
