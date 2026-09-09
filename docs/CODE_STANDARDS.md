# FRAZIL Code Quality Standards

## 1. Scope and rule

本规范适用于 `src/`、`tests/`、生产工具和所有后续 agent。它把“功能工作正常”与“代码可维护、可审查、可实时运行”分开：代码只有在功能验证、Code Quality Review 和 Comment & Documentation Pass 都完成后才能标记 Done。

基本目标是高内聚、低耦合、明确所有权、可验证的实时安全和小而稳定的公共接口。任何例外都必须在 issue/PR 中说明影响、理由、替代方案和退出条件。修改前先检查既有合同：实现已接受的架构、参数、状态或实时合同不自动要求 ADR；只有改变决策、边界、兼容性或正式预算时才补 ADR。

Primary ownership 防止平行或实质性越界 production implementation，不是 filesystem ACL。普通 bounded
task 按明确 scope 修改最少文件；cross-module、production DSP 或 contract/ownership-sensitive 工作才要求
write ownership 和 Allowed/Forbidden paths。Acceptance DRI 默认 review、复现和创建 finding，但 scope 内
的 typo、小型 test/docs 或 trivial integration fix 不需要 DRI Transfer；只有 substantial implementation
responsibility 换人时才按 `COLLABORATION_ROLES.md` 记录 transfer。不得把 code review 变成未声明的
production feature 接管。

## 2. 模块边界与依赖

允许的主要方向：

```text
ui -> narrow plugin parameter interface
ui -> narrow app edit/history command interface
plugin -> app -> dsp
tests -> 被测模块
```

- `WaterProcessor` 只负责 Water transform；`IceProcessor` 只负责 Ice transform；`RoutingEngine` 只负责拓扑和切换；`StageMixer`/`DryWetMixer` 只负责 mix law；`ParameterMapper` 只负责产品参数到 engine 参数的映射；`PluginProcessor` 只负责 Host/JUCE 适配和生命周期。
- 禁止 `dsp -> app/plugin/ui`、`ui -> DSP object`、`DSP -> APVTS`、`Water/Ice -> RoutingMode`，也禁止用隐藏 service locator、global singleton 或前置声明绕过真实依赖。
- UI 只可依赖 narrow plugin parameter interface 和 narrow app edit/history command interface；不得依赖 `AudioEngine`、Water/Ice、RoutingEngine、DSP primitives、DSP buffer/state 或具体 PluginProcessor。
- `StateModel` 只依赖 application value types；Plugin Host State Adapter 调用 StateModel，禁止 `app -> plugin` 反向依赖。
- 新依赖必须有明确 owner 和测试入口；不得为了复用把多个变化原因合进一个巨大 class，也不得为抽象而抽象。
- Public API 保持小而稳定：优先值类型、`const` 引用和明确的 enum/value object；不得暴露可随意修改的内部容器或 APVTS 作为跨层通用接口。

## 3. 所有权、生命周期与状态

- 优先 RAII 和值语义；动态所有权使用 `std::unique_ptr`/`std::shared_ptr`，并说明为什么需要共享所有权。
- 非 owning 指针/引用必须在接口或注释中说明 owner、有效期和线程假设；禁止悬空引用、隐式借用和依赖析构顺序的全局对象。
- 状态只保留真正跨越 sample、block、callback 或对象生命周期的内容；临时计算值留在最小作用域。
- 不使用 mutable global/static runtime state。`constexpr` 常量允许；function-local static 不得隐藏音频初始化、缓存、随机源或线程不安全的一次性工作。
- 分配、buffer、delay line、FFT、随机源等在 `prepare()` 或明确的非实时生命周期阶段准备好；析构和 reset 的所有权行为要可读、可测试。

## 4. 命名、格式和 include

以 `.clang-format` 为机械基线：C++20、4 空格、列宽 100、include regroup/sort、现有 `member_` 后缀保持一致。语义规则如下：

- 类型、class、struct、enum class 使用 `PascalCase`；函数、局部变量和参数使用 `lowerCamelCase`；私有成员使用 `member_`；常量使用有语义的 `k...` 名称。
- 参数 ID 使用点分隔的小写合同名，例如 `water.enabled`、`global.mix`。
- 用 `enum class` 表达有限集合；不要用裸整数表达 routing、来源或状态。
- Header 自洽并只 include 直接使用的声明；避免循环 include。纯 app/DSP 不得使用聚合 `JuceHeader.h`，但在确有直接类型需求时可以使用最小 JUCE audio module（例如 `juce_audio_basics`、`juce_dsp`）；不得依赖 APVTS、PluginProcessor、PluginEditor、Host-specific object 或 UI module。
- `clang-format` 不能替代设计审查；格式化后的大面积无关 diff 应避免混入功能 PR。

## 5. 常量、宏和复杂度

