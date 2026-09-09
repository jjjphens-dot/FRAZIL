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
  gain automation smoothing、prepareToPlay -> setStateInformation -> processBlock 生命周期 restore，以及全部 routing mode 切换后的 inactive amount/state reopen
  integration cases；
- `frazil_processor_property`：TEST-002 的数据驱动 processor property harness；完整覆盖
  44.1/48/96 kHz、32/64/128/256/512/1024 block、mono/stereo 的 runtime matrix，并在
  48 kHz/128/stereo canonical 子矩阵覆盖参数极值、enable/routing combinations、silence、
  impulse、deterministic noise、extreme finite input、prepare/reset/repeated prepare/repeated
  reset/zero-length lifecycle 和 fresh-processor deterministic output；
- `frazil_latency_contract`：ARCH-LAT-001 的 canonical TESTDATA-001 impulse alignment 与
  `getLatencySamples() == 0` / zero-tail metadata regression；
- `frazil_performance_baseline`：PERF-BASE-001 的 headless 48 kHz/128/stereo processor
  workload，记录 mean/P95/P99/worst、callback deadline、working set 和 measured-callback
  `operator new` observation；报告写入 ignored preset build tree；
- `frazil_render` + `tools/render_testdata.py`：RENDER-001 的离线 WAV smoke，固定 input/config/seed
  通过当前 AudioEngine 处理，检查 finite output、重复运行字节一致性，并生成完整当前配置、
  input/output metadata 与 SHA-256 manifest；CTest 产物写入 preset build tree 下的 ignored
  `rendered/`，手工运行默认写入 ignored `testdata/rendered/`；
- `tools/test_render_cli.py`：回归 `frazil_render` 的 `--help` 成功路径、enable/routing/balance/
  amount 非法值拒绝、一组非默认完整配置在 manifest 中的逐字段保留，以及同一 output path
  重复渲染时覆盖而非追加 WAV；
- `tools/verify_testdata.py` 与 `tools/test_testdata.py`：`TESTDATA-001` 的十个 canonical
  DSP diagnostic 输入、schema-v2 manifest、SignalSpec-derived objective/parameters/properties/
  windows/targets、机器可读 provenance、MIT 来源、PCM24 WAV metadata、存储策略、SHA-256
  完整性、完整 generator-to-temporary WAV + manifest 语义/字节级可复现性、44.1/48/96 kHz
  temporary generation、每个信号的 semantic regression 和 input/manifest 双向集合回归；
- `tools/signal_generators.py` 与 `tools/analyze_testdata.py`：不写入 canonical corpus 的
  deterministic algorithm probes，以及可选的 waveform、FFT、Welch PSD、RMS、DC、stereo
  correlation 和 STFT/spectrogram experiment analysis；analyzer 不是 TESTDATA-001 semantic
  exit blocker；

后续按 `docs/TESTING.md` 增加 DSP property、完整 render regression 和真实 Host/DAW acceptance；
测试 target 不依赖运行中的插件 editor。
