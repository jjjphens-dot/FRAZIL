# FRAZIL agent 协作与工程执行规范

本文件约束所有在本仓库内工作的自动化 agent 与开发者。产品与架构真相以 `docs/FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` 为总纲；阶段顺序、依赖和验收门槛以 `docs/CODING_PLAN.md` 为准。若两者冲突，先停止扩大实现范围，新增或更新 ADR，再同步相关文档。

## 1. 当前基线

- 当前阶段：M0 已部分完成，M1 尚未完成。
- 已有：JUCE 9.0.1、CMake/Ninja presets、VST3/Standalone、pass-through AudioEngine、APVTS 状态保存、9 个参数占位、smoke test。
- 已有：可移植 CI preset 与 Hosted CI 验证；
- 尚缺：ParameterMapper/Snapshot、真实 gain/routing/Water/Ice DSP、编辑历史、完整测试体系和正式 UI。
- 当前参数占位使用 `water.enable` / `ice.enable`，而目标合同为 `water.enabled` / `ice.enabled`；必须在首个公开兼容性基线前完成一次性迁移并加测试。
- GitHub 远端为 `https://github.com/jjjphens-dot/FRAZIL`。开始产品代码前必须确认工作目录是该仓库的 Git 工作树，且 `origin` 指向该地址。

不得把规划中的模块、历史验证结果或本地已有工具误写为“当前已实现”。完成状态必须由代码、测试或可复现验证记录支持。

## 2. 开工前最小检查

1. 阅读与任务直接相关的 `docs/` 文档和 ADR。
2. 执行 `git status --short`，保护用户已有改动；不得覆盖或清理无关变更。
3. 用 `rg` 定位现有实现和测试，避免创建平行架构。
4. 将任务映射到 `docs/CODING_PLAN.md` 的 milestone、issue ID、依赖和 exit gate。
5. 若任务触及参数 ID、范围、state schema、routing 语义或实时路径，先检查现有合同。实现已接受合同不自动创建 ADR；只有需要改变架构决策、依赖边界、公共模块职责、参数/状态兼容性、routing、realtime、latency、random-state 或正式性能合同时，才先更新/新增 ADR，再改实现。

### Required reading

每次生产代码任务开始前必须阅读：

- `docs/CODE_STANDARDS.md`；
- `docs/DOCUMENT_GOVERNANCE.md`；
- `docs/MODULE_INDEX.md`；
- 相关模块 README、`docs/CODING_PLAN.md`、`docs/PARAMETERS.md`、`docs/TESTING.md` 和 ADR。

所有生产代码修改必须遵守 `docs/CODE_STANDARDS.md`。违反该规范的代码不能因为“功能工作正常”而视为 Done。

### Mandatory development phases

代码任务必须按以下阶段执行并在输出中报告：

```text
Contract Review
  -> Implementation
  -> Functional Validation
  -> Code Quality Review
  -> Comment & Documentation Pass
  -> Final Validation
```

功能完成后不得跳过独立的 Code Quality Review 或 Comment & Documentation Pass。

### Forbidden shortcuts

Agent 不得：

- 功能通过后直接结束而不更新注释、模块 README 或 `MODULE_INDEX.md`；
- 修改 LOCKED contract 以迁就实现；
- 使用 mutable global/static state 省事；
- 用巨大 class 聚合多个变化原因；
- 创建隐藏跨层 dependency、service locator 或 global singleton；
- 用 macro 代替正常 C++ abstraction；
- 复制代码而不检查共享 primitive 是否合理；
- 为抽象而抽象，或顺手进行无关重构。

## 3. 目录与依赖方向

- `src/plugin/`：JUCE `AudioProcessor`、VST3/Standalone 适配、总线、`ParameterLayout.*` 静态参数声明、Host state adapter。
- `src/app/`：`AudioEngine`、`ParameterMapper`、`ParameterSnapshot`、`StateModel`、`EditHistoryManager`。
- `src/dsp/`：Water、Ice、Routing、StageMixer、DryWetMixer 与确有复用价值的 primitive。
- `src/ui/`：编辑器和控件；只表达产品参数和发起 UI transaction，不执行 DSP。
- `experiments/`：允许失败的声音实验；candidate、A/B、prototype 和 listening exploration 不自动要求 ADR，不能被生产 target 直接依赖；采纳为 production decision 或改变既有合同/边界时才按 ADR trigger 处理。
- `tests/`：unit、DSP property、render、integration、host validation。
- `testdata/input/` 与 `testdata/reference/`：小型、可授权、可复现的测试素材；`testdata/rendered/` 永不提交。
- `external/`：固定版本的第三方依赖。除已记录的构建兼容补丁外，不修改 vendor 代码；升级或补丁必须有 ADR/说明。

允许的主依赖方向：

```text
ui -> narrow plugin parameter interface
ui -> narrow app edit/history command interface
plugin -> app -> dsp
tests -> 被测模块
```

`StateModel` 只依赖 application value types；Plugin Host State Adapter 调用 StateModel。禁止 `app -> plugin`、`dsp -> app/plugin/ui`、`ui -> AudioEngine/DSP object`、`Water/Ice -> RoutingMode`、DSP 直接读取 APVTS。

## 4. 实时音频线程硬规则

进入 `processBlock()` / `process()` 的代码必须满足：

- 不做文件、网络或 console I/O；
- 不使用 `new/delete`、不可控容器增长或运行时首次初始化；
- 不等待线程，不获取阻塞锁，不访问 UI 或 UndoManager；
- 不以异常作为正常控制流；
- 所需 buffer、delay line、FFT 状态和随机源在 `prepare()` 预分配；
- 每个 block 开始只获取一次一致的 `ParameterSnapshot`；
- 连续参数有明确 smoothing；离散 routing/enable 采用 click-free transition；
- 任意有限输入、合法参数和支持的 sample rate/block size 下不得产生 NaN/Inf；
- 随机 DSP 可注入固定 seed，以支持确定性渲染测试。

