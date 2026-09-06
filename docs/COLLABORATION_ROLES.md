# FRAZIL 双人协作分工

> 状态：Maintained collaboration guide<br>
> 适用阶段：M0 -> v1.0<br>
> 角色：Engineering Lead、Sound & Host Lead<br>
> 具体 assignee 与实时状态：仅在 GitHub Issues/Project 中维护

## 1. 协作模型

FRAZIL 的两位开发者具有互补能力：一位拥有更丰富的代码开发经验，另一位拥有更丰富的编曲、音乐制作和插件使用经验。项目采用“双负责人制”，使每项产品能力同时通过工程质量与声音/宿主体验两道门。

每个 vertical slice 必须指定：

- Implementation DRI：对实现完整性负责；
- Acceptance DRI：对声音、用户行为或宿主验收负责；
- Joint Gate：涉及长期合同的决策必须两人同意。

角色不绑定姓名。团队只在 GitHub 中把成员映射到角色，不因人员变化改写长期规则。当前阶段也不按“Water 一人、Ice 一人”拆成两个互不审查的技术孤岛。

## 2. 角色职责

### 2.1 Engineering Lead

由代码开发经验更丰富的成员担任，主要负责：

- C++20、JUCE、CMake、CI、测试框架和开发工具；
- `plugin -> app -> dsp` 的依赖边界、公共接口和对象所有权；
- audio-thread realtime safety、预分配、smoothing、transition 和 finite output；
- ParameterLayout、ParameterSnapshot、ParameterMapper、StateModel 与 AudioEngine；
- production Water/Ice/Routing DSP 的工程实现；
- unit/property/integration/render harness、ASAN、pluginval 和性能测量；
- code quality review、公共 API 注释、module README 与 MODULE_INDEX 同步。

Engineering Lead 不单独决定最终听感、macro 产品语义、DAW 工作流和 UI 操作优先级，也不能为了实现方便改变参数或 routing 合同。

### 2.2 Sound & Host Lead

由编曲、音乐制作和插件使用经验更丰富的成员担任，主要负责：

- Water/Ice perceptual brief、声音参考、反例和 reject criteria；
- reference corpus 的素材选择、使用场景和许可证信息；
- candidate A/B、loudness match、独立评分和 accept/revise/reject；
- 参数名称、默认值、范围、macro 可理解性和极端行为；
- 目标 DAW 中的参数枚举、automation、state save/reopen 和 offline render；
- routing、enable、Global Mix、Input/Output Gain 的真实插件操作验证；
- UI 信息层级、操作速度、视觉信号流和产品验收；
- 用户文档、已知限制和音乐制作示例。

Sound & Host Lead 不单独决定 production audio-thread 是否安全、跨模块依赖、对象生命周期和 state migration 技术方案，也不能让未通过 property/performance test 的算法进入 `src/dsp/`。

## 3. Ownership matrix

| 工作领域 | Implementation DRI | Acceptance DRI / Required reviewer | 必需证据 |
|---|---|---|---|
| CMake、dependency、CI | Engineering Lead | Sound & Host Lead 复现入口 | fresh clone、Hosted CI、命令记录 |
| GitHub metadata/workflow | Engineering Lead | Sound & Host Lead 检查协作可用性 | labels、milestones、board、ruleset |
| HOST-000 compatibility matrix | Sound & Host Lead | Engineering Lead 检查可支持性 | DAW/OS/format/version matrix |
| ParameterLayout/IDs | Engineering Lead | Sound & Host Lead 审核用户语义 | contract tests、Host enumeration |
| Snapshot/Mapper/State | Engineering Lead | Sound & Host Lead 验证 DAW 行为 | unit/integration、save/reopen |
| Gain/Mix/Smoothing | Engineering Lead | Sound & Host Lead 验证听感和 automation | signal tests、automation renders |
| TESTDATA-001 corpus | Sound & Host Lead | Engineering Lead 检查可复现性 | license、hash、audio metadata |
| Render/measurement tooling | Engineering Lead | Sound & Host Lead 检查易用性 | one-command pack、manifest |
| Water/Ice experiment | Sound & Host Lead 定义实验 | Engineering Lead 实现/审查工具 | perceptual brief、A/B、rubric |
| Water/Ice production DSP | Engineering Lead | Sound & Host Lead 最终验收 | property/render/listening/perf |
| Routing | Engineering Lead | Sound & Host Lead 验证信号流 | routing matrix、DAW、A/B |
| UI | Engineering Lead 实现 | Sound & Host Lead 主导 UX 验收 | screenshots、interaction、DAW |
| EditHistoryManager | Engineering Lead | Sound & Host Lead 验证操作模型 | gesture/source/state tests |
| Beta performance | Engineering Lead 测量/优化 | Sound & Host Lead 检查声音无退化 | mean/P95/P99/worst + listening |
| Release | Engineering Lead 技术签核 | Sound & Host Lead 产品签核 | validators、DAW、listening、hash |

