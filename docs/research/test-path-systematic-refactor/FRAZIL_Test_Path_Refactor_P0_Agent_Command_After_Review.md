# FRAZIL 测试路径重构 — Review 后 Agent 执行命令

> 项目：`jjjphens-dot/FRAZIL`  
> 任务性质：测试基础设施重构 / P0  
> 当前状态：**Planning Complete / Implementation Not Started**  
> 本轮目标：**只完成 P0，不允许继续推进 P1–P6。**

---

# 1. Review 结论

最新提交：

```text
593a08fe9262912fe2b25e13aae297bdd29b0d6c
docs(testing): add test path refactor agent plan
```

当前提交只新增：

```text
docs/research/test-path-systematic-refactor/
FRAZIL_Test_Path_Systematic_Refactor_Agent_Plan.md
```

没有修改：

```text
CMakeLists.txt
CMakePresets.json
.github/workflows/ci.yml
tests/**
experiments/water/SPIKE-W-DSP-001/tests/**
```

因此当前真实状态必须认定为：

```text
TEST PATH REFACTOR

Planning:        COMPLETE
Implementation: NOT STARTED
P0:             NOT STARTED
P1–P6:          NOT STARTED
Validation:     NOT RUN
```

不得将 `593a08f` 描述为测试重构已经完成。

---

# 2. P0 Blocker：当前方案分支基线错误

当前方案分支：

```text
docs/test-path-systematic-refactor-agent-plan
HEAD = 593a08fe9262912fe2b25e13aae297bdd29b0d6c
```

其 parent：

```text
3c95e47212a03d43ccf06a2484d8f3861a4f6b33
```

而本轮测试体系实际 review 的 Water research 基线为：

```text
ff75735ffd9ec6a29d1ce76e277074e361dc902b
```

Git compare 显示：

```text
status: diverged
593a08f ahead by: 1
593a08f behind by: 81
merge base: 3c95e472...
```

因此：

> **禁止直接在 `docs/test-path-systematic-refactor-agent-plan` 分支上开始实施测试重构。**

该分支基于过旧的 `main`，不包含当前 A1 / B1 / B2 / D1 / Protect / Preview / R3.1 测试体系。

---

# 3. 第一条命令：重新确认 authoritative HEAD

执行前重新检查仓库。

至少检查：

```bash
git fetch --all --prune
git branch -a
git log --all --date=iso --pretty=format:"%h %ad %d %s" -n 50
```

确认：

1. 当前最新 Water research implementation branch；
2. 当前最新 test-related implementation branch；
3. 是否已有比 `ff75735...` 更新且已经包含后续 Water 实现的 HEAD。

如果没有更新，则本轮 authoritative implementation HEAD 使用：

```text
ff75735ffd9ec6a29d1ce76e277074e361dc902b
```

如果已有更新：

- 使用更新后的实际 implementation HEAD；
- 在最终报告中写清楚：
  ```text
  previous review baseline: ff75735...
  actual execution baseline: <NEW_SHA>
  reason: newer implementation commit exists
  ```

禁止默认以 `main@3c95e472` 为实施基线。

---

# 4. 创建正确的实施分支

从 authoritative implementation HEAD 创建新分支。

建议：

```bash
git checkout <AUTHORITATIVE_IMPLEMENTATION_HEAD>
git checkout -b codex/refactor/test-paths-p0
```

然后仅迁移方案文档提交：

```bash
git cherry-pick 593a08fe9262912fe2b25e13aae297bdd29b0d6c
```

如果 cherry-pick 因路径/文档冲突失败：

- 只解决该 Markdown 文档冲突；
- 不借机修改其他文件；
- 不从旧分支 merge 代码。

完成后确认：

```bash
git status
git log --oneline --decorate -n 5
```

工作树必须 clean 后再开始 P0。

---

# 5. 本轮只允许执行 P0

本轮允许修改：

```text
CMakeLists.txt
CMakePresets.json
experiments/water/SPIKE-W-DSP-001/CMakeLists.txt
tests/README.md
docs/TESTING.md
必要的测试基础设施文档
必要的 coverage/test inventory 文档
```

如为了 label / preset 注册确有必要，可修改少量 test registration 相关文件。

本轮禁止：

