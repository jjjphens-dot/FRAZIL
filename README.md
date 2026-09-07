# FRAZIL

FRAZIL 是一个以 Water / Ice 声音材质化为核心的实时音频效果器，首要格式为 VST3，并保留 Standalone 作为开发与测试宿主。

当前处于 M1 早期：Windows + JUCE 9.0.1 构建链路、静态参数合同、Snapshot/Mapper 和基础 gain/mix skeleton 已接入，wet path 仍为 pass-through，Water / Ice / Routing 生产 DSP 尚未实现。详细事实基线见 [`docs/PROJECT_STATUS.md`](docs/PROJECT_STATUS.md)，实施顺序见 [`docs/CODING_PLAN.md`](docs/CODING_PLAN.md)。

## 产品合同摘要

- Parallel：`parallel.balance` 控制 Water 与 Ice 两条并联支路的相对比例。
- Serial：`water.amount` 与 `ice.amount` 分别控制两个 stage mix，支持 Water -> Ice 与 Ice -> Water。
- 时间上的 Water -> Ice 变化由 DAW automation 完成，v1 不实现内部 morph timeline。
- `input.gain` 位于完整处理链之前，`global.mix` 混合 dry/wet，`output.gain` 位于最终输出。
- 所有正式参数静态注册、ID 稳定、可自动化、可保存恢复；UI 显隐不改变 Host 参数集合。
- Undo/Redo 只管理插件 UI 编辑历史，不记录 Host automation 或工程恢复。

## 文档入口

- [架构总纲](docs/FRAZIL_PROJECT_ARCHITECTURE_v0.3.md)
- [高密度 Coding Plan](docs/CODING_PLAN.md)
- [当前实现与差距](docs/PROJECT_STATUS.md)
- [产品命名与身份](docs/PRODUCT_IDENTITY.md)
- [参数与状态合同](docs/PARAMETERS.md)
- [测试与发布门槛](docs/TESTING.md)
- [GitHub 协作流程](docs/GITHUB_WORKFLOW.md)
- [双人协作分工](docs/COLLABORATION_ROLES.md)
- [开发环境](docs/ENVIRONMENT.md)
- [HOST-000 平台与 DAW 兼容性矩阵（产品目标已冻结，HOST-001 evidence pending）](docs/HOST-000_COMPATIBILITY_MATRIX.md)
- [贡献指南](CONTRIBUTING.md)

## 本地构建

首次配置或全新 checkout 先恢复固定依赖：

```powershell
.\tools\bootstrap_dependencies.ps1
python tools/check_portability.py
```

在已初始化 MSVC developer environment 的 Windows PowerShell 中构建：

本地构建入口使用 tools/build_safe.py，默认 6 个 job、硬上限 8 个 job。 共享 configure preset 同时注入 CMAKE_BUILD_PARALLEL_LEVEL=6，约束 JUCE configure 阶段的 nested build；本机 Debug、Release、ASAN 等重型 pipeline 必须串行执行。

```powershell
cmake --preset windows-debug
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug
```

`windows-release` 与 `windows-asan` 使用同名 configure/build/test preset。CI 或其他 Windows
机器在 MSVC developer environment 已初始化后使用 `ci-windows-debug`，所有共享 preset 都使用 portable tool discovery。
Windows 工具链初始化、pluginval 和本地配置见 [`docs/ENVIRONMENT.md`](docs/ENVIRONMENT.md)。
所有构建输出、Python 环境和工具缓存必须保持 repository-local 或由 ignored local configuration 指定，并且不得提交。

## 仓库

GitHub：<https://github.com/jjjphens-dot/FRAZIL>

许可证：MIT，详见 [LICENSE](LICENSE)。

初版已按 `docs/CODING_PLAN.md` 的 M0 仓库接入清单推送到 `main`。后续提交仍需确认不包含 `build/`、`.venv/`、`tools/bin/`、`tools/downloads/` 或生成音频。
