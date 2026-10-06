# FRAZIL 测试路径系统性重构 —— 强制实际落地版 Agent 执行命令

> Repository: `jjjphens-dot/FRAZIL`
> 任务性质：测试基础设施系统性重构
> 核心要求：**必须修改实际工程实现并完成验证；不接受仅文档修订。**
> 上一轮工程审查基准：`ff75735ffd9ec6a29d1ce76e277074e361dc902b`
> 执行前必须重新确认是否已有更新的 authoritative implementation HEAD。

---

## 0. 最高优先级规则：禁止再次只改文档

本任务已经完成多轮分析、review 和规划。现在不允许再次新增一份 plan/review/instruction Markdown 后停止。

以下行为直接视为任务失败：

- 发现 CMake/CTest/CI 问题，只在文档中写 TODO；
- 指出 `frazil_smoke` 依赖整个 Water research graph，却不修改 dependency graph；
- 指出 module-fast 过滤错误，却不修改 preset；
- 指出 CLI test 混入 full matrix/listening/native，却继续保留原状；
- 指出 performance 与 correctness 混写，却只更新 `TESTING.md`；
- 指出 CI 每次全量执行浪费，却不修改 workflow；
- 用 `PLANNED` / `FOLLOW-UP` / `DEFERRED` 掩盖本任务范围内已经确认的问题；
- 最终 branch 只有 `docs/**` 改动。

### 问题闭环只允许三种状态

1. `FIXED`：实际实现已修改，并有测试/日志证明。
2. `NOT A BUG / INTENTIONAL`：有当前代码与合同证明原行为是有意设计。
3. `BLOCKED`：存在真实、可复现、无法安全解决的 blocker；必须停止并把整个任务标为 `INCOMPLETE`。

禁止用 `TODO`、`later`、`future work` 关闭本任务范围内问题。

---

# 1. 必须结合当前项目文档执行

开始前必须读取当前 authoritative HEAD 中：

- `AGENTS.md`
- `CMakeLists.txt`
- `CMakePresets.json`
- `.github/workflows/ci.yml`
- `docs/TESTING.md`
- `docs/CODING_PLAN.md`
- `docs/MODULE_INDEX.md`
- `docs/PROJECT_STATUS.md`
- `docs/DOCUMENT_GOVERNANCE.md`
- `docs/ENVIRONMENT.md`
- `tests/README.md`
- `tools/README.md`
- `experiments/water/README.md`
- `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt`
- `experiments/water/SPIKE-W-DSP-001/README.md`

必须遵守以下已有项目规则：

### `docs/TESTING.md`
现有测试层级是：

- L0 Build/Smoke
- L1 Unit
- L2 DSP Property
- L3 Render Regression
- L4 Integration
- L5 Plugin Validation
- L6 DAW Acceptance
- L7 Listening
- L8 Performance

且明确要求 **Validation is impact-based**。

因此本次重构要把 impact-based 原则真正落实到 CTest / Preset / CI，而不是继续让普通改动跑全部 Water research。

`docs/TESTING.md` 同时说明测试合同/层级/门槛属于 CONTROLLED 内容，因此修改测试架构时必须同步 `docs/CODING_PLAN.md` 或相关 ADR，不能只改 `TESTING.md`。

### `AGENTS.md`
代码任务必须经历：

`Contract Review -> Implementation -> Functional Validation -> Code Quality Review -> Comment & Documentation Pass -> Final Validation`

其中 **Implementation 必须真实发生**。

Windows build 必须走：

`python tools/build_safe.py --preset <name>`

不得用裸 `cmake --build` / `ninja` 绕过安全 wrapper。Debug / Release / ASAN 重型 pipeline 必须串行执行。

### `MODULE_INDEX.md` / `PROJECT_STATUS.md`
A1 / B1 / B2 / D1 / Protect / Preview 当前属于 Water research / developer tooling；Production Water DSP 尚未正式实现。本任务不得改变声音算法、物理模型、默认值、Host 参数、state、routing、latency 或 perceptual contract。

---

# 2. 当前已确认、必须实际修正的问题

## TP-001：`frazil_smoke` 被错误用作 research build-all

当前 Water SPIKE CMake 存在：