```text
修改 DSP algorithm
修改 Water 声音
修改 A1/B1/B2/D1 数学实现
修改 Protect 算法
修改 Host parameter
修改 APVTS/state serialization
修改 routing
修改 production latency
修改 perceptual contract
删除大量 test cases
拆大型 test source
拆 CLI
拆 Preview executable
做 P1/P2/P3/P4/P5/P6
```

---

# 6. P0 任务 1：Current Test Inventory

重新 configure 当前 authoritative HEAD。

然后输出全部测试清单：

```bash
ctest --test-dir <CURRENT_BUILD_DIR> -N
```

记录：

```text
Total CTest count
Core test count
Water test count
Preview test count
Research/native/listening test count
Tests without labels
```

同时记录当前完整 CTest baseline timing：

```bash
ctest --preset <CURRENT_FULL_DEBUG_PRESET> --output-on-failure
```

禁止为了生成 baseline 重复运行失败测试直到通过。

如存在 known failure：

- 保留第一次失败；
- 写明 known issue；
- 不隐藏。

---

# 7. P0 任务 2：给现有测试添加分类 Label

P0 不改变 test body。

只添加分类。

至少使用以下三个正交维度：

## Module

```text
core
water-common
water-a1
water-b1
water-b2
water-d1
water-protect
water-preview
```

## Tier

```text
fast
slow
```

## Kind

按实际情况：

```text
unit
integration
property
cli
render
native
research
performance
listening
evidence
```

一个测试可以拥有多个 label。

例如：

```cmake
set_tests_properties(
    frazil_water_bubble_a1
    PROPERTIES
    LABELS "water-a1;fast;unit"
)
```

研究测试：

```cmake
set_tests_properties(
    frazil_water_flow_d1_convergence
    PROPERTIES
    LABELS "water-d1;slow;research"
)
```

---

# 8. 重要修正：Module Label 不等于 Module Fast

禁止把：

```bash
ctest -L water-a1
```

定义为 “A1 fast”。

因为 slow/research test 同样可能拥有：

```text
water-a1
```

所以：

```bash
ctest -L water-a1
```

会把：

```text
water-a1 + fast
water-a1 + slow
water-a1 + research
```

全部选中。

---

# 9. CLI 中必须使用 AND 过滤

Module fast CLI 应采用多个 `-L`：

```bash
ctest -L fast -L water-a1
ctest -L fast -L water-b1
ctest -L fast -L water-b2
ctest -L fast -L water-d1
ctest -L fast -L water-preview
```

多个 `-L` 必须形成 intersection。

验证：

```text
water-a1 fast
```

不得包含：

```text
water-a1 slow
water-a1 research
```

---

# 10. Test Preset 必须真正实现 module ∩ fast

CTest preset 的 module fast 不得只使用：

```json
"label": "water-a1"
```

否则会重新包含 slow/research。

推荐使用：

```text
test-name/module registration
+
fast label
```

例如概念上：

```json
{
  "name": "windows-debug-water-a1",
  "configurePreset": "windows-debug",
  "filter": {
    "include": {
      "name": "^frazil_water_.*a1.*",
      "label": "^fast$"
    }
  }
}
```

具体 regex 必须根据实际 test name inventory 设计。

不要机械照抄该 regex。

验收要求：

```text
windows-debug-water-a1
```

实际选出的测试必须等价于：

```text
module = water-a1
AND
tier = fast
```

B1/B2/D1/Preview 同理。

如果现有 CTest preset 机制无法优雅表达 intersection：

可采用 helper labels：

```text
fast-water-a1
fast-water-b1
...
```

但优先保持：

```text
module / tier / kind
```

正交设计。

---

# 11. P0 任务 3：建立 Presets

至少建立：

```text
windows-debug-fast
windows-debug-core

windows-debug-water-common
windows-debug-water-a1
windows-debug-water-b1
windows-debug-water-b2
windows-debug-water-d1
windows-debug-preview

windows-debug-full
```

如已有 equivalent preset：

- 复用；
- 不重复制造新名称。

本轮不用新建完整 CI routing。

P0 只要求本地/CTest preset 可用。

---

# 12. P0 任务 4：修正 frazil_smoke dependency

重点检查：

```text
frazil_smoke
```

历史 Water CMake 曾有类似：

```cmake
add_dependencies(
    frazil_smoke
    ${water_test_targets}
    frazil_water_experiment_render
    ...
)
```

这使：

```text
smoke
```

语义被破坏。

本轮必须解除：

```text
frazil_smoke
→ all Water research targets
```

的强依赖。

