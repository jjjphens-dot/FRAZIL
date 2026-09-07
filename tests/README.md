# tests/

当前测试 target：

- `frazil_smoke`：CTest wiring smoke；
- `frazil_tests`：ProcessSpec、ParameterLayout 精确类型/名称/单位/choice 合同、
  ParameterSnapshot、ParameterMapper、DryWetMixer、LinearSmoother repeated-target/retarget
  block regression、RandomSource、AudioEngine gain staging、首 block priming、reset、zero-length
  和 runtime buffer invariant unit cases；M1-C StateModel/Host State Adapter 的 versioned
  round-trip、legacy ID migration、duplicate/nonnumeric/malformed invalid parser fallback、routing 和 inactive retention cases。

后续按 `docs/TESTING.md` 增加 state、DSP property、离线渲染回归和 Host/Plugin 集成测试；
测试 target 不依赖插件 editor。
