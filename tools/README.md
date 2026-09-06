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

建议工具：

- `pluginval`
- Steinberg VST3 Validator（如项目确实需要）
- AudioPluginHost 的构建说明或路径配置

大型第三方二进制文件默认不提交到仓库。当前已将 Ninja 1.13.2 和 pluginval 1.0.4 放置在 `tools/bin`，下载归档位于 `tools/downloads`；两者均已加入 `.gitignore`，来源和版本需在环境审计中记录。