完成后：

```text
build frazil_smoke
```

不得自动要求构建：

```text
D1 native
convergence
performance
listening
Preview
全部 Water test targets
```

---

# 13. 不要误删 build dependencies

解除 smoke dependency 时必须确认：

- 没有 target 因此无法独立 build；
- test executable 自己声明正确依赖；
- helper executable 的依赖仍通过 target_link_libraries / add_dependencies 正确表达；
- 不允许依赖 “以前 smoke 会顺带先 build 它”。

如发现隐式依赖：

必须修复真正的 target dependency，而不是重新让 smoke 依赖整个测试树。

---

# 14. P0 任务 5：Full Path 必须保持

P0 完成后必须仍能运行全部原有测试。

即：

```text
Full test set before P0
≈
Full test set after P0
```

允许：

- test order 变化；
- label 新增；
- preset 名称新增；
- build dependency 更合理。

禁止：

- test 静默消失；
- full matrix 被删除；
- listening/native/convergence 被取消注册；
- coverage 被减少。

P0 不是 test deletion phase。

---

# 15. P0 任务 6：生成 Coverage / Test Path Matrix 初版

新增：

```text
docs/testing/TEST_PATH_MATRIX.md
```

或者当前文档体系中等价路径。

至少记录：

| Test | Module | Tier | Kind | Fast | Full |
|---|---|---|---|---|---|
| frazil_unit | core | fast | unit | yes | yes |
| ... | ... | ... | ... | ... | ... |

P0 不需要完成“旧→拆分后新测试”的 P1 mapping。

只需要：

```text
当前 test
→ module
→ tier
→ kind
→ preset
```

---

# 16. P0 任务 7：同步文档

至少同步：

```text
tests/README.md
docs/TESTING.md
```

文档明确：

```text
Fast regression != Full validation
Module label != Module fast
Automated test != Listening acceptance
Research test PASS != Algorithm accepted
```

日常推荐入口改为：

```bash
ctest --preset windows-debug-fast
```

而不是无条件完整：

```bash
ctest
```

但仍必须保留 full validation 命令。

---

# 17. 本轮不做 CI Path Routing

当前 Agent plan 中：

```text
ci-core
ci-water
validation-full
```

属于后续 P5。

本轮禁止为了“顺手完善”直接大规模重写：

```text
.github/workflows/ci.yml
```

例外：

如果现有 workflow 因新增 preset name 发生必然失效，可以做**最小兼容修改**。

但不得在 P0 中实现：

```text
changed-files routing
module-selective jobs
research workflow migration
dependency install routing
```

这些必须留到后续独立 review。

---

# 18. Validation：P0 完成后必须运行

至少运行：

## A. Smoke build

```bash
cmake --build --preset <debug-preset> --target frazil_smoke
```

确认不会构建整个 Water research universe。

---

## B. Core

```bash
ctest --preset windows-debug-core --output-on-failure
```

---

## C. Fast

```bash
ctest --preset windows-debug-fast --output-on-failure
```

---

## D. Module fast

至少：

```bash
ctest --preset windows-debug-water-a1 --output-on-failure
ctest --preset windows-debug-water-b1 --output-on-failure
ctest --preset windows-debug-water-b2 --output-on-failure
ctest --preset windows-debug-water-d1 --output-on-failure
ctest --preset windows-debug-preview --output-on-failure
```

如果某模块在当前 HEAD 不存在：

- 明确写 N/A；
- 不伪造。

---

## E. Full Debug

```bash
ctest --preset windows-debug-full --output-on-failure
```

必须至少执行一次。

---

# 19. 必须记录 Timing

记录：

```text
Before P0 full Debug CTest:
After P0 full Debug CTest:

Core:
Fast:

Water Common Fast:
A1 Fast:
B1 Fast:
B2 Fast:
D1 Fast:
Preview Fast:
```

P0 的主要目标不是立即达到最终 30 秒。

因为本轮还没有拆大型 CLI / full matrix。

因此：

> **P0 允许 fast 仍然偏慢。**

但必须证明：

```text
分类正确
路径正确
slow/research 没有错误进入 fast
smoke 已解耦
```

真正的大幅提速属于 P1–P4。

---

# 20. 检查 Fast 中是否错误混入 Slow / Research

必须通过 CTest show-only 或等价方式检查。

例如：

```bash
ctest --preset windows-debug-fast -N
```

