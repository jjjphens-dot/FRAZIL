# FRAZIL 环境策略

本文档区分 FRAZIL 项目要求与某一台参考机器的验证证据。除非明确标为参考证据，文中的路径均为 repository-relative path。

## Project Requirements

- Windows x64 开发环境。
- CMake 3.25 或更高版本。
- Ninja；可以使用系统 PATH 中的 Ninja，也可以把本地副本放在 repository-local 的 tools/bin。
- MSVC v143 和 Windows SDK，且 MSVC developer environment 已初始化，使 cl、rc 和 mt 可以被工具发现。
- Python 用于 DSP 实验和跨平台工具；Python 依赖见 requirements-dsp.txt。
- 启用 `FRAZIL_BUILD_WATER_EXPERIMENT=ON` 时，先运行 `python -m pip install -r requirements-dsp.txt`；
  Protect listening-pack CTest 使用其中的 NumPy/SoundFile 并调用真实 research renderer。Hosted CI 同样安装
  该依赖文件；安装依赖的解释器必须与 CMake 的 `Python3_EXECUTABLE` 和生成的 CTest 命令一致。
  未启用 research 的 production build 不新增 Python 包依赖。
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

build、.venv、external/JUCE、tools/bin、tools/downloads 和 testdata/rendered 均为本地或生成内容，不提交到 Git。构建目录也可以由被忽略的 CMakeUserPresets.json 指定到下述工作区边界内的本地位置。

## Local Workspace Boundary

`<workspace-root>` 是用户指定的本地工作区父目录，`<repo-root>` 是其中一个 FRAZIL 工作树。
具体路径按用户指示或工作区上级 `AGENTS.md` 所在位置在本机解析；未指定时使用主工作树的父目录。
不同工作机使用各自实际的工作区绝对路径，不能把某台机器的盘符或用户名固化为共享规则。
linked worktree 应通过 `git worktree list` 和 `git rev-parse --path-format=absolute --git-common-dir`
确认主仓库归属，不将临时 checkout 的位置当作新的工作区边界。

- 新分支可以留在已有工作树中；需要隔离时，只在 `<workspace-root>` 内创建 worktree、review checkout
  或临时 clone，例如 `<workspace-root>/FRAZIL-<task>`。
- build、render、日志和其他项目验证输出也必须在该边界内；优先使用 `<repo-root>/build/`，本地 preset
  不能被用来绕过目录限制。创建、复制和移动前检查绝对目标路径及 junction/symlink 的实际指向。
- 只有工具硬限制或复现要求使目录内方案确实不可行时才允许例外；执行前说明必要性、已排除的目录内
  替代方案、外部位置和收尾方式。已有用户授权有效，不因例外本身机械要求再次确认。仅为缩短路径或
  操作方便，不得另选磁盘根目录、用户目录或临时目录。
- 已有外部副本不自动移动或删除；系统工具、用户提供的附件和平台管理的缓存不因本规则而迁移。
  托管 CI 继续使用 runner 提供的工作区，不依赖本机路径。

上级目录说明优先以自身位置为锚点，仓库内路径使用相对路径或上述占位符。执行时用 `Resolve-Path`、
`Join-Path` 等解析并检查目标绝对路径；确需机器专属绝对路径时，仅使用 ignored 配置或本机环境变量。
这不放宽现有禁止在 tracked source/config/script/canonical documentation 中提交个人绝对路径的规则。
工作树迁移应保留源码及本地证据，修复 Git worktree 登记，并重新生成引用旧位置的 CMake 配置后验证。

## Shared Configuration and Local Configuration

tracked 的 CMakePresets.json、.vscode/tasks.json 和 CI workflow 不包含开发者个人路径。共享 Windows preset 继承 portable-windows-base：

- 编译器和 Windows SDK 工具使用 cl、rc、mt 的 tool discovery；
- Ninja 使用 PATH discovery，并优先搜索 <repo-root>/tools/bin；
- 构建目录使用 <repo-root>/build/<preset-name>；
- windows-debug、windows-release、windows-asan 和 ci-windows-debug 共用该基础配置。

如果某台机器需要特殊的 compiler、SDK、Ninja 或 build directory，应使用已忽略的 CMakeUserPresets.json 或开发者自己的环境变量。build directory 仍受 Local Workspace Boundary 约束。不要修改 tracked preset 来提交个人安装位置。

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

