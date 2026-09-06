# FRAZIL 开发环境配置与问题记录

> 本文档是独立的环境配置文档，不替代、不修改 FRAZIL 项目架构文档。
>
> 适用范围：Windows 11、VS Code、CMake/Ninja、JUCE、MSVC、VST3/Standalone 开发。
>
> 当前验证日期：2026-09-06

## 1. 配置原则

- IDE 使用 VS Code。
- 不新增、升级或依赖 Visual Studio IDE 工作流。
- Windows 原生 C++ 编译仍使用现有的 MSVC v143 和 Windows SDK；它们来自机器上已有的 Visual Studio Community 工具链安装。
- 项目依赖、构建输出、Python 虚拟环境和本地工具统一放在 F: 盘。
- 不向全局 Python 环境安装 DSP 依赖。
- 不把 Ninja、pluginval 等二进制工具提交到 Git 仓库。

“不安装 Visual Studio IDE”与“使用 MSVC 编译器”并不冲突：VS Code 负责编辑和调试界面，MSVC/Windows SDK 负责 Windows 原生编译和链接。

## 2. 磁盘布局

项目根目录：

```text
F:\coding\FRAZIL
├─ .venv\                         项目专用 Python 环境
├─ build\
│  ├─ windows-debug\              Debug 构建
│  ├─ windows-release\            Release 构建
│  └─ windows-asan\               AddressSanitizer 构建
├─ external\JUCE\                JUCE 9.0.1
├─ tools\
│  ├─ bin\ninja.exe              Ninja 1.13.2
│  ├─ bin\pluginval.exe           pluginval 1.0.4
│  └─ downloads\                  下载归档，不提交到 Git
├─ src\
├─ tests\
├─ CMakeLists.txt
├─ CMakePresets.json
└─ FRAZIL_ENVIRONMENT_SETUP.md
```

系统中已经存在的工具仍可能位于 C: 或 D: 盘，例如 CMake、Python、Windows 系统文件和 LLVM；本次没有迁移或重装它们。新增的项目文件、缓存和依赖均放在 F: 盘。

## 3. 已验证工具版本

### 3.1 系统工具链

| 工具 | 版本/位置 | 用途 |
|---|---|---|
| Windows | Windows 11 23H2，Build 22631 | 操作系统 |
| VS Code | 1.136.1，`D:\Microsoft VS Code` | IDE |
| Git | 2.46.0，`D:\Git` | 版本控制和依赖获取 |
| CMake | 4.3.2 | 配置和生成构建文件 |
| CTest | 4.3.2 | 自动化测试 |
| MSVC | 19.43.34809，`F:\Visual Studio\Community` | C/C++ 编译器和链接器 |
| Windows SDK | 10.0.22621.0，`F:\Windows Kits\10` | Windows 头文件、资源编译器、Manifest 工具 |
| LLVM | 19.1.3，`D:\LLVM` | clang-format/clang-tidy |
| Python | 3.12.4，`C:\Program Files\python` | 创建项目虚拟环境和 DSP 实验 |

### 3.2 项目本地工具和依赖

| 工具/依赖 | 版本 | 项目内位置 |
|---|---:|---|
| Ninja | 1.13.2 | `F:\coding\FRAZIL\tools\bin\ninja.exe` |
| JUCE | 9.0.1 | `F:\coding\FRAZIL\external\JUCE` |
| pluginval | 1.0.4 | `F:\coding\FRAZIL\tools\bin\pluginval.exe` |
| NumPy | 2.5.2 | `.venv` |
| SciPy | 1.18.1 | `.venv` |
| soundfile | 0.14.0 | `.venv` |
| matplotlib | 3.11.1 | `.venv` |

Ninja 使用官方发布包，pluginval 使用 Tracktion 官方发布包。协作者重新下载时应固定版本，不要直接追踪未验证的最新版。

## 4. 首次配置步骤

以下命令假设在 PowerShell 中执行，并且当前目录为项目根目录。

