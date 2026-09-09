# FRAZIL agent 协作与工程执行规范

本文件约束所有在本仓库内工作的自动化 agent 与开发者。产品与架构真相以 `docs/FRAZIL_PROJECT_ARCHITECTURE_v0.3.md` 为总纲；阶段顺序、依赖和验收门槛以 `docs/CODING_PLAN.md` 为准。若两者冲突，先停止扩大实现范围，新增或更新 ADR，再同步相关文档。

## 0. 哈希计算默认禁用

- Agent 默认不得对源码、构建产物、依赖、render、日志或工作树批量计算、比较或记录 hash/checksum。
- 只有在用户明确要求、现有验收/安全完整性合同明确要求，或发布流程的既有步骤不可避免地需要时，才允许执行哈希操作，并应限定到必要的目标。
- 不得为了生成报告、证明普通构建成功、比较普通工作树差异或“顺手留证”而启动哈希工具；已有 commit SHA 作为 Git 身份引用时不等于需要重新计算内容 hash。
- 没有上述必要性时，直接跳过哈希步骤，不新增 hash 字段、manifest 或相关文档证据。
## 0.1 本地构建资源安全

- 禁止 agent 在本机执行不带明确 job 数的 cmake --build ... --parallel。
- 本地 Windows 构建必须通过 python tools/build_safe.py --preset <name>，默认 6 个 job，硬上限 8 个 job；wrapper 会在构建前检查可用物理内存，并把完整输出写入 ignored 的 build 日志。
- 安全检查拒绝时不得通过删除检查、提高并发上限或改用裸 CMake/Ninja 命令绕过；应停止并报告资源状态。
- Hosted CI 也使用同一受控 wrapper；构建失败时只回显有限日志尾部，避免把海量 compiler include 输出灌入终端。
- 共享的 portable-windows-base configure preset 注入 CMAKE_BUILD_PARALLEL_LEVEL=6，用于约束 configure 阶段的 JUCE nested build；不得通过修改环境变量绕过安全检查。
- Agent 不得在本机并发运行多个 configure/build/test pipeline；Debug、Release、ASAN 和其他重型 preset 必须串行执行。
## 1. 当前基线

- 当前阶段：M1-C 进行中；M1-A/M1-B Parameter/Engine foundation 与 M1-C STATE-001 versioned StateModel/Host State Adapter foundation 已合入 `main`，M1 尚未完成。
- 已有：JUCE 9.0.1、CMake/Ninja presets、VST3/Standalone、pass-through wet path、APVTS 状态保存、集中式 ParameterLayout、ParameterSnapshot/Mapper、基础 gain/mix/smoothing、9 个 Host 参数和 smoke test。
- 已有：可移植 CI preset 与 Hosted CI 验证；M1-A/M1-B 的首块 priming、retarget 和 runtime buffer invariant regression 已建立；STATE-001 的 schema migration/fallback、XML restore、STATE-002 mode-value-retention 和 AUTO-001 PluginProcessor integration evidence 已建立。
- 已有：`TESTDATA-001` 可复现 engineering corpus 与 `RENDER-001` pass-through offline smoke 已进入 `main`；这些能力只按 regression/finding 维护，不另建平行实现。
- 尚缺：`TEST-002` processor property、`PERF-BASE-001`、`ARCH-LAT-001` 完整证据、真实 Host/DAW validation、编辑历史、Water/Ice/Routing DSP 和正式 UI；公开参数 freeze 与完整兼容性 evidence 仍待完成。
- 当前参数合同已将历史 `water.enable` / `ice.enable` 迁移为 `water.enabled` / `ice.enabled`；现有 state migration evidence 已建立，公开兼容性基线前仍需完整 compatibility evidence 与正式参数 freeze。
- GitHub 远端为 `https://github.com/jjjphens-dot/FRAZIL`。开始产品代码前必须确认工作目录是该仓库的 Git 工作树，且 `origin` 指向该地址。

不得把规划中的模块、历史验证结果或本地已有工具误写为“当前已实现”。完成状态必须由代码、测试或可复现验证记录支持。

## 2. Agent execution model

### Normal bounded task

实现前只做与风险相称的最小检查：

1. 确认 scope、non-goals 和主要 ownership/domain；
2. 执行 `git status --short`，保护用户已有和无关改动；
3. 阅读 task-relevant plan/work item、直接相关 module/contract，以及现有 implementation/tests；
4. 用 `rg` 复用现有实现和测试，不创建平行 production path；
5. 只修改完成任务所需的最少文件，并运行与变更成比例的验证。

read、review、analysis、本地 edit/test/commit 和 diff inspection 不要求 GitHub identity preflight。PR creator
和 authenticated account 是 existing-PR push 时的运行时检查，不是开始实现的前置条件。

### Ownership-sensitive or cross-module task

仅当任务跨 primary ownership、修改另一角色负责的 production DSP/app/plugin、跨多个模块、存在明显
scope-creep 风险、改变 LOCKED/CONTROLLED contract，或需要实质 DRI handoff 时，才按需补充确认
Implementation/Acceptance DRI、write/path scope、handoff condition、Joint Gate 和相关 contract/ADR。

Primary ownership 防止平行或实质性越界 production implementation，不是 filesystem ACL。当前 scope 内的
typo、文档同步、小型测试修正或 trivial integration fix 可以由另一角色完成，不要求 DRI Transfer；只有
substantial implementation responsibility 真正换人时才记录 transfer。详细规则见
`docs/COLLABORATION_ROLES.md`。

