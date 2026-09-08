# tools/

项目相关外部工具的配置位置。

## 固定依赖恢复

JUCE 不随 FRAZIL 主仓库提交。全新 checkout 先执行：

```powershell
.\tools\bootstrap_dependencies.ps1
```

脚本会校验 JUCE 9.0.1 commit
`e18f7f506c0b96f2c738a0bcd7fe6467a5005ad8`，并幂等应用
`tools/patches/JUCE-9.0.1-msvc-toolchain.patch`。构建输出和本地工具二进制不应
提交；版本与来源记录在 `docs/ENVIRONMENT.md`。

Build safety:

    python tools/build_safe.py --preset windows-debug

The wrapper is the only approved local build entry: it refuses more than eight
jobs and defaults to six, refuses insufficient or unknown physical memory status, and writes full compiler
output to ignored build/safe-build/<preset>.log. It prints only a bounded tail
when the build fails. The shared configure preset also sets CMAKE_BUILD_PARALLEL_LEVEL=6 for JUCE nested builds. Do not replace it with a bare --parallel invocation, and do not run multiple heavy configure/build/test pipelines concurrently.
Portability and documentation checks：

    python tools/check_portability.py
    python tools/test_check_portability.py
    python tools/check_markdown_links.py
    python tools/test_check_markdown_links.py
    python tools/check_vscode_tasks.py
    python tools/test_check_vscode_tasks.py

scanner 对 tracked source/config/script/canonical documentation 中的机器相关绝对路径一律失败；当前仓库不提供 absolute-path allowlist。回归测试覆盖 Windows、Linux、macOS、UNC、绝对 Markdown link、正常 repo-relative/tool-discovery 路径，以及带 marker 的绕过尝试。

Reference input corpus:

    python tools/generate_testdata.py
    python tools/verify_testdata.py
    python tools/test_testdata.py

`generate_testdata.py` creates the eight deterministic, synthetic `TESTDATA-001`
WAV fixtures and their manifest. Its `generate_corpus` API accepts the input
directory, manifest path and manifest root explicitly, and the CLI exposes the
same context through `--input-dir`, `--manifest` and `--manifest-root` so a
complete corpus can be generated outside the repository. `verify_testdata.py` checks provenance, the
repository MIT license, repository/no-LFS storage, WAV metadata and the
manifest's SHA-256 values. The manifest records each input's id, filename,
purpose, sourceType, source, author, redistribution terms, sample rate, bit depth,
channels, duration and repository/artifact/LFS storage policy. These hashes are
limited to the required reference inputs; rendered output remains ignored.

Offline render smoke:

    python tools/render_testdata.py --renderer build/windows-debug/frazil_render_artefacts/Debug/frazil_render.exe

The CTest preset runs this smoke after building `frazil_render`. The C++ harness
processes the selected input through the current `AudioEngine` without an audio
device, while the Python wrapper runs it twice, compares output bytes, and
writes an ignored WAV/metadata/hash manifest under `testdata/rendered/`.

建议工具：

- `pluginval`
- Steinberg VST3 Validator（如项目确实需要）
- AudioPluginHost 的构建说明或路径配置

大型第三方二进制文件默认不提交到仓库。当前已将 Ninja 1.13.2 和 pluginval 1.0.4 放置在 `tools/bin`，下载归档位于 `tools/downloads`；两者均已加入 `.gitignore`，来源和版本需在环境审计中记录。