- 消除 magic number：使用带单位/语义的 `constexpr`，例如 `kReferenceSampleRateHz`、`kMaximumHistoryEntries`，并在必要时说明来源或 ADR。
- 宏只用于平台、JUCE 或构建系统确实要求的边界；项目自行新增的宏必须在定义附近或关联文档说明 purpose、为何必须用宏、scope、platform/build impact，以及临时宏的 removal condition。JUCE/framework 必需宏不必重复解释框架本身，但其行为影响 ownership、ABI 或 build 时必须注明。不得用宏代替普通 C++ abstraction、类型安全或配置数据。
- 函数应有单一可读的变化原因；当分支、嵌套或状态组合难以测试时拆分 helper/class，而不是复制代码或引入过度泛化的框架。
- 删除死代码和已失效注释；deferred work 通过 issue/计划 ID 记录，不靠无主 TODO 隐藏。

## 6. 实时音频路径

进入 `processBlock()`/`process()` 的路径不得：

- 做文件、网络、console/log I/O，等待线程，获取阻塞锁，访问 UI、UndoManager 或 Host state；
- 使用 `new/delete`、不可控容器增长、异常控制流或隐藏的运行时首次初始化；
- 直接读取 APVTS 多次。每个 block 开始只取得一次一致的 `ParameterSnapshot`，再交给值类型的 engine 参数；
- 让 enable/routing 立即跳变，或让连续参数没有明确 smoothing。

`prepare()` 负责预分配与生命周期初始化；离散 routing/enable 使用可验证的 click-free transition；所有支持的 sample rate/block size、有限输入和合法参数都必须保持 finite output。实时规则不是“过早优化”：每条规则都应能在代码路径审查、property test、render 或测量中追溯。

## 7. 注释、接口和文档

注释优先说明代码本身看不出的 why、contract、invariant、ownership、单位/range、实时限制和算法假设；不要逐行复述语法。公共 class/function、关键算法和跨线程/跨模块接口必须有准确说明，且变更时同步更新。

### Mandatory comments for non-obvious persistent state

以下持久状态若存在，必须有清晰注释：filter state、resonator state、delay read/write index、random generator state、seed、smoother state、routing transition state、crossfade state、scratch buffer、threshold、语义不明显的 counter、具有特殊行为的 sample/channel index、stateful DSP accumulator，以及单位不明显的 parameter-derived cached value。

按实际需要说明 purpose、unit、range、lifetime、thread ownership、algorithmic role 和 reset behavior。例如：

```cpp
// Circular delay-line write position, in samples.
// Owned and updated only by the audio thread; reset() returns it to zero.
std::size_t delayWriteIndex_ {};
```

明显的局部变量不需要机械注释；优先使用好命名，并只为非显然的 contract/state/why 添加注释。

每次代码修改完成后必须执行：

1. 功能与相关测试验证；
2. cohesion/coupling、命名、作用域、ownership、global/static、magic number、macro、include、dead code 和 realtime safety 审查；
3. Comment & Documentation Pass：公共接口、关键算法注释、模块 README、`docs/MODULE_INDEX.md`、ADR/计划/测试合同按影响范围同步；
4. 最终验证并在 PR 写出实际命令和结果；无影响项写 `N/A`。

实现前还必须完成 [Documentation Synchronization Gate](DOCUMENT_GOVERNANCE.md#5-documentation-synchronization-gate)：识别受影响合同和证据文档，必要更新与实现放在同一个 PR，并记录 reviewed-but-unchanged 文档及一致性检查结果。

对涉及 State、Water、Ice、Routing、automation 或 edit/history behavior 的实现工作，在阅读 Architecture、Coding Plan、Parameters contract 和相关 Accepted ADR 后，还应 review `docs/CORE_IMPLEMENTATION_GUIDE.md`。该文档是 Level 3 maintained implementation guide，只可解释实现方法和 experiment candidates，不得覆盖 Level 1/2 contracts 或 Accepted ADR decisions。

## 8. 例外与修改规则

违反本规范的例外必须可定位到 issue/PR；`experiments/` 中的 candidate、A/B 和 prototype 可以先不建 ADR。若候选被采纳为 production architecture/core DSP decision，或改变既有架构决策、依赖方向、公共职责或其他 ADR trigger 合同，才必须有 ADR 和测试。实现既有 accepted contract、补已有测试或不改变公共行为/依赖边界的 refactor 不自动要求 ADR。禁止为了赶 milestone 默默降低实时、所有权、模块边界或文档要求。

## Modification Policy

本文件属于 CONTROLLED 治理文档。修改需要治理 issue/review；若修改了依赖方向、实时规则、命名合同或 Done 门槛，必须同步 `AGENTS.md`、`docs/DOCUMENT_GOVERNANCE.md`、`docs/CODING_PLAN.md` 和受影响模块文档。