CTest also runs the repository's RENDER-001 offline smoke. It writes generated
WAV output and its audit manifest only under the ignored preset build tree,
such as `build/windows-debug/rendered/`; no audio device or DAW is required.
Manual `tools/render_testdata.py` runs default to the separate ignored
`testdata/rendered/` directory.

启用 Water research 时，在同一个 MSVC developer PowerShell 中解析一次解释器，并将同一个路径用于
安装依赖和 configure。不要假设 PATH 的 `python` 与 CMake 自动发现的最高版本相同，也不需要安装多套 Python：

```powershell
$frazilPython = python -c "import sys; print(sys.executable)"
if ($LASTEXITCODE -ne 0) { throw 'Python discovery failed' }
& $frazilPython -m pip install -r requirements-dsp.txt
if ($LASTEXITCODE -ne 0) { throw 'DSP dependency installation failed' }
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON "-DPython3_EXECUTABLE:FILEPATH=$frazilPython"
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
& $frazilPython tools/build_safe.py --preset windows-debug
if ($LASTEXITCODE -ne 0) { throw 'Safe build failed' }
ctest --preset windows-debug --output-on-failure
```

引号保留带空格的 executable 路径。Release/ASAN 如需启用 research，也用同一变量显式 configure 对应
preset，串行构建/测试。可从对应 `build/<preset>/CMakeCache.txt` 的 `Python3_EXECUTABLE` 与
`ctest --preset <preset> -R '^frazil_water_protect_listening$' --show-only=json-v1` 的 command 首项复核路径。

ASAN configure 会从 C++ 编译器位置发现 MSVC runtime directory；测试 target 会把 clang_rt.asan_dynamic-x86_64.dll 复制到可执行文件旁，并为 CTest 注入同一目录。因此构建仍需 VS Code/MSVC developer environment，但构建完成后可从普通 PowerShell、VS Code 测试面板或可执行文件目录运行 ASAN 测试。

本地 build 必须通过 tools/build_safe.py；默认使用 6 个 job，硬上限 8 个 job，并按可用物理内存执行 preflight。wrapper 将完整编译输出写入 ignored 的 build/safe-build 日志，避免终端被 include trace 淹没。 共享 configure preset 将 CMAKE_BUILD_PARALLEL_LEVEL 固定为 6，用于约束 JUCE configure 阶段的 nested build；本机重型 pipeline 必须串行执行。

若本地中文 MSVC 的 `/showIncludes` 在 Ninja rules 中显示乱码，且 `ninja -t deps` 显示包含头文件的
object 为零依赖，不得信任 header-only 修改后的增量结果。在同一 developer shell 执行 `chcp 65001`，
再 `cmake --fresh --preset <preset>`（保留所需 configure options），通过安全 wrapper 重新构建并确认
依赖列表实际包含修改的头文件；必要时使用新的本地 build tree。此为本机编码诊断，不修改共享并发限制。

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
    $frazilPython = python -c "import sys; print(sys.executable)"
    & $frazilPython -m pip install -r requirements-dsp.txt
    cmake --preset ci-windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON "-DPython3_EXECUTABLE:FILEPATH=$frazilPython"
    python tools/build_safe.py --preset ci-windows-debug
    ctest --preset ci-windows-debug

Hosted CI explicitly enables the standalone Water research targets so their property and decoded-render
tests run alongside the production regression suite. Local builds leave the option OFF unless explicitly
requested; details are in [the research README](../experiments/water/SPIKE-W-DSP-001/README.md).

Hosted CI captures `sys.executable` once, installs dependencies with it and passes that exact executable through
`FRAZIL_CI_PYTHON` to CMake. Configure checks the cache and generated listening-test command against the captured
path, failing on mismatch or missing test registration, and logs all three paths. No interpreter version or
machine-specific path is hardcoded; the unchanged listening regression still executes in the full CTest suite.

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

## Standalone Water engineering preview

To build the separate research GUI and its regression, add
`-DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON` to a standard configure command,
then use the unchanged `tools/build_safe.py --preset <preset>` and CTest commands. Both options default
OFF; preview without the experiment option is a configure error. The output is
`build/<preset>/experiments/water/SPIKE-W-DSP-001/frazil_water_preview_artefacts/<config>/FRAZIL Water Research Preview.exe`.
It uses the default stereo output at the loaded WAV sample rate, with no microphone or resampling.
See the [debugging guide](DEV_UI_WATER_DEBUG_GUIDE.md). CI opts in to the preview and its device-free test;
it does not claim physical-device or GUI acceptance.
