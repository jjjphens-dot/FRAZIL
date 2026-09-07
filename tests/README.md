# tests/

当前测试 target：

- `frazil_smoke`：CTest wiring smoke；
- `frazil_tests`：ProcessSpec、ParameterLayout 精确类型/名称/单位/choice 合同、
  ParameterSnapshot、ParameterMapper、DryWetMixer、LinearSmoother repeated-target/retarget
  block regression、RandomSource、AudioEngine gain staging、首 block priming、reset、zero-length
  和 runtime buffer invariant unit cases；M1-C StateModel/Host State Adapter 的 versioned
  round-trip、JUCE `ValueTree::createXml()`/`fromXml()` XML/API restore path、legacy ID
  migration、duplicate/nonnumeric/malformed invalid parser fallback、routing 和 inactive retention cases。
- `frazil_plugin_integration`：实际 `FRAZILAudioProcessor` 的参数写入到音频路径、连续
  gain automation smoothing，以及全部 routing mode 切换后的 inactive amount/state reopen
  integration cases；

后续按 `docs/TESTING.md` 增加 DSP property、离线渲染回归和真实 Host/DAW acceptance；
测试 target 不依赖运行中的插件 editor。
