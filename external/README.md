# external/

项目级第三方依赖目录。

预期：

```text
external/JUCE/
```

JUCE 9.0.1 不提交到 FRAZIL 主仓库。运行 `tools/bootstrap_dependencies.ps1` 会获取固定
commit `e18f7f506c0b96f2c738a0bcd7fe6467a5005ad8`，并应用
`tools/patches/JUCE-9.0.1-msvc-toolchain.patch` 中可审计的 Windows 嵌套 CMake
工具链兼容补丁。它不安装为系统全局依赖；策略和更新要求见
`docs/adr/0004-juce-and-ci-dependency-strategy.md`。