对每个 selected test 检查 label。

Fast 中不得出现：

```text
slow
research
listening
performance
```

除非存在特殊测试确有双重语义。

如存在：

- 必须解释原因；
- 不能默认接受。

---

# 21. 检查 Module Fast 中是否错误混入别的模块

例如：

```text
windows-debug-water-a1
```

正常应包含：

```text
core dependency（如果 preset 设计如此）
water-common dependency（如果明确需要）
water-a1 fast
```

不得无原因包含：

```text
water-b1
water-b2
water-d1
water-preview
```

共享测试必须明确标记：

```text
water-common
```

不要为了方便把所有 Water test 都标成多个 module。

---

# 22. 不允许修改声音行为

最终 diff review 必须确认：

```text
src/dsp
production AudioEngine
Host parameters
state
routing
Water acoustic candidate
```

没有非必要修改。

如果出现 DSP source diff：

暂停。

说明原因。

除非它是纯 build/test registration 必需，否则不要提交。

---

# 23. 不允许开始 P1

P0 完成后：

**STOP。**

不要继续拆：

```text
render_cli_test.py
droplet_b1_cli_test.py
flow_d1_cli_test.py
droplet_b1_tests.cpp
droplet_b2_tests.cpp
bubble_a1_tests.cpp
Preview test executable
```

即使 P0 完成得很顺利，也必须：

```text
commit P0
push P0
report
STOP
```

等待下一次 review。

---

# 24. 推荐 Commit 范围

建议 P0 commit message：

```text
refactor(testing): classify test paths and decouple smoke
```

如果修改量需要拆分：

```text
test(infra): classify existing CTest targets
build(testing): add scoped test presets
build(testing): decouple smoke from Water research targets
docs(testing): document P0 execution paths
```

不要把 P1 内容混入。

---

# 25. Agent 最终回复必须包含

严格按以下格式：

## 1. Execution baseline

```text
branch:
base/head:
authoritative implementation SHA:
plan commit:
```

说明是否仍使用 `ff75735`。

---

## 2. Current-state inventory

```text
CTest total:
Unlabelled before:
Unlabelled after:

Core:
Water Common:
A1:
B1:
B2:
D1:
Protect:
Preview:

Fast:
Slow:
Research:
```

---

## 3. Files changed

完整列出。

---

## 4. Labels

给出完整 label taxonomy 与 test mapping。

---

## 5. Presets

列出：

```text
core
fast
module fast
full
```

并说明如何保证：

```text
module ∩ fast
```

而不是：

```text
module alone
```

---

## 6. Smoke dependency

说明修改前：

```text
frazil_smoke
→ ...
```

修改后：

```text
frazil_smoke
→ ...
```

并说明实际 build 观察。

---

## 7. Test selection evidence

分别贴：

```text
windows-debug-fast -N
windows-debug-water-a1 -N
windows-debug-water-b1 -N
windows-debug-water-b2 -N
windows-debug-water-d1 -N
windows-debug-preview -N
```

的测试数量和测试名称摘要。

---

## 8. Timing

```text
before full:
after full:

core:
fast:
a1:
b1:
b2:
d1:
preview:
```

---

## 9. Full validation

说明：

```text
Full Debug:
PASS / FAIL
test count:
first failure if any:
```

---

## 10. Coverage preservation

明确：

```text
No test body deleted in P0
No expected coverage intentionally removed
```

如果不是，逐项说明。

---

## 11. Production impact

明确确认：

```text
DSP algorithm: unchanged
Water sound: unchanged
Host parameters: unchanged
State serialization: unchanged
Routing: unchanged
Latency: unchanged
Perceptual contract: unchanged
```

---

## 12. Remaining work

只列：

```text
P1
P2
P3
P4
P5
P6
```

不得声称已完成。

---

## 13. STOP statement

最终必须写：

```text
P0 COMPLETE / READY FOR REVIEW
P1 NOT STARTED
```

如果 P0 未全部完成：

```text
P0 INCOMPLETE
```

并说明 blocker。

---

# 26. 本轮验收 Definition of Done

只有以下项目全部满足，P0 才能通过：

