# ADR-0005: v1 零采样 Host Processing Latency

- Status: Accepted
- Date: 2026-09-09

## Modification Policy

本 ADR 的 Accepted Decision 是 v1 latency contract。若未来算法需要 Host 可见的非零
processing latency，必须新增 superseding ADR，并补 Host PDC、兼容性、性能和迁移影响分析；
声音设计用的 intentional delay/tail 不得通过修改本 ADR 的文字来掩盖。

## Context

FRAZIL v1 需要让 Host 能稳定报告处理延迟，同时允许 Water/Ice 在未来使用属于声音设计的
micro-delay、resonance tail、decay 或 echo-like response。整体 processing latency 与这种声音
结果的时间行为不是同一个合同。当前 M1 的 AudioEngine 是 post-input pass-through skeleton，
dry reference、wet path 和 global mix infrastructure 不应偷偷引入 sample shift。

## Decision

- v1 Plugin Host-reported processing latency 固定为 **0 samples**；当前插件不得依赖 lookahead、
  FFT block latency、linear-phase processing、convolution latency 或 Host PDC 才能正确工作；
- AudioEngine 的 prepare/reset/process infrastructure 必须保持 neutral/dry reference 与当前
  dry path time-aligned；未来 routing/mixing scratch buffer 或 topology 也不得引入未声明的
  整体偏移；
- Water/Ice 的 intentional effect delay/tail 属于各自算法 ADR 和测试的声音语义，不自动转换为
  plugin latency metadata；
- canonical `TESTDATA-001` impulse 在 `input.gain=0 dB`、`output.gain=0 dB`、`global.mix=0`
  的 neutral/dry fixture 下用于验证当前 infrastructure 的 sample alignment，插件 metadata
  用 `getLatencySamples()` 单独验证 Host reporting；当前 M1 pass-through 的 zero-tail 只是一项
  skeleton regression，不是未来 Water/Ice 的永久 tail contract。

## Consequences

Water/Ice 若采用真正需要 lookahead、FFT、linear-phase 或 convolution 的结构，必须先暂停实现并
更新 latency contract。Routing 的 branch copy、StageMixer、Global Mix 和 transition 设计必须
保留这一 contract；算法 tail 需要单独定义 reporting、衰减、reset 和测试窗口。未来算法若需要
合法的非零 tail，superseding ADR/algorithm test 可以替换当前 M1 skeleton-tail regression，但
不得改变 v1 processing-latency invariant 而不经过新的兼容性决策。

## Verification

`frazil_latency_contract` 从 `testdata/input/zero_state_response__impulse.wav` 加载 canonical
impulse，先在 prepare 前设置 `input.gain=0 dB`、`output.gain=0 dB`、`global.mix=0`，再按
128-sample blocks 通过实际 `FRAZILAudioProcessor`。它断言 `getLatencySamples() == 0`，并在
neutral/dry path 逐样本比较 input/reference 与 output、比较 peak sample；单独的 failure message
标记当前 M1 skeleton 的 zero-tail regression。CTest/evidence 记录实际 commit、最大 dry error
和 peak sample；该测试不替代未来 Water/Ice intentional-tail tests 或真实 Host 验证。