- `add_dependencies(frazil_smoke frazil_water_flow_d1_latency_native)`
- `add_dependencies(frazil_smoke frazil_water_flow_d1_source_probe)`
- `add_dependencies(frazil_smoke ${water_test_targets} ...performance...render...research_cases...)`

这破坏了 L0 Smoke 的语义。

### 必须修复
- 移除所有 `frazil_smoke -> Water research` 强依赖；
- 由真正使用 helper 的 test/aggregate target 声明依赖；
- 如需要 aggregate build target，建立 `frazil_test_core` / `frazil_test_water_fast` / `frazil_test_water_full` 等；
- `frazil_smoke` 只构建自身真正需要的最小依赖。

---

## TP-002：当前 Test Preset 没有 scoped 路径

当前只有通用 Debug / Release / ASAN / CI preset，没有：

- core
- global fast
- water-common
- A1
- B1
- B2
- D1
- Protect
- Preview
- full

### 必须修复
真实修改 `CMakePresets.json`，建立可执行 scoped presets。

---

## TP-003：Module Label 不能代表 Module Fast

`ctest -L water-a1` 会选中所有带 `water-a1` 的测试，包括 slow/research。

### 必须修复
CLI 必须使用真正的交集，例如：

`ctest -L fast -L water-a1`

Preset 必须实现 `module AND fast`，不能只按 module label。

---

## TP-004：默认完整路径过重

历史 Hosted Debug baseline：

`42/42 PASS，CTest ≈ 484.27 s`

历史主要慢项包括：

- `frazil_water_droplet_b1_cli` ~79.78 s
- `frazil_water_flow_d1_cli` ~69.58 s
- `frazil_water_droplet_b1` ~47.36 s
- `frazil_water_flow_d1_latency_native` ~45.93 s
- `frazil_water_experiment_render_cli` ~41.00 s
- `frazil_water_flow_d1_remediation` ~40.58 s
- `frazil_water_flow_d1_convergence` ~40.26 s
- `frazil_water_preview` ~22.51 s
- `frazil_water_droplet_b2` ~21.77 s

这些数字只是参考，必须在当前 HEAD 重新测量。

---

## TP-005：CLI regression 与 research validation 混在一起

必须实际拆：

### `render_cli_test.py`
拆为：
- `render_cli_smoke.py`
- `render_cli_contract.py`
- `render_cli_full_matrix.py`

### `droplet_b1_cli_test.py`
拆为：
- `b1_cli_smoke.py`
- `b1_cli_contract.py`
- `b1_cli_full_matrix.py`
- `b1_listening_pack_validation.py`

### `flow_d1_cli_test.py`
拆为：
- `d1_cli_smoke.py`
- `d1_cli_contract.py`
- `d1_native_oracle_validation.py`

必须保留旧 coverage，不能因为慢就删除。

---

## TP-006：Correctness 与 Performance 混写

`droplet_b2_tests.cpp` 中存在 callback timing / `std::chrono` 性能测量。

### 必须修复
- correctness test 只保留有限性、attack、spacing、stereo、radius/spread、lifecycle 等正确性；
- performance 移到独立 harness；
- performance 标记 `slow;performance;research`。

---

## TP-007：D1 research validation 混入普通路径

以下保留但必须退出 fast：

- `flow_d1_latency_native`
- `flow_d1_remediation`
- `flow_d1_convergence`

普通 D1 fast 仅保留 canonical functional/schema/finite/small numerical invariant/partition boundary。

---

## TP-008：Preview regression 过于集中

不能只给整个 `frazil_water_preview_tests` 加一个 label 就结束。

必须至少支持 logical groups：

- core
- session
- diagnostics
- audition
- parameters
- workflow

可以共享一个 executable，但要能按 group 执行并注册成多个 CTest entry。

---

## TP-009：Testdata 深度验证应按需

普通 fast/core：
- `verify_testdata.py`

完整 regeneration / semantic / byte reproducibility：
- `test_testdata.py`

仅 testdata/generator 变化或 full validation 时运行完整深度测试。

---

## TP-010：Research Python dependency 不应所有 PR 无条件安装

Core-only job 不得无条件安装完整 NumPy/SciPy/soundfile/matplotlib research stack。

按实际 job/preset 安装；优先复用 canonical `requirements-dsp.txt`，不复制版本。

---

# 3. 先修正实施基线

当前 planning docs 历史来自旧 `main@3c95e472...`，而上一轮真实测试体系 review 基线为：