- [ ] 从当前 authoritative Water implementation HEAD 开始，而不是旧 `main@3c95e472`。
- [ ] `593a08f` 方案文档已正确迁入实施分支。
- [ ] 当前所有 CTest 已 inventory。
- [ ] 当前所有 CTest 有清晰 module/tier/kind 分类。
- [ ] `fast` 与 `slow/research` 有明确隔离。
- [ ] module fast 真正实现 `module AND fast`。
- [ ] `frazil_smoke` 不再依赖整个 Water research target graph。
- [ ] Full CTest 注册没有因 P0 静默减少。
- [ ] Core/Fast/Module/Full preset 均可执行。
- [ ] `tests/README.md` 已同步。
- [ ] `docs/TESTING.md` 已同步。
- [ ] 有当前 `TEST_PATH_MATRIX`。
- [ ] 有 before/after timing。
- [ ] Full Debug 至少执行一次。
- [ ] 未修改 DSP 声音行为。
- [ ] 未推进 P1–P6。
- [ ] 最终提交后停止，等待 review。

---

# 27. 核心执行原则

本轮不要追求：

```text
一次性把整个测试体系重构完
```

本轮只解决：

```text
正确基线
+
正确分类
+
正确选择
+
正确 preset
+
smoke 解耦
+
full 保留
```

也就是先从：

```text
One giant undifferentiated test path
```

变成：

```text
Classified test graph
+
Selectable fast/module/full paths
```

随后再独立推进：

```text
P1 CLI split
P2 matrix split
P3 D1 research isolation
P4 Preview routing
P5 CI routing
P6 physical cleanup
```

---

# 28. 可直接给 Agent 的最终命令

```text
Review 结论：当前 593a08fe9262912fe2b25e13aae297bdd29b0d6c 仅提交了测试路径重构方案文档，未实施任何 CMake/CTest/CI/test code 重构。不要将其视为 P0 完成。

该方案分支基于 main@3c95e47212a03d43ccf06a2484d8f3861a4f6b33，而我们实际 review 的 Water research implementation HEAD 为 ff75735ffd9ec6a29d1ce76e277074e361dc902b；GitHub compare 显示方案分支与该 HEAD diverged，且方案分支落后 81 commits。因此禁止直接在 docs/test-path-systematic-refactor-agent-plan 分支上实施。

先 fetch 全部远端并重新确认当前 authoritative Water implementation HEAD。如果没有比 ff75735 更新的实现基线，则从 ff75735 创建 codex/refactor/test-paths-p0；如果存在更新 HEAD，则使用更新 HEAD，并在报告中记录差异。随后 cherry-pick 593a08f，仅迁入 Agent plan 文档。

本轮严格只推进 P0：
1. inventory 当前全部 CTest；
2. 给所有现有 test 添加正交的 module/tier/kind labels；
3. 建立 core/fast/water-common/A1/B1/B2/D1/Preview/full presets；
4. module fast 必须真正实现 module AND fast，禁止仅用 water-a1 等单一 module label 作为 fast 过滤；
5. CLI 可用多个 -L 实现 AND，例如 ctest -L fast -L water-a1；
6. Test Preset 使用 fast label + module test-name/filter intersection 或其他真正等价机制；
7. 移除 frazil_smoke 对整个 Water research test graph 的强制 dependency；
8. 如发现 target 依赖原本靠 smoke 间接构建，修复真实 target dependency，不恢复 giant smoke dependency；
9. 保持 Full Debug 注册和 coverage 不变；P0 不删除/拆分 test body；
10. 新增 TEST_PATH_MATRIX 初版；
11. 同步 tests/README.md 与 docs/TESTING.md；
12. 运行 smoke build、core、fast、各 module fast 和 full Debug；
13. 记录 before/after test inventory 与 timing；
14. 检查 fast 中不存在 slow/research/listening/performance 意外混入；
15. 检查 module fast 不会无原因执行其他 module；
16. 不修改 DSP、Water 声音、Host parameters、state、routing、latency 或 perceptual contract。

本轮禁止推进 P1–P6。不要拆 render_cli_test.py、droplet_b1_cli_test.py、flow_d1_cli_test.py；不要拆 B1/B2/A1 大型测试；不要实现 CI changed-path routing。

完成 P0 后提交并停止。最终报告必须包含 execution baseline、inventory、files changed、labels、presets、smoke dependency、test selection evidence、timing、full validation、coverage preservation、production impact 和 remaining work。

最终状态只能是：
P0 COMPLETE / READY FOR REVIEW
P1 NOT STARTED

若任一 P0 验收项未满足，则写：
P0 INCOMPLETE
并记录 blocker，不得继续下一阶段。
```
