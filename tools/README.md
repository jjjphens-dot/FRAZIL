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
jobs and defaults to six, refuses insufficient available physical memory, and writes full compiler
output to ignored build/safe-build/<preset>.log. It prints only a bounded tail
when the build fails. The shared configure preset also sets CMAKE_BUILD_PARALLEL_LEVEL=6 for JUCE nested builds. Do not replace it with a bare --parallel invocation, and do not run multiple heavy configure/build/test pipelines concurrently.
Portability checks：

    python tools/check_portability.py
    python tools/test_check_portability.py

scanner 对 tracked source/config/script/canonical documentation 中的机器相关绝对路径一律失败；当前仓库不提供 absolute-path allowlist。回归测试覆盖 Windows、Linux、macOS、UNC、绝对 Markdown link、正常 repo-relative/tool-discovery 路径，以及带 marker 的绕过尝试。

建议工具：

- `pluginval`
- Steinberg VST3 Validator（如项目确实需要）
- AudioPluginHost 的构建说明或路径配置

大型第三方二进制文件默认不提交到仓库。当前已将 Ninja 1.13.2 和 pluginval 1.0.4 放置在 `tools/bin`，下载归档位于 `tools/downloads`；两者均已加入 `.gitignore`，来源和版本需在环境审计中记录。
