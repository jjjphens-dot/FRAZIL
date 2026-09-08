# FRAZIL 环境策略

本文档区分 FRAZIL 项目要求与某一台参考机器的验证证据。除非明确标为参考证据，文中的路径均为 repository-relative path。

## Project Requirements

- Windows x64 开发环境。
- CMake 3.25 或更高版本。
- Ninja；可以使用系统 PATH 中的 Ninja，也可以把本地副本放在 repository-local 的 tools/bin。
- MSVC v143 和 Windows SDK，且 MSVC developer environment 已初始化，使 cl、rc 和 mt 可以被工具发现。
- Python 用于 DSP 实验和跨平台工具；Python 依赖见 requirements-dsp.txt。
- JUCE 9.0.1 由 tools/bootstrap_dependencies.ps1 获取和校验；external/JUCE 是生成的本地依赖目录，不提交到 FRAZIL 主仓库。
- pluginval 仅在执行 VST3 验证时需要；工具版本和下载来源由验证记录维护。

Visual Studio IDE 不是项目必需品。VS Code、Developer PowerShell for Visual Studio 或 x64 Native Tools command prompt 均可作为开发入口；关键是 CMake 能通过 PATH 找到所需工具。

## Repository Layout

把 <repo-root> 作为克隆后的 FRAZIL 根目录：

    <repo-root>/build
    <repo-root>/.venv
    <repo-root>/external/JUCE
    <repo-root>/tools/bin
    <repo-root>/tools/downloads
    <repo-root>/testdata/rendered

build、.venv、external/JUCE、tools/bin、tools/downloads 和 testdata/rendered 均为本地或生成内容，不提交到 Git。构建目录也可以由被忽略的 CMakeUserPresets.json 指定到开发者自己的位置。

## Shared Configuration and Local Configuration

tracked 的 CMakePresets.json、.vscode/tasks.json 和 CI workflow 不包含开发者个人路径。共享 Windows preset 继承 portable-windows-base：

- 编译器和 Windows SDK 工具使用 cl、rc、mt 的 tool discovery；
- Ninja 使用 PATH discovery，并优先搜索 <repo-root>/tools/bin；
- 构建目录使用 <repo-root>/build/<preset-name>；
- windows-debug、windows-release、windows-asan 和 ci-windows-debug 共用该基础配置。

如果某台机器需要特殊的 compiler、SDK、Ninja 或 build directory，应使用已忽略的 CMakeUserPresets.json 或开发者自己的环境变量。不要修改 tracked preset 来提交个人安装位置。

## Standard Local Workflow

在已初始化 MSVC developer environment 的 PowerShell 中，并从 cloned FRAZIL repository root 执行：

    .\tools\bootstrap_dependencies.ps1
    python tools/verify_testdata.py
    cmake --list-presets
    cmake --preset windows-debug
    python tools/build_safe.py --preset windows-debug
    ctest --preset windows-debug

其他配置：

    cmake --preset windows-release
    python tools/build_safe.py --preset windows-release
    ctest --preset windows-release

    cmake --preset windows-asan
    python tools/build_safe.py --preset windows-asan
    ctest --preset windows-asan

ASAN configure 会从 C++ 编译器位置发现 MSVC runtime directory；测试 target 会把 clang_rt.asan_dynamic-x86_64.dll 复制到可执行文件旁，并为 CTest 注入同一目录。因此构建仍需 VS Code/MSVC developer environment，但构建完成后可从普通 PowerShell、VS Code 测试面板或可执行文件目录运行 ASAN 测试。

本地 build 必须通过 tools/build_safe.py；默认使用 6 个 job，硬上限 8 个 job，并按可用物理内存执行 preflight。wrapper 将完整编译输出写入 ignored 的 build/safe-build 日志，避免终端被 include trace 淹没。 共享 configure preset 将 CMAKE_BUILD_PARALLEL_LEVEL 固定为 6，用于约束 JUCE configure 阶段的 nested build；本机重型 pipeline 必须串行执行。

## VS Code

在仓库根目录打开 VS Code：

- 在 Terminal profile 中选择 FRAZIL MSVC x64；该 profile 通过 vswhere 动态发现 Visual Studio，并调用仓库内的 tools/vscode_msvc_env.cmd；
- 首次 checkout 运行 Task: FRAZIL: configure windows-debug (MSVC)；
- Ctrl+Shift+B 调用 FRAZIL: build windows-debug (safe)，F5 的 preLaunchTask 使用同一个受控入口；
- Run and Debug 中选择 FRAZIL Standalone (Debug)；
- task 和 launch configuration 使用 workspaceFolder，不依赖开发者的 clone 位置或个人 Visual Studio 安装盘符。

## pluginval

完成 Debug VST3 构建后，可以从 repository-local 工具位置执行：

    $ProjectRoot = (Resolve-Path .).Path
    $pluginval = Join-Path $ProjectRoot 'tools/bin/pluginval.exe'
    $plugin = Join-Path $ProjectRoot 'build/windows-debug/FRAZIL_artefacts/Debug/VST3/FRAZIL.vst3'
    & $pluginval --strictness-level 5 --validate $plugin

如果 pluginval 位于系统 PATH，也可以直接调用 pluginval。验证结果必须记录工具版本、插件构建类型、commit 和实际结果。

## Portable CI Workflow

GitHub Actions 和其他已初始化 MSVC developer environment 的 Windows 机器使用：

    .\tools\bootstrap_dependencies.ps1
    python tools/check_portability.py
    python tools/check_markdown_links.py
    python tools/check_vscode_tasks.py
    cmake --preset ci-windows-debug
    python tools/build_safe.py --preset ci-windows-debug
    ctest --preset ci-windows-debug

ci-windows-debug 不引用个人盘符、用户名或工具安装目录。CI 在 configure 前运行 portability scan。

## Reference Machine

以下只用于解释历史验证环境，不是 FRAZIL 项目要求，也不构成任何安装路径合同：

- OS：Windows 11 23H2，Build 22631；
- Visual Studio toolchain：MSVC v143，版本 19.43.34809；
- Windows SDK：10.0.22621.0；
- CMake/CTest：4.3.2；
- Ninja：1.13.2；
- Python：3.12.4；
- JUCE：9.0.1，固定 commit 由 bootstrap 脚本校验；
- pluginval：1.0.4；
- 这台机器的历史 Debug、Release、ASAN 和基础 CTest 结果属于 reference-machine-only evidence；新的结果必须用实际命令和日期重新记录。

Reference-machine-only evidence. Not part of the FRAZIL project contract.
