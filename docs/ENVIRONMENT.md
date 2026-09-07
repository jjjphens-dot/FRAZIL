# FRAZIL 环境策略

## IDE 与编译器的区分

VS Code 是 FRAZIL 的主要 IDE。Visual Studio IDE 不是项目必需品。

Windows 原生构建仍需要一个 C++ 编译器和 Windows SDK；本项目不新增或升级 Visual Studio。仓库内 preset 只使用 `cl`、`rc`、`mt` 等环境工具名，不写入任何开发者机器的绝对工具链路径。

## 磁盘布局

当前工作区（以下用 `<workspace-root>` 表示）：

```text
<workspace-root>
```

预期布局：

```text
<workspace-root>\build       # CMake 构建输出
<workspace-root>\.venv       # 项目专用 Python 环境
<workspace-root>\external    # JUCE 等项目依赖
<workspace-root>\tools       # Ninja、pluginval 等外部工具本地副本
```

项目依赖、构建输出和 Python 包留在工作区；系统级 Visual Studio、Windows SDK、CMake、LLVM 和 Python 保持在各自的安装位置。

本机工具链路径只允许写入被 `.gitignore` 忽略的 `CMakeUserPresets.json`。当前验证机使用
`local-windows-debug`、`local-windows-release` 和 `local-windows-asan`；协作者不应复制其中的绝对路径。

## 当前审计结论

- VS Code、Git、CMake、LLVM、Python、MSVC 和 Windows SDK 已存在。
- Ninja 1.13.2 已放置在 `<workspace-root>\tools\bin`，不依赖系统 PATH。
- Python 3.12.4 已创建项目 `.venv`，并安装 NumPy、SciPy、soundfile、matplotlib。
- JUCE 9.0.1 已放置在 `external/JUCE`；pluginval 1.0.4 已放置在 `tools/bin`。
- `tools/bootstrap_dependencies.ps1` 已固定 JUCE 9.0.1 commit，并负责恢复仓库内兼容补丁。
- Debug / Release / ASAN 配置、构建和 CTest（各 3/3，含 plugin integration）均已验证；本分支 Debug VST3 使用 pluginval 1.0.4、strictness 5、seed 12345 验证成功；Steinberg VST3 Validator 尚未接入。
- Steinberg VST3 Validator 和 Catch2 尚未接入，它们不是当前 VS Code 日常编译调试的阻塞项。

## VS Code 调试入口

- `Ctrl+Shift+B`：调用 `FRAZIL: build windows-debug`。
- `Run and Debug` 中选择 `FRAZIL Standalone (Debug)` 后按 F5：构建并启动 Standalone。
- VS Code 使用现有 MSVC/Windows SDK 工具链；请从已初始化的 MSVC developer environment 启动 VS Code，Visual Studio IDE 不参与项目工作流。

## Portable CI preset

GitHub Actions 和其他已初始化 MSVC developer environment 的 Windows 机器使用：

```powershell
.\tools\bootstrap_dependencies.ps1
cmake --preset ci-windows-debug
cmake --build --preset ci-windows-debug
ctest --preset ci-windows-debug
```

`ci-windows-debug` 从 `VCToolsInstallDir` 解析 MSVC，使用 `rc`/`mt` 的环境发现，
不引用本机绝对路径；`windows-debug`/`windows-release`/`windows-asan` 也只依赖已初始化的
MSVC developer environment。当前机器的绝对路径仅存在于本地 ignored user preset 中。