## 4. 当前 M0/M1 的立即分工

当前代码仍是 pass-through AudioEngine；参数 layout 位于 PluginProcessor，Snapshot/Mapper、真实 gain/mix、Water/Ice、Routing 与正式 UI 均未实现。因此先建立 Host/Engine 合同，不立即并行开发 Water 与 Ice。

### Sound & Host Lead

1. `HOST-000`：冻结平台和 DAW compatibility matrix；
2. `TESTDATA-001`：建立 Water/Ice 共用且许可/hash 可追溯的 reference corpus；
3. 复核九个核心参数的名称、默认值、单位、automation、mode relevance 和 state restore；
4. 为 `AUTO-001` 编写真实 DAW automation acceptance scenarios；
5. 为 `PERF-BASE-001` 选择代表性音频、项目和实际 block size；
6. 起草 `EXP-W-001` Water perceptual brief，不把候选算法写成结论。

### Engineering Lead

1. 收口 `GH-001/GH-002`、Hosted CI、dependency bootstrap 和测试入口；
2. `ARCH-001`：建立 ProcessSpec 与 EngineParameters；
3. `PARAM-001`：提取 `src/plugin/ParameterLayout.*` 并处理 `.enable` -> `.enabled`；
4. `PARAM-002`：建立每 block coherent ParameterSnapshot；
5. `PARAM-003`：建立 ParameterMapper 和边界测试；
6. `APP-001`：让 AudioEngine 消费 EngineParameters；
7. `DSP-001..005`：gain、Global Mix、smoothing 和 RandomSource；
8. `STATE-001/002`、`RENDER-001`、`TEST-002` 与 `PERF-BASE-001` 工具/证据。

### 当前合流点

| 合流点 | Engineering Lead 提供 | Sound & Host Lead 提供 | 通过条件 |
|---|---|---|---|
| Host contract review | 参数实现约束、兼容性风险 | DAW/用户语义和 automation 场景 | 九个参数逐项签核 |
| M1 engine gate | Snapshot/Mapper/AudioEngine/tests | DAW save/reopen/automation 复现 | M1 Exit Gate 全满足 |
| Experiment readiness | one-command render/measure tooling | corpus、brief、rubric、references | 可重复 A/B 而无需改核心 C++ |

## 5. M2/M3 Water 与 Ice

第一条 Water vertical slice 使用共同流水线：

```text
Sound & Host Lead 定义听感目标、素材和 reject criteria
    -> 两人 Contract Review
    -> Engineering Lead 实现最小 candidate
    -> Sound & Host Lead 独立盲听和参数扫描
    -> Engineering Lead 处理 artifact、稳定性和性能
    -> 两人共同 accept / revise / reject
    -> 通过 production gate 后进入 src/dsp/
```

Water 流程稳定后再复用到 Ice。M2/M3 可以并行进行 brief、素材和 experiment design，但在第一条 production vertical slice 通过前，不并行推进两套高风险实时 C++ DSP。

每个 candidate 同时通过：

- Engineering Gate：realtime-safe、finite、deterministic test、automation-safe、性能可解释；
- Sound Gate：材质辨识成立、输入仍可辨识、具有音乐用途、artifact 可接受；
- Host Gate：目标 DAW 的 enumeration/automation/state/render 行为可接受。

## 6. 决策权

Engineering Lead 可以阻断 audio-thread 不安全、ownership/dependency 不明、兼容性迁移缺失、NaN/Inf、不可控爆峰/随机、无数据的性能回退及隐藏 global/singleton。