任何 realtime-safety 断言都必须有代码路径审查及相应测试/测量依据。

## 5. 参数与状态合同

- 正式 Host 参数在初始化时静态注册，定义集中在 `src/plugin/ParameterLayout.*`；不得按 routing 动态增删。
- ID、范围、默认值、单位、step、skew、automation 和 smoothing 要与 `docs/PARAMETERS.md` 一致。
- Parallel 只用 `parallel.balance` 控制两支路相对比例；Serial 只用 `water.amount` / `ice.amount` 控制各 stage mix。
- 暂时无效的参数保留值；切换模式不得重置。
- `input.gain` 位于完整处理链之前，同时影响 dry reference 与 wet input；`output.gain` 位于 Global Mix 之后。
- UI 隐藏参数不改变 Host 可见参数集合。
- Undo/Redo 不是 Host 参数，只记录 FRAZIL UI 完成的 gesture/command；不得记录 automation playback 或 Host state restore。
- Host state restore 后清空插件内部 history。
- 公开发布后修改 ID、范围或 state 结构必须提供迁移策略、兼容性测试和 ADR。

## 6. 实验进入生产的门槛

`experiments/` 中的算法只有同时满足以下条件才能移入 `src/dsp/`：

1. 明确假设、参数空间和固定输入素材；
2. 至少提供 baseline 与 candidate 的 loudness-matched A/B；
3. 固定 seed 时可重复；
4. 对 drums、vocal、piano/guitar、pad、bass、noise、impulse 中相关素材完成听测记录；
5. 有 CPU/峰值 callback 时间初测，没有明显实时安全风险；
6. 通过 finite-output、参数极值和 reset/prepare 测试；
7. 参数能归纳为少量产品 macro，而不是直接暴露算法内部旋钮。

未通过时保留为 experiment，不为赶 milestone 降低生产门槛。

## 7. 构建与验证

构建产物必须留在 F: 工作区：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

其他预设：`windows-release`、`windows-asan`。本机完整 MSVC 环境命令见 `docs/ENVIRONMENT.md`。

按变更范围执行最小验证：

- 文档/模板：检查链接、路径、命令和状态表与仓库一致。
- app/dsp：Debug build + 相关 unit/DSP tests。
- 参数/state/routing：Debug + Release、state round-trip、automation/mode retention tests。
- 实时 DSP：Debug + Release + ASAN、property tests、render regression、CPU 记录。
- plugin/platform/UI：pluginval；发布相关变更再跑目标 DAW matrix。

不得只用“编译通过”声明 DSP、Host automation 或听感完成。

## 8. GitHub 工作流

- 使用短生命周期分支：`feat/`、`fix/`、`test/`、`docs/`、`build/`、`experiment/`。
- 每个 PR 只解决一个可验收问题；大型工作先拆 issue，再按依赖顺序合并。
- commit 建议使用 Conventional Commits：`feat(dsp): ...`、`fix(host): ...`、`docs(plan): ...`。
- `main` 始终可配置、可构建、可测试；禁止直接提交未验证的生产 DSP。
- PR 必须填写架构、参数/automation、实时安全、测试和音频评估影响。
- 参数合同、routing、state、核心 DSP 或发布流程变更至少一名另一位开发者审批，相关讨论全部 resolve 后合并。
- 禁止提交 `build/`、`.venv/`、工具二进制、下载归档、生成 render、DAW cache 或个人路径配置。
- 未经明确请求，agent 不执行 push、merge、release、branch protection 或删除远端内容。

推荐保护 `main`：PR 必需、至少 1 approval、必需 CI、解决全部讨论、禁止 force push 和 branch deletion。具体设置见 `docs/GITHUB_WORKFLOW.md`。

## 9. Definition of Ready / Done

Issue 进入 Ready 前必须有：用户/声音问题、范围与非目标、acceptance criteria、测试方式、听测需求、参数/automation/state 影响、依赖和 owner。

完成至少意味着：

- 实现和相关文档同步；
- 目标 presets 构建通过，相关自动测试通过；
- Code Quality Review 已检查 cohesion、coupling、naming、scope、ownership、globals、magic numbers、macros、includes、realtime safety 和 dead code；
- Comment & Documentation Pass 已检查 public API、关键算法、单位/range、ownership、realtime contract、module README 和 `MODULE_INDEX.md`；
- 每个 architecture、parameter/state、realtime、performance 和 documentation 影响项都明确填写；无影响时写 `N/A`；
- realtime-safety、smoothing、bypass、state、mode retention 按影响范围验证；
- 涉及声音的变更有可重复 render 与听测结论；
- 涉及 Host 的变更有 pluginval/DAW 证据；
- 没有把 P1/P2 功能暗中带入 P0；
- PR 获得要求的 review 并合入 `main`。

## 10. Agent 输出要求

- 先报告实际基线、风险和假设，再说明改动结果。
- 新文件与新抽象必须说明由哪个真实需求驱动。
- 遇到文档与代码冲突时显式列出，不擅自把任一方当成事实。
- 完成后给出实际执行过的命令和结果；未执行的验证要明确标注。
- 需要用户决定声音审美、许可、产品范围或不可逆外部操作时，停止在安全边界并提出一个具体问题。

## 11. 本文件修改策略

`AGENTS.md` 是 Agent 直接入口，属于 CONTROLLED 文档。修改必须与工程治理或架构规则 issue 相关，并同步 `docs/DOCUMENT_GOVERNANCE.md`、`docs/CODE_STANDARDS.md` 或相关计划文档；不得通过修改本文件绕过 LOCKED contract。
