# FRAZIL 开发环境配置与验证指南

> 目标：让仓库可以被 clone 到任意正常 Windows 路径，并在满足项目工具要求后完成 bootstrap、configure、build、test 和基本 VS Code 开发流程。
>
> 本文中的 <repo-root> 表示开发者实际打开的 FRAZIL repository root，不是固定盘符或固定用户名。

## 1. 工具要求

- Windows x64；
- Git、PowerShell、CMake 3.25+、CTest 和 Ninja；
- MSVC v143、Windows SDK，以及已初始化的 MSVC developer environment；
- Python 和 requirements-dsp.txt 中的 DSP 实验依赖；
- JUCE 9.0.1 由仓库 bootstrap 脚本恢复；
- pluginval 仅在执行插件验证时需要。

Visual Studio IDE 不是必需品。本项目推荐直接在 VS Code 中选择 FRAZIL MSVC x64 Terminal profile；workspace 会通过 vswhere 动态发现 Visual Studio 和 Windows SDK，不需要从 Developer PowerShell 重新启动 VS Code。

## 2. 打开仓库并确认根目录

在 cloned FRAZIL repository root 打开 PowerShell，然后确认：

    $ProjectRoot = (Resolve-Path .).Path
    $ProjectRoot
    git status --short

后续命令都从该目录执行。不要把 clone 位置写入 tracked 配置、脚本或 canonical documentation。

## 3. 本地目录约定

项目使用 repository-relative 目录：

    <repo-root>/build
    <repo-root>/.venv
    <repo-root>/external
    <repo-root>/tools/bin
    <repo-root>/tools/downloads
    <repo-root>/testdata/rendered

这些目录由脚本或工具生成，已在 .gitignore 中排除。开发者若需要把 build 放到其他位置，应使用被忽略的 CMakeUserPresets.json。

## 4. Python 虚拟环境

从 repository root 执行：

    $ProjectRoot = (Resolve-Path .).Path
    $Python = Get-Command python.exe -ErrorAction Stop
    $venv = Join-Path $ProjectRoot '.venv'
    & $Python.Source -m venv --copies $venv
    & (Join-Path $venv 'Scripts/python.exe') -m pip install --upgrade pip
    & (Join-Path $venv 'Scripts/python.exe') -m pip install -r (Join-Path $ProjectRoot 'requirements-dsp.txt')

验证：

    & (Join-Path $venv 'Scripts/python.exe') -c "import numpy, scipy, soundfile, matplotlib; print('DSP dependencies ready')"

不要把个人 Python 安装目录写入 FRAZIL 文件。

## 5. Ninja 和本地工具

可以使用系统 PATH 中的 Ninja；共享 preset 会先搜索 repository-local tools/bin。若团队决定使用 repository-local Ninja，可从官方发布页获取固定版本：

    $ProjectRoot = (Resolve-Path .).Path
    $toolsBin = Join-Path $ProjectRoot 'tools/bin'
    $toolsDownloads = Join-Path $ProjectRoot 'tools/downloads'
    New-Item -ItemType Directory -Force -Path $toolsBin, $toolsDownloads | Out-Null
    $ninjaArchive = Join-Path $toolsDownloads 'ninja-win.zip'
    Invoke-WebRequest -Uri 'https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-win.zip' -OutFile $ninjaArchive
    Expand-Archive -LiteralPath $ninjaArchive -DestinationPath $toolsBin -Force
    & (Join-Path $toolsBin 'ninja.exe') --version

pluginval 也应解压到 repository-local tools/bin 或安装到 PATH，版本由验证记录固定：

    $pluginvalArchive = Join-Path $toolsDownloads 'pluginval_Windows.zip'
    Invoke-WebRequest -Uri 'https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip' -OutFile $pluginvalArchive
    Expand-Archive -LiteralPath $pluginvalArchive -DestinationPath $toolsBin -Force

工具下载归档和二进制不提交到 Git。

## 6. JUCE bootstrap

全新 checkout 或 external/JUCE 缺失时执行：

    $ProjectRoot = (Resolve-Path .).Path
    & (Join-Path $ProjectRoot 'tools/bootstrap_dependencies.ps1')

脚本会获取 JUCE 9.0.1 固定 commit，并幂等应用仓库内的兼容补丁。不要依赖另一台机器上偶然存在的 external/JUCE。

## 7. MSVC tool discovery

