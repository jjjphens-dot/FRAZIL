# tests/

当前测试 target：

- `frazil_smoke`：CTest wiring smoke；
- `frazil_tests`：M0 AudioEngine lifecycle/pass-through unit cases。

后续按 `docs/TESTING.md` 增加 ParameterLayout、DSP property、离线渲染回归和
Host/Plugin 集成测试；测试 target 不依赖插件 GUI。