`ff75735ffd9ec6a29d1ce76e277074e361dc902b`

执行：

```bash
git fetch --all --prune
git branch -a
git log --all --date=iso --pretty=format:"%h %ad %d %s" -n 80
```

重新确认：
- 最新 Water implementation HEAD；
- 最新 test-related implementation HEAD；
- 最新 R3.1 closeout HEAD。

如果没有更新的综合实现 HEAD，则使用 `ff75735...`。

从正确 HEAD 新建：

```bash
git checkout <AUTHORITATIVE_HEAD>
git checkout -b codex/refactor/test-paths
```

已有两份 planning/review 文档可迁入作为 reference，但迁入文档不是任务成果。

---

# 4. 建立真实 Test Inventory

先 configure，再：

```bash
ctest --test-dir <build-dir> -N
```

记录每个 test：

- name
- owner/module
- tier
- kind
- current runtime
- helper dependencies
- current CTest command

建立 `docs/testing/TEST_PATH_MATRIX.md`，但该表必须由实际 CMake registration 驱动，不能反过来只写表不改 CMake。

---

# 5. 为所有 CTest 增加真实分类

## Module
至少：
- `core`
- `water-common`
- `water-a1`
- `water-b1`
- `water-b2`
- `water-d1`
- `water-protect`
- `water-preview`

必要时可增加 `water-legacy` / `tooling`，但不能为方便随意多标 module。

## Tier
- `fast`
- `slow`

同一个 test 不允许同时 fast + slow。

## Kind
按实际组合：
- `smoke`
- `unit`
- `integration`
- `property`
- `cli`
- `render`
- `native`
- `research`
- `performance`
- `listening`
- `evidence`
- `tooling`

分类必须通过 CMake `set_tests_properties` 或等价 helper 实际注册。

---

# 6. 建立 scoped Test Presets

至少：

- `windows-debug-core`
- `windows-debug-fast`
- `windows-debug-water-common`
- `windows-debug-water-a1`
- `windows-debug-water-b1`
- `windows-debug-water-b2`
- `windows-debug-water-d1`
- `windows-debug-water-protect`
- `windows-debug-preview`
- `windows-debug-full`
- `windows-release-full`
- `windows-asan-full`

要求：
- core = `core AND fast`
- global fast = 所有 `fast`
- module fast = `module AND fast`
- full = 当前全部注册测试

使用 `ctest --preset ... -N` 证明 selection 正确。

---

# 7. 修复 Smoke Build Graph

删除所有 Water CMake 中错误的：

`add_dependencies(frazil_smoke ...)`

对于真正需要 helper executable 的测试，给对应 target 建立显式 dependency。

不得通过“smoke 顺便先 build”维持隐式依赖。

如果需要 aggregate target，显式创建，不允许继续滥用 `frazil_smoke`。

---

# 8. 拆大型 CLI

## 8.1 Render CLI
Smoke 只保留：
- 48k canonical input
- block 128
- baseline
- one residual
- finite
- length
- one invalid config/mode

Contract：
- schema
- overwrite
- active/inactive config
- odd block
- key rejection

Full：
- 44.1/48/96
- full block matrix
- modes/composition
- extensive invalid corpus
- cross-rate

## 8.2 B1 CLI
把 study/listening pack/full rate-block matrix移出 fast。

## 8.3 D1 CLI
把 native source/oracle comparison 移出 fast，独立为 slow/native/research。

---

# 9. 拆重型 C++ matrix

## B1
拆为：
- `b1_fast`
- `b1_full_matrix`

## A1
拆为：
- `a1_fast`
- `a1_full`

## B2
correctness 与 performance 完全分离。

Fast 只保留 representative + boundary；Full 保留完整矩阵。

---

# 10. D1 Research Isolation

`latency_native / remediation / convergence` 必须继续存在，但只能由 slow/full/research 路径运行。

不得为了提速删除研究证据能力。

---

# 11. Preview Logical Groups

为 Preview test runner实现 `--group` 或等价机制，并注册多个 CTest。

目标：小改动可只跑相关 Preview group，而不是全部 Preview regression。

---

# 12. Testdata 路径拆分

Core/Fast：
`verify_testdata.py`

Full/Testdata-change：
`test_testdata.py`

CI 必须真正体现差异。

---