### Task-relevant reading

每次 production task 只读取直接相关的 canonical source：

- State：相关 parameter/state contract、ADR、module docs 和 tests；
- DSP：realtime contract、对应 algorithm ADR/guide 和 testing contract；
- Build/CI：environment、build 和 workflow 文档；
- Host acceptance：HOST matrix 与 testing/acceptance protocol。

改变 LOCKED/CONTROLLED contract 时必须完整阅读对应 canonical contract/ADR 并执行 Documentation
Synchronization Gate。所有 production code 仍受 `docs/CODE_STANDARDS.md` 约束；
`docs/CORE_IMPLEMENTATION_GUIDE.md` 只提供候选方法和实现解释，不能覆盖 Architecture、Coding Plan、
Parameters 或 Accepted ADR。

### Joint Gate trigger

Joint Gate 只在改变 Parameter ID/order/range/default、state compatibility、Water/Ice production algorithm、
routing/StageMixer/crossfade 语义、latency/tail、random persistence、formal performance budget 或 Beta/Release
go/no-go 时阻断。普通 bug fix、测试覆盖、render harness maintenance、performance measurement 实现、DAW
evidence、docs update 和不改变合同的 bounded refactor 不触发 Joint Gate。

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

### Documentation Impact Check / Synchronization Gate

任务先按实际 scope 执行 [Documentation Impact Check](docs/DOCUMENT_GOVERNANCE.md#5-documentation-synchronization-gate)。
普通 bounded task 只检查直接相关的 contract/module/evidence；只有 architecture、公共接口、模块职责、
参数/state/routing、realtime/latency/random、Host/UI documented behavior、build/dependency/CI contract、正式
performance contract、milestone/status/support 或 release compatibility claim 等 Full Gate trigger 被命中时，
才执行完整 Documentation Synchronization Gate。需要更新的文档必须与实现进入同一个 PR；只对直接相关但
无需更新的文档记录理由，最终反馈按变更风险报告 Documentation Review。

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

构建产物必须保存在 repository-local 或 developer-configured build directory 中，并且不得提交到 Git。
例如 <repo-root>/build/，或由 CMakeUserPresets.json 指定的本地构建目录。

```powershell
cmake --preset windows-debug
python tools/build_safe.py --preset windows-debug
ctest --preset windows-debug
```

### Repository Portability Rules

- 禁止在 tracked source/config/script/canonical documentation 中提交开发者个人绝对路径。
- 禁止依赖固定盘符、开发者用户名或 Visual Studio、Windows SDK、Python、DAW 的个人安装目录。
- 机器相关路径必须使用 repo-relative path、environment variable、tool discovery、CMakeUserPresets.json 或 ignored local configuration。
- Tracked reference-machine evidence 可以记录 OS、工具版本、SDK/toolchain 版本和泛化后的路径占位符，但不得保存开发者原始绝对路径。
- 确实需要保存的本机原始路径只能存在于 ignored/untracked local evidence 中，不得提交到 Git。

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
- PR 必须完成 Documentation Impact Review，并在模板中记录受影响文档和一致性检查结果。
- 参数合同、routing、state、核心 DSP 或发布流程变更至少一名另一位开发者审批，相关讨论全部 resolve 后合并。
- Implementation/Acceptance DRI、PR creator、commit author/committer 和 reviewer 是不同事实，不要求相同或预绑定。Reviewer 仍按 independence、scope、evidence 和 decision 负责；fallback comment/manual evidence 不得冒充 formal `APPROVE`。
- 仅在准备向已有 PR branch 执行本 context 的首次 push 时，检查 open PR creator 与当前 authenticated account；相同 repository + branch + PR + auth session 的成功结果可以复用。branch/repository/PR/auth 变化、权限/账号异常或用户说明账号变化时重新检查。没有 open PR 时按普通 push/PR 创建流程处理，PR creator 无需等于 Implementation DRI。
- 已有 PR 的严格规则保持不变：`current authenticated push account == existing PR creator`。不一致时停止 push 并检查登录；不得跨账号 push、伪造 author、rewrite history、force push 或建立无意义的新 PR。完整流程见 `docs/GITHUB_WORKFLOW.md`。
- 禁止提交 `build/`、`.venv/`、工具二进制、下载归档、生成 render、DAW cache 或个人路径配置。
- 未经明确请求，agent 不执行 push、merge、release、branch protection 或删除远端内容。

推荐保护 `main`：PR 必需、至少 1 approval、必需 CI、解决全部讨论、禁止 force push 和 branch deletion。具体设置见 `docs/GITHUB_WORKFLOW.md`。

## 9. Definition of Ready / Done

Issue 进入 Ready 前必须有 Stable ID/Milestone、用户/声音问题、scope/non-goals、Implementation DRI、
Acceptance DRI 和 acceptance criteria。只有 cross-module、production DSP、contract/ownership-sensitive、
Host handoff 或 milestone-gate 工作才必须补 write/path scope、inputs/outputs、handoff 和 Joint Gate 字段。

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
- 最终反馈必须明确列出文档变更、已审查但无需更新的文档、跨文档一致性结果和未执行的验证。

## 11. 本文件修改策略

`AGENTS.md` 是 Agent 直接入口，属于 CONTROLLED 文档。修改必须与工程治理或架构规则 issue 相关，并同步 `docs/DOCUMENT_GOVERNANCE.md`、`docs/CODE_STANDARDS.md` 或相关计划文档；不得通过修改本文件绕过 LOCKED contract。
