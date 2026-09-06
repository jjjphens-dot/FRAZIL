# FRAZIL 环境策略

## IDE 与编译器的区分

VS Code 是 FRAZIL 的主要 IDE。Visual Studio IDE 不是项目必需品。

Windows 原生构建仍需要一个 C++ 编译器和 Windows SDK。当前机器已经存在 Visual Studio Community 2022 的 MSVC v143、Windows SDK 22621 和 AddressSanitizer 组件；本项目不新增或升级 Visual Studio。

## 磁盘布局

当前工作区：

```text
F:\coding\FRAZIL
```

预期布局：

```text
F:\coding\FRAZIL\build       # CMake 构建输出
F:\coding\FRAZIL\.venv       # 项目专用 Python 环境
F:\coding\FRAZIL\external    # JUCE 等项目依赖
F:\coding\FRAZIL\tools       # Ninja、pluginval 等外部工具本地副本
```

不把项目依赖、构建输出或 Python 包安装到 C:。系统已有的 Visual Studio、CMake、LLVM 和 Python 保持原状。

## 当前审计结论

- VS Code、Git、CMake、LLVM、Python、MSVC 和 Windows SDK 已存在。
- Ninja 1.13.2 已放置在 `F:\coding\FRAZIL\tools\bin`，不依赖系统 PATH。
- Python 3.12.4 已创建项目 `.venv`，并安装 NumPy、SciPy、soundfile、matplotlib。
- JUCE 9.0.1 已放置在 `external/JUCE`；pluginval 1.0.4 已放置在 `tools/bin`。
- `tools/bootstrap_dependencies.ps1` 已固定 JUCE 9.0.1 commit，并负责恢复仓库内兼容补丁。
- Debug / Release / ASAN 配置、构建和 CTest smoke 测试均已验证；Debug VST3 已通过 pluginval 严格度 5；Debug unit target 已验证。
- Steinberg VST3 Validator 和 Catch2 尚未接入，它们不是当前 VS Code 日常编译调试的阻塞项。

## VS Code 调试入口

- `Ctrl+Shift+B`：调用 `FRAZIL: build windows-debug`。
- `Run and Debug` 中选择 `FRAZIL Standalone (Debug)` 后按 F5：构建并启动 Standalone。
- VS Code 使用现有 MSVC/Windows SDK 工具链；Visual Studio IDE 不参与项目工作流。

## Portable CI preset

GitHub Actions 和其他已初始化 MSVC developer environment 的 Windows 机器使用：

```powershell
.\tools\bootstrap_dependencies.ps1
cmake --preset ci-windows-debug
cmake --build --preset ci-windows-debug
ctest --preset ci-windows-debug
```

`ci-windows-debug` 从 `VCToolsInstallDir` 解析 MSVC，使用 `rc`/`mt` 的环境发现，
不引用本机 F: 盘路径；本机 `windows-debug`/`windows-release`/`windows-asan` preset
仍保留固定工具链路径，便于当前开发机复现。