# 13. CI 按影响范围路由

修改 `.github/workflows/ci.yml`，至少形成逻辑：

## `ci-core`
- portability/tool checks
- core build
- core fast

## `ci-water-fast`
changed files 决定：
- water-common
- A1
- B1
- B2
- D1
- Protect
- Preview

共享 Water headers/renderer/config 变化必须触发真正依赖它的模块，不能只按文件名字符串猜测。

## `validation-full`
通过 workflow_dispatch 或明确 stage gate运行：
- Debug full
- Release full
- ASAN full
- native/convergence/performance/listening/evidence（按阶段适用）

---

# 14. Coverage Preservation 硬门槛

每次拆测试必须建立：

`Old coverage -> New Fast + New Contract + New Full/Research`

`docs/testing/TEST_PATH_MATRIX.md` 必须列出 old responsibility 和 new path。

正常情况下：
`Removed coverage = none`

如果删除重复 case，必须证明有精确等价覆盖。

---

# 15. Fast Representative Matrix

不要在 fast 中跑完整笛卡尔积。

默认参考：

- 48 kHz / 128：canonical
- 48 kHz / 257：odd
- 96 kHz / 1024：large representative
- 44.1 kHz / 128：cross-rate

但不得删除必要的：
- zero
- odd
- finite
- reset/reprepare
- stereo isolation
- deterministic seed
- boundary capacity
- strict schema/rejection

---

# 16. 性能目标

参考旧 baseline：
`Full CTest ≈ 484.27 s`

最终目标：
- Core-only：约 ≤ 5–10 s
- Global fast：约 ≤ 30–40 s
- 单 Water module fast：约 ≤ 30 s
- Full：允许数分钟

如果 fast 仍被少数长测试明显拖慢：
1. 输出 per-test time；
2. 定位责任；
3. 继续拆分/修正；
4. 重新验证。

禁止只在报告中写“still slow”。

---

# 17. Water R3.1 既有问题不能被测试重构掩盖

`PROJECT_STATUS.md` 当前仍记录 Water R3.1 research 的：
- Python write fault；
- native/renderer failure history；
- realtime margin open。

这些不是本测试路径任务要解决的 Water 算法问题，但必须：
- 保留；
- 不写成 FIXED；
- 不因重构消失；
- full/research path 仍能复现/保留第一失败证据。

如果重构引入新的 failure 或改变失败性质，则必须修复重构本身。

---

# 18. 文档必须在实现之后同步

顺序必须是：

1. 实际 CMake/CTest/test/CI 修改；
2. Functional Validation；
3. Code Quality Review；
4. 最后才更新文档；
5. Final Validation。

实现完成后至少同步：
- `tests/README.md`
- `docs/TESTING.md`
- `docs/CODING_PLAN.md`
- `docs/MODULE_INDEX.md`（职责变化时）
- `tools/README.md`（入口变化时）
- `docs/PROJECT_STATUS.md`（只记录实际验证结果）
- `docs/testing/TEST_PATH_MATRIX.md`

如 build/CI contract 变化，按 Documentation Impact Check 同步 `ENVIRONMENT.md` / `GITHUB_WORKFLOW.md`。

---

# 19. 严禁修改生产声音/合同

不得修改：
- `src/dsp/**` 声音算法；
- A1/B1/B2/D1 物理模型；
- Water defaults；
- random semantics；
- Host Parameter IDs/ranges；
- APVTS layout；
- state schema；
- routing semantics；
- production latency；
- Perceptual Contract；
- candidate acceptance。

测试难拆时，优先修改 test harness / adapter / registration，不要改 DSP 迁就测试。

---

# 20. 验证要求

## Static
至少执行：
- `python tools/check_portability.py`
- `python tools/check_markdown_links.py`
- `python tools/check_vscode_tasks.py`
- `git diff --check`

## Build
全部 Windows build 使用：
`python tools/build_safe.py --preset <name>`

## Selection
执行所有 scoped preset 的 `-N`，确认：
- fast 不含 slow/research/listening/performance；
- module-fast 不含无关 module；
- full 没有静默丢测试。

## Execution
实际执行：
- core
- global fast
- water-common
- A1
- B1
- B2
- D1
- Protect
- Preview
- Debug full

再按项目当前适用范围串行执行：
- Release full
- ASAN full