### 4.1 创建项目目录

```powershell
Set-Location F:\coding\FRAZIL

New-Item -ItemType Directory -Force `
  F:\coding\FRAZIL\tools\bin, `
  F:\coding\FRAZIL\tools\downloads, `
  F:\coding\FRAZIL\external | Out-Null
```

### 4.2 创建 Python 虚拟环境

使用已有的系统 Python 创建副本式虚拟环境，避免项目运行时依赖系统 Python 的目录结构：

```powershell
$ProjectRoot = 'F:\coding\FRAZIL'
$PythonExe = 'C:\Program Files\python\python.exe'

& $PythonExe -m venv --copies "$ProjectRoot\.venv"
$env:PIP_CACHE_DIR = "$ProjectRoot\.cache\pip"

& "$ProjectRoot\.venv\Scripts\python.exe" -m pip install --upgrade pip
& "$ProjectRoot\.venv\Scripts\python.exe" -m pip install -r "$ProjectRoot\requirements-dsp.txt"
```

验证：

```powershell
& .\.venv\Scripts\python.exe -c "import numpy, scipy, soundfile, matplotlib; print(numpy.__version__, scipy.__version__, soundfile.__version__, matplotlib.__version__)"
```

### 4.3 安装 Ninja 到 F 盘

```powershell
$NinjaZip = 'F:\coding\FRAZIL\tools\downloads\ninja-win.zip'

Invoke-WebRequest `
  -Uri 'https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-win.zip' `
  -OutFile $NinjaZip

Expand-Archive -LiteralPath $NinjaZip `
  -DestinationPath 'F:\coding\FRAZIL\tools\bin' -Force

& 'F:\coding\FRAZIL\tools\bin\ninja.exe' --version
```

### 4.4 获取 JUCE

全新 checkout 不要依赖工作区中偶然存在的 `external\JUCE`，执行仓库脚本：

```powershell
& F:\coding\FRAZIL\tools\bootstrap_dependencies.ps1
```

脚本固定 JUCE 9.0.1 commit，并应用仓库内 `tools\patches` 的兼容补丁；重新获取 JUCE 后仍使用该脚本校验和恢复。

### 4.5 获取 pluginval

```powershell
$PluginvalZip = 'F:\coding\FRAZIL\tools\downloads\pluginval_Windows.zip'

Invoke-WebRequest `
  -Uri 'https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip' `
  -OutFile $PluginvalZip

Expand-Archive -LiteralPath $PluginvalZip `
  -DestinationPath 'F:\coding\FRAZIL\tools\bin' -Force
```

解压后确保最终文件存在：

```text
F:\coding\FRAZIL\tools\bin\pluginval.exe
```

## 5. CMake 和 MSVC 配置

项目使用 [CMakePresets.json](F:/coding/FRAZIL/CMakePresets.json)，不依赖 CMake GUI，也不依赖 Visual Studio IDE。
GitHub Actions 或其他标准 Windows 环境使用 `ci-windows-debug` portable preset；本机固定 F: 工具链使用 `windows-debug`、`windows-release` 和 `windows-asan`。

预设中固定了以下路径：

```text
CMAKE_MAKE_PROGRAM = F:/coding/FRAZIL/tools/bin/ninja.exe
CMAKE_C_COMPILER  = F:/Visual Studio/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x64/cl.exe
CMAKE_CXX_COMPILER = F:/Visual Studio/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x64/cl.exe
CMAKE_RC_COMPILER = F:/Windows Kits/10/bin/10.0.22621.0/x64/rc.exe
CMAKE_MT = F:/Windows Kits/10/bin/10.0.22621.0/x64/mt.exe
```

构建前仍建议调用 MSVC 环境脚本：

```powershell
$vcvars = 'F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat'

cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --preset windows-debug'
```

注意：必须在 `vcvars64.bat` 执行完成后再把项目 Ninja 目录放到 PATH 前面。否则 `vcvars64.bat` 可能覆盖 PATH，导致 CMake 调用到其他位置的 Ninja。

## 6. 构建、测试和插件验证

### 6.1 Debug

```powershell
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --preset windows-debug'
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --build --preset windows-debug --parallel 4'
ctest --preset windows-debug
```

Debug 输出：

```text
F:\coding\FRAZIL\build\windows-debug\FRAZIL_artefacts\Debug\Standalone\FRAZIL.exe
F:\coding\FRAZIL\build\windows-debug\FRAZIL_artefacts\Debug\VST3\FRAZIL.vst3
```

### 6.2 Release

```powershell
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --preset windows-release'
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --build --preset windows-release --parallel 4'
ctest --preset windows-release
```

### 6.3 AddressSanitizer

```powershell
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --preset windows-asan'
cmd /d /c 'call "F:\Visual Studio\Community\VC\Auxiliary\Build\vcvars64.bat" && set "PATH=F:\coding\FRAZIL\tools\bin;%PATH%" && cmake --build --preset windows-asan --parallel 4'
ctest --preset windows-asan
```

ASAN 测试运行时需要把 MSVC 的运行时目录加入 PATH；该配置已经写入 `windows-asan` test preset：

```text
F:/Visual Studio/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x64
```

### 6.4 pluginval

```powershell
& 'F:\coding\FRAZIL\tools\bin\pluginval.exe' `
  --strictness-level 5 `
  --validate 'F:\coding\FRAZIL\build\windows-debug\FRAZIL_artefacts\Debug\VST3\FRAZIL.vst3'
```

当前验证覆盖了冷启动、热启动、编辑器、状态保存恢复、参数自动化、音频处理、总线布局，以及 44100/48000/96000 Hz 和多种 block size。

## 7. 本次配置遇到的问题及处理方式

### 问题 1：JUCE 配置阶段缺少 C 语言编译器

**现象**：首次 CMake 配置 JUCE 目标时失败。

**原因**：FRAZIL 的顶层 CMake 项目最初只声明了 C++，而 JUCE 会编译部分 C 源文件。

**修复**：将顶层项目声明改为：

```cmake
project(FRAZIL VERSION 0.1.0 LANGUAGES C CXX)
```

### 问题 2：JUCE 嵌套项目没有继承 F 盘工具链

**现象**：顶层 CMake 能识别 F 盘的 MSVC、Windows SDK 和 Ninja，但 JUCE 的 `juceaide` 或 VST3 manifest helper 在嵌套配置阶段找不到 `mt.exe`，或者调用了 C 盘其他位置的 Ninja。

**原因**：JUCE 的部分辅助工程通过嵌套 CMake 调用生成，未自动继承本项目明确指定的 `CMAKE_MT`、编译器和资源编译器路径。

**修复**：两个局部兼容补丁保存在仓库的
`tools\patches\JUCE-9.0.1-msvc-toolchain.patch`，由 bootstrap 脚本幂等应用：

1. `external/JUCE/extras/Build/CMake/JUCEUtils.cmake`
   - VST3 manifest helper 的嵌套 CMake 调用显式传递 `-DCMAKE_MT=${CMAKE_MT}`。
2. `external/JUCE/extras/Build/juceaide/CMakeLists.txt`
   - `juceaide` 嵌套配置显式传递 `CMAKE_C_COMPILER`、`CMAKE_CXX_COMPILER`、`CMAKE_RC_COMPILER` 和 `CMAKE_MT`。

这不是 FRAZIL 业务代码修改，而是为了支持“MSVC/Windows SDK 安装在 F 盘”的 JUCE 构建兼容补丁。若更换 JUCE 版本，应更新 patch、commit 和 ADR，并重新验证。

### 问题 3：PATH 顺序导致调用了错误的 Ninja

**现象**：明明项目内有 `tools\bin\ninja.exe`，但构建日志显示 CMake 调用了其他位置的 Ninja。

**原因**：先设置项目 PATH，再运行 `vcvars64.bat` 时，Visual Studio 环境脚本重新整理了 PATH。

**修复**：始终采用以下顺序：

```text
先 call vcvars64.bat
再把 F:\coding\FRAZIL\tools\bin 放到 PATH 前面
最后执行 cmake
```

### 问题 4：ASAN 配置误选 Clang

**现象**：ASAN 配置使用了 LLVM 的 Clang，但预设中的 `/fsanitize=address` 等参数按 MSVC 方式编写，导致配置或编译失败。

**原因**：PATH 中的 LLVM 排在 MSVC 工具链前，CMake 自动选择了 Clang。

**修复**：本机 Windows 预设明确指定 MSVC 的 `cl.exe`、`rc.exe` 和 `mt.exe`；portable CI 预设从 MSVC developer environment 解析 `cl.exe`，并通过 PATH 使用 `rc`/`mt`，ASAN 继续使用 MSVC AddressSanitizer。

### 问题 5：ASAN CTest 找不到运行时 DLL

**现象**：ASAN 构建成功，但 CTest 返回 `0xc0000135`，提示无法启动测试程序。

**原因**：测试进程找不到 MSVC AddressSanitizer 运行时 DLL。

**修复**：在 `windows-asan` test preset 的 PATH 前面加入：

```text
F:\Visual Studio\Community\VC\Tools\MSVC\14.43.34808\bin\Hostx64\x64
```

修复后 ASAN smoke test 通过。

### 问题 6：默认构建目标可能漏掉 Standalone

**现象**：VST3 已生成，但 Standalone 目录为空或没有最新的可执行文件。

**原因**：JUCE 同时生成多个格式目标，直接依赖 Ninja 默认目标不够明确。

**修复**：`CMakePresets.json` 的 Debug、Release、ASAN build preset 均指定：

```json
"targets": ["FRAZIL_All", "frazil_smoke", "frazil_tests"]
```

这样协作者使用 `cmake --build --preset ...` 时会同时构建 Standalone 和 VST3。

### 问题 7：pluginval 跳过 Steinberg VST3 Validator

**现象**：pluginval 输出 `Skipping vst3 validator as validator path hasn't been set`。

