# FRAZIL 当前实现与差距

> 快照日期：2026-09-06  
> 依据：本地源码/构建目录审计、当前 Debug/portable build/CTest、GitHub 仓库页面。  
> 原则：这里只记录已验证事实；目标和待办分别由架构总纲与 Coding Plan 管理。

## 1. 结论

项目已有可构建的 JUCE M0 骨架，但尚未进入产品 DSP 实现。最优先的工作不是 Water/Ice 算法，而是先把本地目录纳入 Git、建立可移植 CI，并锁定参数/状态/Application-DSP 接口合同。

当前阻塞性差距：

1. 本地 Git 工作树和 `origin` 已初始化，首个 commit `96b3659` 已推送到远端 `main`；Hosted CI 尚未在本次审计中确认；
2. 本机 presets 固定 F: 盘编译器/SDK；portable `ci-windows-debug` preset 已添加并在本机 MSVC 环境验证，Hosted CI 尚待首推后验证；
3. 参数 ID 存在 `water.enable`/`ice.enable` 与架构目标 `water.enabled`/`ice.enabled` 的冲突；
4. APVTS 参数虽然已注册，但尚未通过 Snapshot/Mapper 进入 AudioEngine；
5. 只有 pass-through、CTest smoke 和 M0 AudioEngine unit target，尚未覆盖插件参数行为；
6. Water、Ice、Routing、gain processing、smoothing、Undo/Redo 和正式 UI 均未实现。
7. MIT `LICENSE` 已加入；第三方 notice 策略和 GitHub 仓库 metadata 仍待收口。

## 2. 已有资产

| 范围 | 当前事实 | 成熟度 |
|---|---|---|
| Build | CMake 3.22+、C++20、Ninja presets | 本机可用；不可移植 |
| Formats | JUCE target 声明 VST3 + Standalone | 已接入 |
| Dependency | `external/JUCE` 为 9.0.1，本机文档记录两个兼容补丁 | 需确定仓库获取/补丁策略 |
| Plugin shell | mono/stereo bus check、editor、state XML round-trip | 骨架 |
| Parameters | 9 个 APVTS 占位参数静态注册 | 合同未锁定 |
| App | `AudioEngine::prepare/reset/process` | pass-through |
| UI | 640x360 M0 占位界面 | 非产品 UI |
| Tests | `frazil_smoke` + `frazil_tests` CTest | M0 wiring/lifecycle 覆盖；DSP/Host 测试未开始 |
| Local validation | Debug 与 portable preset 已 configure/build；两个 CTest case 均 PASS | 已验证 |
| pluginval | 本轮 Debug VST3 strictness 5 SUCCESS；Steinberg validator 因未配置而跳过 | 已验证（不等于独立 VST3 validator） |
| Remote | `jjjphens-dot/FRAZIL` public repository；`origin` 已绑定，首个 `main` commit `96b3659` 已推送 | 已验证；Hosted CI 待确认 |

## 3. 当前源码映射

```text
PluginProcessor
  ├─ owns APVTS
  ├─ declares parameters inline
  ├─ saves/restores APVTS XML
  └─ calls AudioEngine::process(buffer)

AudioEngine
  └─ pass-through placeholder

PluginEditor
  └─ static M0 label
```

目标映射应在 M1 后变为：

```text
PluginProcessor
  ├─ ParameterLayout
  ├─ Host State Adapter
  └─ ParameterSnapshot source
          ↓
    ParameterMapper
          ↓
      AudioEngine
  ├─ Input Gain
  ├─ RoutingEngine -> Water / Ice / StageMixer
  ├─ Global Mix
  └─ Output Gain
```

## 4. 参数差异审计

| 语义 | 当前代码 | 目标合同 | 动作 |
|---|---|---|---|
| Water enable | `water.enable` | `water.enabled` | 首个公开版本前一次性更名并测试枚举/state |
| Ice enable | `ice.enable` | `ice.enabled` | 同上 |
| Routing | `routing.mode` | `routing.mode` | 保留 |
| Parallel balance | `parallel.balance` | `parallel.balance` | 保留 |
| Water stage amount | `water.amount` | `water.amount` | 保留 |
| Ice stage amount | `ice.amount` | `ice.amount` | 保留 |
| Input trim | `input.gain` | `input.gain` | 保留，补 DSP 与 smoothing |
| Global dry/wet | `global.mix` | `global.mix` | 保留，补 DSP 与 smoothing |
| Output trim | `output.gain` | `output.gain` | 保留，补 DSP 与 smoothing |

在 M1 参数合同 PR 合并前，不得创建公开 preset/session 兼容性承诺。若已有外部用户使用过当前占位构建，应先确认是否需要兼容别名/迁移。

## 5. 现状对应 milestone

- M0 Repository & Governance：**进行中**。本地 Git、portable preset、bootstrap、CI 文件、基础测试 target、MIT 许可证和首次 push 已完成；Hosted CI 与 GitHub metadata 尚未收口。
- M1 Audio Skeleton & Parameter Contract：**进行中早期**。有 AudioEngine、静态参数和 state 骨架，缺 Snapshot/Mapper、真实 gain、自动化验收与离线渲染。
- M2 Water：**未开始**。
- M3 Ice：**未开始**。
- M4 Routing：**未开始**。
- M5 UI & Edit History：**未开始**。
- M6 Beta Hardening：**未开始**。
- M7 v1.0 Release：**未开始**。

## 6. 不应从现状推断的结论

- 参数“能被 APVTS 注册”不等于 automation click-free 或 DSP 已消费参数；
- state XML 能 round-trip 不等于跨版本兼容策略已建立；
- pluginval 历史通过不等于真实 DAW matrix 已通过；
- pass-through 能构建不等于 routing/gain/dry-wet 的数学和增益结构正确；
- 目录 README 存在不等于对应模块已经实现；
- 本地 `external/JUCE` 存在不等于 fresh clone 可复现。

## 7. 下一步唯一推荐入口

按 `docs/CODING_PLAN.md` 收口 M0：确认 Hosted CI 与 GitHub metadata，再进入 M1 参数合同和 Application/DSP 接口。不要先写 Water/Ice 生产算法。