保留第一次失败，不允许 retry-until-pass。

---

# 21. Code Quality Review 必须修复 finding

实现后单独检查：

- CMake target ownership；
- label 冲突；
- preset regex 是否脆弱；
- helper dependency 是否仍偷偷依赖 smoke；
- Python helper 是否重复；
- performance/listening 是否混入 fast；
- testdata regeneration 是否混入 core；
- shared Water file CI mapping 是否漏依赖；
- ASAN runtime handling 是否仍完整；
- production source 是否出现无关 diff。

**发现问题后必须立即修复并重新验证，不能只记录 review finding。**

---

# 22. 最终验收表

以下全部必须 PASS 或有明确 pre-existing failure 证据：

- authoritative baseline 正确；
- smoke 完全解耦；
- 全部 tests 已分类；
- module AND fast 正确；
- core preset 正确；
- global fast 正确；
- A1/B1/B2/D1/Protect/Preview scoped path 正确；
- heavy CLI 已拆；
- A1/B1/B2 matrix 已拆；
- B2 performance 已隔离；
- D1 research 已隔离；
- Preview groups 可独立运行；
- testdata verify/regeneration 已分离；
- research dependency 按需；
- CI impact routing 已落地；
- coverage mapping 完整；
- Full Debug 完整；
- Release/ASAN 按适用范围完成；
- production DSP/Host/state/routing 无改变；
- 文档在实现后同步。

任一当前任务范围内项目仍是 `TODO/PLANNED`，不得声明完成。

---

# 23. 推荐 commit 划分

建议：

1. `refactor(testing): classify existing test graph`
2. `build(testing): decouple smoke and add scoped presets`
3. `test(water): split heavy CLI validation paths`
4. `test(water): separate fast and full DSP matrices`
5. `test(water): isolate D1 research and preview groups`
6. `ci(testing): route core water and full validation`
7. `docs(testing): synchronize implemented test architecture`

禁止以 `docs(testing): describe future refactor` 作为最终成果。

---

# 24. Agent 最终报告格式

必须逐项提供：

## Execution baseline
- repository
- branch
- authoritative base SHA
- final HEAD
- planning docs imported

## Problems and closure
对每个 TP-xxx：
- problem
- root cause
- files changed
- actual fix
- validation
- final status (`FIXED / NOT A BUG / BLOCKED`)

## Files changed
按 Build / CTest / Test source / Tooling / CI / Docs 分类。

## Test architecture
列出 module / tier / kind / preset。

## Smoke dependency before/after
明确真实 dependency graph 变化。

## Heavy-test split
逐项说明 render CLI / B1 CLI / D1 CLI / A1 / B1 / B2 / D1 / Preview。

## CI routing
说明 Core/A1/B1/B2/D1/Preview/shared/full 各跑什么。

## Timing
至少：
- Full before / after
- Core
- Global fast
- Water common
- A1
- B1
- B2
- D1
- Protect
- Preview

## Coverage
- old coverage retained
- moved coverage
- removed coverage（正常应为 none）

## Validation
- Debug
- Release
- ASAN
- static checks

## Production impact
必须明确：
- Water DSP unchanged
- Audio output behavior unchanged
- Host parameters unchanged
- State schema unchanged
- Routing unchanged
- Latency contract unchanged
- Perceptual contract unchanged

## Blockers
理想：
`Test-path-refactor blockers: 0`

Water research 既有问题单独列，不得混同。

---

# 25. Definition of Done

只有以下全部满足才可结束：

- [ ] 从当前 authoritative implementation HEAD 执行；
- [ ] 没有在旧 planning branch 上实现；
- [ ] 真实修改 CMake/CTest/test/CI，而不是只有文档；
- [ ] `frazil_smoke` 已解除 research build-all；
- [ ] 所有 CTest 有 module/tier/kind；
- [ ] fast 不含 slow/research/listening/performance；
- [ ] module-fast 真正是 module AND fast；
- [ ] heavy CLI 已实际拆分；
- [ ] A1/B1/B2 fast/full matrix 已实际拆分；
- [ ] B2 performance 与 correctness 已分离；
- [ ] D1 native/remediation/convergence 已从 fast 隔离；
- [ ] Preview 已形成可选择 logical groups；
- [ ] testdata verify 与 regeneration 已分离；
- [ ] research dependencies 按需；
- [ ] CI impact-based routing 已落地；
- [ ] Full validation capability 未丢；
- [ ] `TEST_PATH_MATRIX.md` 与实际 CMake 一致；
- [ ] `docs/TESTING.md` 与 `docs/CODING_PLAN.md` 已同步；
- [ ] `PROJECT_STATUS.md` 只记录实际事实；
- [ ] Debug/Release/ASAN 按适用范围执行；
- [ ] 本任务发现的问题全部 FIXED / NOT-A-BUG / BLOCKED；
- [ ] 若有 BLOCKED，任务状态为 INCOMPLETE；
- [ ] 无生产 Water 声音/Host/state/routing 合同变化。

