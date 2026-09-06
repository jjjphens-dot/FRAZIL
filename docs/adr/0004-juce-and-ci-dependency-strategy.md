# ADR-0004: JUCE 与 CI 依赖策略

- Status: Accepted
- Date: 2026-09-06

## Modification Policy

本 ADR 的 Accepted Decision 是 LOCKED 依赖/CI 策略。升级 JUCE、补丁或 portable CI 合同必须新增/更新 ADR、记录 revision/兼容性证据，并同步环境与工作流文档；不删除历史事实。

## Context

当前项目从 `external/JUCE` 构建 JUCE 9.0.1，本机副本含两个针对 F: 盘 MSVC/SDK 的兼容补丁；本地 CMake presets 也固定绝对路径。GitHub 远端为空，fresh clone 和 hosted CI 尚不可复现。

## Decision

采用固定 commit 下载 + 仓库内 patch 文件的策略：

- `external/JUCE` 由 `tools/bootstrap_dependencies.ps1` 获取，不进入 FRAZIL 主仓库；
- commit 固定为 `e18f7f506c0b96f2c738a0bcd7fe6467a5005ad8`（JUCE 9.0.1 tag）；
- F: 盘 MSVC/Windows SDK 所需的两个嵌套 CMake 兼容修改以
  `tools/patches/JUCE-9.0.1-msvc-toolchain.patch` 保存并由脚本幂等应用；
- CI 使用无绝对路径的 portable preset，并在 runner 环境中提供 MSVC、CMake 和 Ninja；
- 不 vendoring JUCE，也不允许 CI 使用未固定的 JUCE `master`。

## Consequences

本地 preset 与 portable CI preset 分离，但共享编译特性和 target contract。更新 JUCE
版本必须同时更新 commit、patch、bootstrap 验证和本 ADR，并重新跑本地与 CI 构建。

## Verification

全新目录仅凭仓库内容和 `tools/bootstrap_dependencies.ps1` 能获取固定依赖，Windows
runner 完成 configure/build/CTest；本地 F: preset 仍通过。
