# ADR-0003: 实时处理与层级边界

- Status: Accepted
- Date: 2026-09-06

## Context

实时音频 callback 的延迟上界和可预测性优先于一般应用代码便利性。FRAZIL 同时需要 Host/UI 状态、编辑历史、随机声音事件与可重复测试。

## Decision

- 主依赖方向为 `plugin -> app -> dsp`；UI 只通过参数/command 接口交互；
- process path 禁止 I/O、logging、不可控分配、阻塞锁、UI/history、线程等待和异常控制流；
- buffer 与算法状态在 `prepare()` 预分配，`reset()` 可重复；
- 参数通过 block Snapshot 输入，连续量 smoothing，离散量 click-free transition；
- 随机 DSP 使用可注入 seed 的局部 RandomSource；
- production DSP 需 finite-output/property test 与 callback 性能测量。

## Consequences

Undo/Redo、state migration、文件解析、render manifest 和日志均在非音频线程。算法若需要运行时扩容或阻塞资源，必须重设设计而不是在 callback 中补锁。

## Verification

ASAN/property tests、allocation instrumentation、automation stress 与 Reference Machine callback benchmark 通过。