---

# 26. 可直接交给 Agent 的最终命令

现在开始执行 FRAZIL 测试路径系统性重构，禁止再次生成新的 planning-only / review-only Markdown 后停止。

先读取当前项目的 AGENTS.md、CMakeLists.txt、CMakePresets.json、.github/workflows/ci.yml、docs/TESTING.md、docs/CODING_PLAN.md、docs/MODULE_INDEX.md、docs/PROJECT_STATUS.md、tests/README.md、experiments/water/README.md 与 experiments/water/SPIKE-W-DSP-001/CMakeLists.txt。重新 fetch 全部分支并确认 authoritative Water/test implementation HEAD；上一轮 review 基准为 ff75735ffd9ec6a29d1ce76e277074e361dc902b，如果没有更新的综合实现 HEAD，就以它为基线，从正确实现 HEAD 创建 codex/refactor/test-paths。不要在基于旧 main@3c95e472 的 planning branch 上实施。

本次工作必须真实修复已经确认的全部 test-path 问题：完成当前 CTest inventory；增加真实 module/tier/kind labels；建立 core/global-fast/water-common/A1/B1/B2/D1/Protect/Preview/full presets，并保证 module-fast 真正是 module AND fast；移除所有 frazil_smoke 对 Water research graph 的错误 dependency，并修复由此暴露的真正 helper dependency；实际拆分 render_cli_test.py、droplet_b1_cli_test.py、flow_d1_cli_test.py；拆分 A1/B1 的 fast/full matrix；从 B2 correctness 中移除 performance timing；把 D1 latency-native/remediation/convergence 隔离到 slow/native/research；给 Preview 实现可选择 logical groups；把 testdata verify 与 full regeneration 分离；把 research Python dependency 改为按 job/preset 安装；修改 CI 实现 impact-based core/module-fast 路由与显式 full validation；建立 TEST_PATH_MATRIX 证明 coverage 未丢。

任何本任务范围内已经发现的问题都不允许只写进 TODO 或文档。发现后必须修复；如确实无法安全修复，给出可复现 blocker，并将整个任务标为 INCOMPLETE。绝对不允许“问题已经提出，但代码保持原样，然后在文档中说后续处理”。

所有 Windows build 必须使用 python tools/build_safe.py --preset <name>，重型 Debug/Release/ASAN 串行执行。实现完成后必须运行 scoped selection、所有 fast/module paths、full Debug，并按适用范围执行 Release/ASAN；保留第一次失败，不做 retry-until-pass。若 fast 仍被少数长测试明显拖慢，继续定位和拆分，不允许只报告问题。

实现完成并通过 Functional Validation、Code Quality Review 后再同步文档。由于 docs/TESTING.md 的测试合同属于 CONTROLLED，必须同步 docs/CODING_PLAN.md；PROJECT_STATUS.md 只能记录实际验证事实，不得提前写完成。

不得修改 Water 声音算法、物理模型、默认值、Host parameters、state、routing、latency 或 perceptual contract。

最终报告必须按 problem -> root cause -> files changed -> fix -> validation -> status 逐项闭环。范围内问题只允许 FIXED / NOT A BUG / BLOCKED。存在 BLOCKED 时不得声明 COMPLETE。

只有测试路径问题全部闭环、Full coverage 保留、fast/module/full 路径实际可运行且生产 DSP/Host 合同无改变时，最终状态才能写：

`TEST PATH SYSTEMATIC REFACTOR: COMPLETE`
`TEST-PATH BLOCKERS: 0`

否则必须写：

`TEST PATH SYSTEMATIC REFACTOR: INCOMPLETE`
并给出 blocker。