在 VS Code 中打开 FRAZIL MSVC x64 Terminal profile 后验证：

    Get-Command cl.exe
    Get-Command rc.exe
    Get-Command mt.exe

CMakePresets.json 使用 cl、rc、mt 的 tool discovery，不使用任何个人 Visual Studio 或 Windows SDK 安装路径。若系统安装了多个 toolchain，选择正确的 developer environment，或在被忽略的 CMakeUserPresets.json 中配置本地差异。

## 8. Configure、build 和 test

    cmake --list-presets
    cmake --preset windows-debug
    python tools/build_safe.py --preset windows-debug
    ctest --preset windows-debug

Release 和 ASAN：

    cmake --preset windows-release
    python tools/build_safe.py --preset windows-release
    ctest --preset windows-release

    cmake --preset windows-asan
    python tools/build_safe.py --preset windows-asan
    ctest --preset windows-asan

ASAN CTest 通过 VCToolsInstallDir 找到 MSVC runtime directory；这要求测试命令继承已初始化的 MSVC environment。

本地构建统一使用 tools/build_safe.py；默认 6 个 job、硬上限 8 个 job，并在低可用内存时拒绝启动。完整编译输出写入 ignored 的 build/safe-build 日志。 共享 configure preset 将 CMAKE_BUILD_PARALLEL_LEVEL 固定为 6，以约束 JUCE nested build；本机重型 configure/build/test pipeline 必须串行执行。

## 9. Portability scan

提交前和本地验证时运行：

    python tools/check_portability.py
    python tools/check_markdown_links.py
    python tools/check_vscode_tasks.py

scanner 检查 tracked source/config/script/canonical documentation 中的 Windows、Linux user、macOS user、UNC 和绝对 Markdown link 路径；它排除 .git、build、external、.venv、tools/bin、tools/downloads 和 generated/rendered/binary files。发现未经允许的路径时返回非零退出码。

当前仓库不提供 absolute-path allowlist；tracked source/config/script/canonical documentation 中发现的机器相关绝对路径一律失败。未来若确有 reference-machine evidence 需求，必须先设计针对明确文件的窄范围机制，不得在 build/test/runtime/CI/tooling 文件中绕过扫描。

## 10. VS Code

在 <repo-root> 打开 VS Code：

- 在 VS Code Terminal profile 中选择 FRAZIL MSVC x64；该 profile 通过 vswhere 动态发现 Visual Studio，并调用仓库内的 tools/vscode_msvc_env.cmd；
- 首次 checkout 运行 Task: FRAZIL: configure windows-debug (MSVC)；
- Ctrl+Shift+B 执行 FRAZIL: build windows-debug (safe)，F5 的 preLaunchTask 使用同一个受控入口；
- Run and Debug 选择 FRAZIL Standalone (Debug)；
- task 使用 workspaceFolder，不依赖开发者的 clone 位置或个人 Visual Studio 安装盘符。

VS Code workspace profile 和 task 会自动加载 MSVC developer environment，不需要把 VS Code 从 Developer PowerShell 重新启动。

## 11. pluginval

    $ProjectRoot = (Resolve-Path .).Path
    $pluginval = Join-Path $ProjectRoot 'tools/bin/pluginval.exe'
    $plugin = Join-Path $ProjectRoot 'build/windows-debug/FRAZIL_artefacts/Debug/VST3/FRAZIL.vst3'
    & $pluginval --strictness-level 5 --validate $plugin

也可以使用 PATH 中的 pluginval。验证报告应记录版本、commit、build type 和实际结果。

## 12. 常见问题

- configure 找不到 cl、rc 或 mt：确认当前 VS Code terminal 使用 FRAZIL MSVC x64 profile，并重新运行 FRAZIL: configure windows-debug (MSVC)；
- configure 找不到 Ninja：安装 Ninja，或把固定版本放入 tools/bin；
- JUCE 缺失或 revision 不匹配：重新运行 tools/bootstrap_dependencies.ps1；
- ASAN 测试找不到 runtime DLL：确认命令继承 VCToolsInstallDir，且使用 windows-asan test preset；
- VS Code build task 找不到 build tree：先运行 cmake --preset windows-debug；
- 不要把上述问题通过修改 tracked preset 改成某个个人绝对路径。

Tracked reference-machine evidence 只能记录工具版本、OS 信息和泛化后的路径描述，不得保存开发者原始安装目录、用户名、盘符或其它 raw absolute paths。
如本地诊断确需保存这些信息，应放在 ignored/untracked local evidence 中。