Sound & Host Lead 可以阻断材质身份不成立、输入主体不可辨识、参数语义与听感不符、DAW 行为反直觉、candidate 退化成额外 one-shot 拟音或 GUI 无法服务真实制作工作流。

以下事项必须两人同意：

- Parameter ID、choice、range、default、macro 和 `PARAM-FREEZE-001`；
- Water/Ice production algorithm；
- routing crossfade/stage mix law 与 `ADR-R-001`；
- latency/tail/random persistence 语义；
- formal performance budget；
- Beta 和 Release go/no-go。

意见不一致时工作项进入 Blocked，记录双方证据和最小验证实验；任一角色不得覆盖另一领域的阻断意见。

## 7. Issue 交接格式

```text
Stable ID / Milestone:
Implementation DRI:
Acceptance DRI:
Contract and related ADR:
Problem / sound or user goal:
Scope:
Non-goals:
Inputs / fixtures:
Engineering acceptance:
Sound / DAW acceptance:
Documentation impact:
Joint decision required:
```

DRI 负责推动工作，不代表可以自行验收。具体姓名、状态、截止信息和 board column 只写 GitHub。

## 8. 标准协作流程

1. Contract Review：两人确认合同、范围、非目标、fixture、验收和 ADR trigger；
2. Implementation：Implementation DRI 在短生命周期分支实现，团队 WIP 不超过 2；
3. Functional Validation：工程侧执行自动测试/性能，声音侧执行 listening/DAW/automation/state/UI；
4. Code Quality Review：按 `CODE_STANDARDS.md` 检查边界、ownership、realtime 和可维护性；
5. Comment & Documentation Pass：同步注释、module README、`MODULE_INDEX.md` 和受影响合同；
6. Final Validation：另一位成员复现关键证据，PR 记录命令、环境、素材/seed、结果和未执行项。

## 9. Review、能力交叉与节奏

- Engineering Lead authored PR：Sound & Host Lead 检查用户/Host/声音行为和测试可理解性；
- Sound & Host Lead authored experiment/docs/test-data PR：Engineering Lead 检查可复现性、许可、格式和 LOCKED contract；
- 参数、routing、state、核心 DSP、latency、random、performance budget 和 release PR 必须两人 review；
- reviewer 至少说明复现了哪项证据或明确 review 边界，不能只写“LGTM”。

Sound & Host Lead 逐步掌握 test manifest、Python experiment/参数 sweep、C++ unit test、简单 mapping/UI attachment，以及独立运行 CTest/pluginval。Engineering Lead 必须在目标 DAW 复现 automation/state、独立记录听测，并维护无需修改核心 C++ 即可完成 A/B 的工具。

推荐节奏：

- Milestone kickoff：共同完成 Contract Review、依赖排序和 issue 拆分；
- 日常异步：状态、证据和 blocker 更新到 GitHub issue/PR；
- 每周 engineering review：接口、technical debt、realtime/performance、CI；
- 每周 listening/Host review：固定素材 A/B、DAW automation/state、产品风险；
- Milestone exit review：逐条检查 Coding Plan Exit Gate。

## 10. 反模式

- 工程负责人写完功能后才交给另一人试听；
- 声音负责人只给口头形容，不提供素材、参数和复现步骤；
- 一人拥有全部代码知识，另一人无法构建或解释接口；
- 一人拥有全部声音判断，另一人从不在 DAW 中复现；
- 按 Water/Ice 切成互不审查的技术孤岛；
- 为并行而在公共合同冻结前创建两套不兼容接口；
- 把 live assignee 和短期状态写进长期文档。

## 11. Modification Policy

本文件属于 LEVEL 3 MAINTAINED collaboration guide，用于解释 `CODING_PLAN.md` 已接受的两人协作方式，不改变 Level 1/2 产品、架构、参数、实时或测试合同。

- 角色名称保持稳定，具体人员映射和实时 assignments 由 GitHub 管理；
- 修改 decision rights、review requirement、WIP 或 production workflow 时，按 CONTROLLED 变更同步 `AGENTS.md`、`CODE_STANDARDS.md`、`DOCUMENT_GOVERNANCE.md` 和 `CODING_PLAN.md`；
- 更新示例、交接格式或解释性文字可作为普通 docs PR；
- 本文件不得降低 Production Definition of Done 或绕过 LOCKED contract。