**原因**：尚未单独安装或配置 Steinberg Validator。

**处理**：这不是当前日常编译、调试和 pluginval 基础验证的阻塞项。若后续发布前需要更严格的 VST3 合规验证，再单独获取并配置该工具。

## 8. VS Code 使用方式

打开目录：

```text
F:\coding\FRAZIL
```

推荐扩展由 `.vscode\extensions.json` 提供，核心扩展为：

- C/C++
- CMake Tools
- ChatGPT/Codex 扩展（如果协作者使用）

项目已提供：

- `.vscode\tasks.json`：`Ctrl+Shift+B` 构建 `windows-debug`。
- `.vscode\launch.json`：选择 `FRAZIL Standalone (Debug)` 后按 F5，构建并启动 Standalone。
- `CMakePresets.json`：统一管理 Debug、Release、ASAN。

## 9. 当前验证结论

截至 2026-09-06：

- Debug 构建通过。
- Release 构建通过。
- Debug、Release、ASAN 的 `frazil_smoke` CTest 通过；Debug 还验证了 `frazil_tests` 的 2 个 unit cases。
- Debug 和 Release VST3 均通过 pluginval 严格度 5。
- Standalone 和 VST3 产物均可生成。
- 日常 VS Code 编译、Standalone 调试和 VST3 验证链路可用。

尚未覆盖：

- 在真实 DAW（例如 REAPER）中手工加载插件。
- 真实音频设备、ASIO 驱动和多设备切换测试。
- 更完整的 Catch2 v3 单元测试框架尚未接入；当前仓库已有 `frazil_tests` M0 target。
- Steinberg VST3 Validator 独立验证。
