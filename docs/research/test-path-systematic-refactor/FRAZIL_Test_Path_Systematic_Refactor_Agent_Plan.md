# FRAZIL 测试路径系统性重构 — Agent 执行方案

> 用途：可直接交给 Agent / Codex 执行  
> 项目：`jjjphens-dot/FRAZIL`  
> 主题：测试体系分模块、分层级、分执行路径重构  
> 核心目标：**保留有效测试资产，删除错误的默认测试路径，显著缩短日常开发反馈时间，同时不降低阶段验收能力。**

---

## 0. 执行前必须先做的事情

在修改任何代码前：

1. 拉取并检查仓库当前最新 HEAD。
2. 阅读以下文件并确认当前实现是否已发生变化：
   - `CMakeLists.txt`
   - `CMakePresets.json`
   - `.github/workflows/ci.yml`
   - `tests/README.md`
   - `docs/TESTING.md`
   - `experiments/water/SPIKE-W-DSP-001/CMakeLists.txt`
3. 枚举当前所有 CTest：
   ```bash
   ctest --test-dir <build-dir> -N
   ```
4. 不要假设本方案记录的 test 数量、耗时、HEAD 仍完全不变。
5. 若仓库已存在测试分组/label/preset 重构，则：
   - 先对照本方案目标；
   - 复用已有实现；
   - 不重复创建等价机制；
   - 在最终报告中明确说明与本方案相比已有实现覆盖了哪些内容。

本方案形成时观察到的基准状态：

- Water research 已经拥有 A1 / B1 / B2 / D1 / Protect / Preview 等相对独立模块。
- Hosted Windows Debug 一次完整 CTest 曾达到约 `42/42 PASS`、`484 s`。
- 测试耗时主要集中在少量研究、CLI、native 和 full-matrix 测试。
- 当前默认 CI 曾启用：
  - `FRAZIL_BUILD_WATER_EXPERIMENT=ON`
  - `FRAZIL_BUILD_WATER_PREVIEW=ON`
  - 然后运行完整 `ctest`
- 当前 Water CMake 中曾存在类似：
  ```cmake
  add_dependencies(frazil_smoke ${water_test_targets} ...)
  ```
  使 `smoke` 间接依赖大批 Water research targets。

以上仅作为重构背景，不得替代对当前 HEAD 的重新检查。

---

# 1. 任务目标

此次任务不是简单“删除测试”。

必须将 FRAZIL 的测试体系从：

```text
修改任意代码
    ↓
构建几乎全部测试目标
    ↓
运行几乎全部 CTest
    ↓
执行研究矩阵 / CLI / native / listening / evidence
    ↓
较长时间后才能获得基础回归反馈
```

重构为：

```text
修改代码
    │
    ├─ Core changed
    │     └─ Core Fast
    │
    ├─ Water Common changed
    │     └─ Core + Water Common Fast
    │
    ├─ A1 changed
    │     └─ Core + A1 Fast
    │
    ├─ B1/B2 changed
    │     └─ Core + B1/B2 Fast
    │
    ├─ D1 changed
    │     └─ Core + D1 Fast
    │
    └─ Preview changed
          └─ Preview Fast

阶段验收 / 明确手动触发
    ↓
Module Full
    ↓
Research Validation
    ↓
Native / Convergence / Performance / Listening / Evidence
```

最终要求：

1. **普通开发反馈快。**
2. **模块边界清晰。**
3. **研究验证不丢失。**
4. **阶段验收仍然可以完整执行。**
5. **默认 PR 不再重复执行无关模块。**
6. **Smoke 真正成为 smoke。**
7. **CTest、CMake Build target、CI 三个层面都完成解耦。**

---

# 2. 核心设计原则

## 2.1 Regression Test 与 Research Validation 必须分开

### Regression Test

回答：

> 当前修改有没有破坏已知合同、基础行为、模块不变量或接口？

特点：

- 快速；
- 可重复；
- 高频运行；
- 应作为普通 PR 的默认验证；
- 不生成大规模研究证据；
- 不承担候选算法是否“值得采用”的结论。

---

### Research Validation

回答：

> 当前候选是否有足够证据进入下一阶段？

包含但不限于：

- 大型 sample-rate / block / mode 参数矩阵；
- convergence；
- remediation；
- native numerical study；
- 性能多轮测量；
- listening pack；
- evidence CSV；
- provenance；
- binary hash；
- frozen-head validation；
- human review material。

特点：

- 低频运行；
- 允许耗时；
- 应由阶段 gate 或 workflow_dispatch 显式触发；
- 不应成为普通 PR 的默认回归路径。

---

# 3. 目标测试域

至少建立以下逻辑测试域。

| 测试域 | 职责 | 普通 PR 默认行为 |
|---|---|---|
| `core` | 产品 App / Plugin / State / Parameter / 基础 DSP | 始终运行 |
| `water-common` | Water 共用基础、mapping、event pool、公共 model | Water/shared 变更时 |
| `water-a1` | Bubble A1 | A1 或共享实现变化时 |
| `water-b1` | Droplet B1 | B1 或共享实现变化时 |
| `water-b2` | Droplet B2 | B2 或共享实现变化时 |
| `water-d1` | Flow D1 | D1 或共享实现变化时 |
| `water-protect` | Protect | Protect 或相关 routing 变化时 |
| `water-preview` | Preview / Debug UI / developer workflow | Preview/UI 变化时 |
| `research-validation` | convergence/native/listening/performance/evidence | 默认不运行 |

B1 与 B2 可以在 CI 层共用一个 `water-droplet` job，但 CTest label 建议仍保留独立模块身份。

---

# 4. CTest Label 体系

必须给现有测试增加明确的 Label。

建议至少使用以下 label：

```text
core
fast
slow

water-common
water-a1
water-b1
water-b2
water-d1
water-protect
water-preview

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

## 4.1 Label 规则

一个测试可以拥有多个 label。

示例：

```cmake
set_tests_properties(
    frazil_water_bubble_a1
    PROPERTIES LABELS "fast;water-a1;unit"
)
```

CLI smoke：

```cmake
set_tests_properties(
    frazil_water_bubble_a1_cli_smoke
    PROPERTIES LABELS "fast;water-a1;cli"
)
```

完整研究：

```cmake
set_tests_properties(
    frazil_water_flow_d1_convergence
    PROPERTIES LABELS "slow;water-d1;research"
)
```

Listening：

```cmake
set_tests_properties(
    frazil_water_protect_listening
    PROPERTIES LABELS "slow;water-protect;listening;research"
)
```

---

# 5. 不允许再使用一个“全量 CTest”作为日常唯一入口

至少应支持以下命令：

```bash
ctest -L core
ctest -L fast
ctest -L water-a1
ctest -L water-b1
ctest -L water-b2
ctest -L water-d1
ctest -L water-preview
```

同时支持完整验证：

```bash
ctest --preset windows-debug-full
```

或者：

```bash
ctest -L research
```

但普通开发文档中默认推荐命令必须改为 fast/module 级别，而不是直接完整 `ctest`。

---

# 6. Test Preset 重构

在 `CMakePresets.json` 中增加或等价实现以下测试入口。

建议：

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
windows-release-full
windows-asan-full
```

具体实现可使用 CTest preset 的 `filter.include.label`。

例如概念上：

```json
{
  "name": "windows-debug-fast",
  "configurePreset": "windows-debug",
  "filter": {
    "include": {
      "label": "fast"
    }
  },
  "output": {
    "outputOnFailure": true
  }
}
```

不要机械照抄；以项目当前 `CMakePresets.json` schema 和 CMake 版本为准。

---

# 7. Build Target 也必须解耦

只拆 CTest 不够。

当前项目历史上存在 build 阶段本身就构建大量 research targets 的问题。

必须避免：

```cmake
add_dependencies(frazil_smoke ${water_test_targets} ...)
```

这类设计。

## 7.1 Smoke 语义

`frazil_smoke` 必须只负责：

- 最基本 executable wiring；
- 最低限度构建/运行验证；
- 不间接要求构建全部 Water research targets；
- 不间接要求构建 performance / preview / native study。

---

## 7.2 建议新增 CMake profile

建议实现：

```text
FRAZIL_BUILD_WATER_EXPERIMENT
FRAZIL_BUILD_WATER_PREVIEW

FRAZIL_WATER_TEST_PROFILE
    OFF
    FAST
    FULL
```

语义：

### OFF

不构建 Water research test targets。

### FAST

仅构建：

- Water common；
- module functional/unit；
- module CLI smoke；
- 当前必要 Preview fast targets。

不得构建：

- full convergence；
- heavy native study；
- listening pack generation；
- performance benchmark；
- evidence publication helpers（除非自身代码被修改且需要独立测试）。

### FULL

构建所有 Water research validation 需要的 targets。

如果认为枚举型 cache variable 实现不适合当前 CMake，也可使用多个 BOOL option，但必须保持同样能力。

---

# 8. 建议的逻辑目录结构

不要在第一阶段立即大规模移动文件。

先通过 Label / Preset / Target 完成逻辑分组。

稳定后再逐步迁移为：

```text
tests/
├── core/
│   ├── unit/
│   ├── integration/
│   ├── contract/
│   └── render/
│
└── performance/
    └── product/

experiments/water/SPIKE-W-DSP-001/
├── tests/
│   ├── common/
│   ├── a1/
│   ├── b1/
│   ├── b2/
│   ├── d1/
│   ├── protect/
│   └── preview/
│
└── validation/
    ├── a1/
    ├── b1/
    ├── b2/
    ├── d1/
    ├── listening/
    ├── performance/
    └── evidence/
```

注意：

- 历史 evidence 文档中引用的旧路径不得静默重写成“过去就在新路径”。
- 历史证据保留历史事实。
- current docs 可说明测试资产已迁移。

---

# 9. 现有测试的具体处理要求

## 9.1 Core

保留：

- `frazil_smoke`
- `frazil_unit`
- `frazil_plugin_integration`
- `frazil_processor_property`
- `frazil_latency_contract`
- `frazil_render`
- `frazil_render_cli`

但要进一步标注：

```text
core + fast
core + property
core + integration
core + render
```

其中：

- `frazil_smoke` 不得构建整个 research tree；
- `frazil_render` 保留一个 canonical render smoke；
- `frazil_render_cli` 只承担产品 renderer 的 CLI contract，不承担 Water research 全矩阵。

---

## 9.2 `droplet_b1_tests.cpp`

当前该测试历史上同时覆盖：

- sample rate；
- block size；
- probability；
- radius；
- persistence；
- capacity；
- stereo；
- lifecycle；
- long-running time-domain behavior。

必须拆成逻辑上的：

```text
b1_fast
b1_matrix_full
```

### `b1_fast`

只保留：

- prepare contract；
- finite；
- reset；
- deterministic seed identity；
- canonical rate；
- canonical block；
- 一个 odd block；
- 一个 boundary capacity；
- 关键 lifecycle；
- 关键 stereo invariant。

### `b1_matrix_full`

保留完整：

- 多 sample-rate；
- 多 block；
- 多 parameter组合；
- capacity sweep；
- extended lifecycle。

`b1_matrix_full` 标记：

```text
slow;water-b1;research
```

不要进入普通 fast PR。

---

## 9.3 `droplet_b2_tests.cpp`

当前历史实现中 correctness 测试内混有 `std::chrono` callback timing。

必须拆开：

### correctness

只验证：

- finite；
- attack/onset；
- transient spacing；
- stereo behavior；
- radius/spread；
- lifecycle；
- processing correctness。

### performance

移动至：

```text
performance/
```

或现有 B2 performance executable。

**correctness test 中禁止继续以 wall-clock timing 作为普通 pass/fail 内容。**

---

## 9.4 `bubble_a1_tests.cpp`

拆为：

```text
a1_fast
a1_full
```

fast 只验证：

- physics/invariant；
- finite；
- seed identity；
- pool basic behavior；
- canonical render；
- 一组关键 capacity/cull case。

full 再运行：

- 大容量；
- 多 rate；
- 多参数；
- extended lifecycle；
- long matrix。

---

# 10. CLI 测试必须系统性瘦身

这是本次最重要的代码重构部分之一。

---

## 10.1 `render_cli_test.py`

当前历史实现承担了过多职责，包括：

- 3 sample rates；
- 多 block；
- 多 mode；
- A/B/D/C；
- modal；
- motion；
- protect；
- malformed config；
- structural invalid；
- composition；
- full audio comparison。

必须拆为：

```text
render_cli_smoke.py
render_cli_contract.py
validation/render_full_matrix.py
```

### `render_cli_smoke.py`

普通 fast：

- `--help` 或最基本调用；
- 一个 48k input；
- block=128；
- 一个 baseline；
- 一个 residual；
- 一个 invalid config；
- 一个 invalid mode；
- 输出 finite；
- 输出长度正确。

目标：

**秒级完成。**

### `render_cli_contract.py`

模块变化时：

- descriptor/schema；
- output overwrite contract；
- active/inactive config；
- 关键错误码；
- 一两个 odd/boundary block。

### `validation/render_full_matrix.py`

完整：

- 44.1/48/96；
- full block matrix；
- 多 mode；
- composition；
- extensive invalid corpus；
- cross-rate。

只属于 full validation。

---

# 11. `droplet_b1_cli_test.py` 必须拆分

当前历史实现中存在：

- descriptor；
- rate matrix；
- block matrix；
- stereo swap；
- multiple composition；
- invalid schema；
- study script；
- listening reviewer form。

必须拆成：

```text
b1_cli_smoke.py
b1_cli_contract.py
validation/b1_full_matrix.py
validation/b1_listening_pack.py
```

## 11.1 `b1_cli_smoke.py`

只允许：

- descriptor 能读取；
- 一个 canonical B1 render；
- 一个 partition boundary；
- 一个 invalid config；
- one composition sanity。

## 11.2 `b1_listening_pack.py`

负责：

- `droplet_b1_study.py`
- listening report；
- review form；
- human intake material。

不得注册到默认 CTest fast。

---

# 12. `flow_d1_cli_test.py` 必须拆分

拆成：

```text
d1_cli_smoke.py
d1_cli_contract.py
d1_native_oracle_validation.py
```

## fast smoke

只验证：

- descriptor；
- canonical D1 mode；
- one valid config；
- one invalid config；
- one partition boundary；
- finite output。

## contract

验证：

- mode isolation；
- explicit defaults；
- disabled identity；
- key schema boundaries。

## native oracle validation

负责：

- source probe；
- native comparison；
- multi-rate；
- transfer validation。

该部分标记：

```text
slow;water-d1;native;research
```

---

# 13. D1 大型研究测试必须退出普通 PR

以下类型的测试默认不进入普通 PR：

```text
flow_d1_latency_native
flow_d1_remediation
flow_d1_convergence
```

处理：

```text
water-d1-full
research-validation
```

只有以下情况触发：

- D1 candidate 进入阶段验收；
- D1 numerical implementation 有实质变化；
- 明确手动 dispatch；
- release / milestone gate。

普通 D1 PR 只运行：

```text
d1_fast
d1_cli_smoke
d1_contract
```

---

# 14. Listening 不属于普通 CTest Gate

以下内容应移出 fast：

- protect listening；
- B1 listening pack；
- Water review pack；
- human blank forms；
- subjective comparison package。

可以保留 CTest registration，但必须：

```text
LABELS "slow;listening;research"
```

更推荐提供明确命令：

```bash
cmake --build ... --target frazil_water_listening_pack
```

或者 Python runner：

```bash
python .../validation/listening/run.py
```

普通 PR 不应自动生成 listening pack。

---

# 15. Performance 不属于 correctness

所有 performance 测试必须做到：

1. 不和 correctness assertions 混写；
2. 不在普通 fast CTest 中执行；
3. 不使用单次 wall-clock 抖动导致普通 PR fail；
4. 保留：
   - workload；
   - provenance；
   - fixed repetitions；
   - allocation；
   - mean/P95/P99；
   - callback deadline。
5. 通过：
   ```text
   performance
   slow
   research
   ```
   label 或独立 target 执行。

---

# 16. Preview 测试重构

当前 Preview 历史上将许多文件编译成一个大型：

```text
frazil_water_preview_tests
```

不要求立刻创建十几个 executable。

推荐优先保持单 executable，通过 group/filter 选择子套件，例如：

```bash
frazil_water_preview_tests --group core
frazil_water_preview_tests --group session
frazil_water_preview_tests --group diagnostics
frazil_water_preview_tests --group audition
frazil_water_preview_tests --group parameters
```

然后注册成多个 CTest：

```text
frazil_water_preview_core
frazil_water_preview_session
frazil_water_preview_diagnostics
frazil_water_preview_audition
frazil_water_preview_parameters
```

这样：

- 编译仍然共享；
- CTest 可以按 label 路由；
- failure 定位更清楚；
- 不需要因为一个小改动执行所有 Preview regression。

如果现有 test harness 不适合 filter，可保留一个 executable，但第一阶段至少为其建立独立 `water-preview` label。

---

# 17. Testdata 重构

当前 canonical `testdata/input` 不应因为“测试太多”而粗暴删除。

测试数据本身不是主要瓶颈。

必须拆执行路径：

## 普通 PR

运行：

```bash
python tools/verify_testdata.py
```

只检查：

- manifest；
- SHA；
- WAV metadata；
- corpus 完整性；
- 文件集合一致性。

## 只有以下文件变化时

```text
tools/generate_testdata.py
tools/signal_generators.py
tools/verify_testdata.py
tools/test_testdata.py
testdata/**
```

才运行：

```bash
python tools/test_testdata.py
```

包括：

- temporary regeneration；
- multi-rate generation；
- semantic validation；
- byte reproducibility。

普通 DSP/Water 参数调整不需要重新生成整个 canonical corpus。

---

# 18. Python DSP dependency 安装也要按需

当前普通 CI 历史上会安装完整 DSP experiment dependencies。

目标：

### Core-only PR

不需要安装 NumPy / SciPy / SoundFile / Matplotlib 等 research dependency，除非 core test 实际依赖。

### Water fast

只安装 fast test 真正需要的 dependency。

### Research validation

才安装：

- NumPy；
- SciPy；
- soundfile；
- matplotlib；
- 其他研究依赖。

避免仅修改产品参数/文档也支付 research Python environment 初始化成本。

---

# 19. CI 架构

最终建议至少有三条逻辑路径。

---

## 19.1 `ci-core`

普通代码 PR 基础 gate。

包含：

```text
checkout
portability
必要 build-tool checks
restore JUCE
configure core/fast
build core targets
ctest core/fast
```

目标：

- 快；
- 稳定；
- 始终运行。

---

## 19.2 `ci-water`

仅 Water/shared DSP 改动触发。

根据 path filter / changed-files 决定模块。

示意：

```text
A1 files changed
    → core + water-common + water-a1

B1 files changed
    → core + water-common + water-b1

B2 files changed
    → core + water-common + water-b2

D1 files changed
    → core + water-common + water-d1

Preview files changed
    → water-preview

shared Water research headers changed
    → water-common + impacted modules
```

不要默认：

```text
A1 changed → B1+B2+D1+Preview 全跑
```

---

## 19.3 `validation-full`

仅：

- `workflow_dispatch`
- milestone；
- stage gate；
- important merge；
- release；
- explicit validation branch。

运行：

```text
Debug full
Release full
ASAN full
native studies
convergence
remediation
performance
evidence
listening pack（如本阶段需要）
```

该 workflow 可以耗时，不要求与 fast 一样短。

---

# 20. 路径触发建议

CI path filter 可以参考：

```text
src/app/**
src/dsp/**
src/plugin/**
tests/**
```

→ `core`

```text
experiments/water/SPIKE-W-DSP-001/dsp/*A1*
experiments/water/SPIKE-W-DSP-001/tests/*bubble_a1*
```

→ `water-a1`

```text
*DropletB1*
*b1*
```

→ `water-b1`

```text
*DropletB2*
*b2*
```

→ `water-b2`

```text
*FlowD1*
*flow_d1*
```

→ `water-d1`

```text
preview/**
tests/preview_*
src/ui/**
```

→ `water-preview`

但不要只依赖文件名字符串猜测。

Agent 必须先查看当前模块文件边界，再制作 path mapping。

共享文件，例如：

```text
ResearchConfig
common descriptors
event pool
shared renderer
ReadConfig
```

应触发所有受影响 Water fast 模块，不能错误只归为一个模块。

---

# 21. Representative Matrix 规则

Fast tests 不再运行完整 Cartesian Product。

## Canonical

```text
48 kHz
128 samples
```

## Odd boundary

```text
48 kHz
257 samples
```

## Large representative

```text
96 kHz
1024 samples
```

## Cross-rate representative

```text
44.1 kHz
128 samples
```

Fast 中具体保留几组按模块需要决定。

原则：

- 每个维度至少有 canonical；
- 关键边界至少一个；
- 不在 fast 中进行所有维度笛卡尔积。

完整：

```text
44.1 / 48 / 96
×
1 / 7 / 32 / 64 / 128 / 256 / 257 / 512 / 1024
×
all config combinations
```

仅属于 full validation。

---

# 22. 不允许为了提速牺牲以下合同

重构不得删除以下能力：

- finite output；
- reset；
- prepare/reprepare；
- deterministic seed identity；
- zero/short/odd block；
- stereo isolation；
- inactive module identity；
- parameter range rejection；
- strict descriptor/schema；
- state restore；
- Host parameter mapping；
- allocation contract；
- known latency contract；
-关键 A1/B1/B2/D1 mathematical invariants；
- regression against previously accepted behavior where applicable。

区别只是：

**这些合同由合适的模块路径负责，而不是每条路径重复验证。**

---

# 23. 第一阶段禁止做的事情

Phase 1 只做分类/基础解耦时：

禁止：

- 删除大量 test case；
- 修改 DSP algorithm；
- 修改声音参数；
- 修改 Water candidate selection；
- 修改 accepted research conclusion；
- 修改 Host parameter contract；
- 修改 state serialization；
- 修改 production latency；
- 重新定义 Water perceptual contract；
- 改变已有 evidence 的历史结论。

测试重构必须是 engineering infrastructure change。

---

# 24. 推荐实施阶段

---

## Phase 1 — Inventory + Labels + Presets

目标：

**不改变测试逻辑，只改变分类与执行入口。**

工作：

1. 枚举全部 test。
2. 记录当前 label=none 状态。
3. 为每个 test 添加：
   - module；
   - tier；
   - kind。
4. 增加 fast/core/module/full presets。
5. 删除 `frazil_smoke` 对所有 research target 的强制 dependency。
6. 创建 meta targets（如必要）：
   ```text
   frazil_core_tests
   frazil_water_fast_tests
   frazil_water_full_tests
   ```
7. 保证 full 仍能运行旧的全部 tests。

验收：

- Full 结果与重构前一致；
- Fast 已经可以独立执行；
- Smoke 不再构建整个 research universe；
- 不改测试断言。

---

## Phase 2 — 拆 CLI

优先：

```text
render_cli_test.py
droplet_b1_cli_test.py
flow_d1_cli_test.py
```

拆成：

```text
smoke
contract
full validation
listening/native
```

验收：

- 原测试覆盖点都有去向；
- 不允许静默丢失 coverage；
- 建 coverage mapping 表。

---

## Phase 3 — 拆 B1/B2/A1 大型 C++ 测试

目标：

```text
fast invariant
full matrix
performance
```

分离。

重点：

- B2 correctness 去除 timing；
- B1 大矩阵退出 fast；
- A1 capacity/full matrix 退出 fast。

---

## Phase 4 — D1 research isolation

移动逻辑：

```text
latency_native
remediation
convergence
```

到 D1 full/research。

普通 D1 fast 只保留：

- functional；
- schema；
- canonical numerical invariants；
- small boundary set。

---

## Phase 5 — Preview routing

为 Preview 添加 group/filter 或拆注册入口。

达到：

```text
preview-core
preview-session
preview-diagnostics
preview-audition
preview-parameters
```

至少做到 logical routing。

---

## Phase 6 — CI Routing

建立：

```text
ci-core
ci-water
validation-full
```

按 changed path/module 运行。

---

## Phase 7 — Physical directory cleanup

在逻辑体系稳定后：

- 移动 tests；
- 更新 CMake；
- 更新 docs；
- 保留历史 evidence 的事实语义。

---

# 25. 量化性能目标

基于历史观测，当前完整 Hosted CTest 曾约：

```text
484 s
```

且大部分耗时来自少数 slow research tests。

重构目标：

## Core-only

```text
≤ 5~10 s CTest
```

如果因平台原因略高可接受，但必须给出实际测量。

## Fast

```text
≤ 30~40 s CTest
```

理想目标。

## 单 Water module fast

```text
≤ 30 s
```

## Full

不要求压缩到几十秒。

允许：

```text
数分钟 ~ 十几分钟
```

因为它是明确的阶段验证。

注意：

这些是工程目标，不应通过删除关键断言“刷时间”。

---

# 26. CI 日常时间目标

普通 PR 不应再默认：

```text
Install all research deps
Build all Water research
Build all Preview
Run all 40+ tests
Run convergence/native/listening
```

目标：

```text
普通 core PR：
  core fast

普通 A1 PR：
  core fast + A1 fast

普通 B1 PR：
  core fast + B1 fast

普通 D1 PR：
  core fast + D1 fast
```

Full workflow 独立。

---

# 27. 文档同步要求

修改后同步：

```text
tests/README.md
docs/TESTING.md
tools/README.md（如命令变化）
experiments/water/SPIKE-W-DSP-001/README.md（若存在相关测试说明）
docs/PROJECT_STATUS.md（仅记录实际状态）
```

文档中要明确：

```text
Fast regression != Full validation
Automated regression != Listening acceptance
Full numerical pass != Human sound acceptance
```

不要把：

```text
CTest PASS
```

描述成：

```text
Water algorithm accepted
```

---

# 28. README 建议最终给开发者的入口

建议形成类似：

```bash
# 日常基础开发
ctest --preset windows-debug-fast

# Core
ctest --preset windows-debug-core

# Water A1
ctest --preset windows-debug-water-a1

# Water B1
ctest --preset windows-debug-water-b1

# Water B2
ctest --preset windows-debug-water-b2

# Water D1
ctest --preset windows-debug-water-d1

# Preview
ctest --preset windows-debug-preview

# 阶段完整验证
ctest --preset windows-debug-full
ctest --preset windows-release-full
ctest --preset windows-asan-full
```

实际名称可按当前项目风格调整，但语义必须保留。

---

# 29. Coverage Mapping 必须提交

本次重构必须新增一份 coverage mapping 文档，例如：

```text
docs/testing/TEST_PATH_MATRIX.md
```

内容至少包括：

| Old Test | Coverage | New Fast Test | New Full Test | Validation |
|---|---|---|---|---|
| droplet_b1_tests | prepare/finite/block/capacity/... | b1_fast | b1_matrix_full | - |
| droplet_b1_cli_test | schema/render/study/listening | b1_cli_smoke | b1_cli_full | b1_listening |
| flow_d1_cli_test | schema/partition/native | d1_cli_smoke | d1_cli_contract | d1_native |
| ... | ... | ... | ... | ... |

作用：

防止重构时因“提速”意外丢失测试覆盖。

---

# 30. 最终验收要求

Agent 完成后必须给出以下证据。

## 30.1 Test Inventory

列出：

```text
总 CTest 数量
fast 数量
core 数量
各 Water module 数量
slow/research 数量
```

---

## 30.2 Build Inventory

说明：

- fast profile 构建哪些 targets；
- full profile 构建哪些 targets；
- smoke 是否仍依赖 research targets。

---

## 30.3 Timing

至少实际测量：

```text
core test time
fast test time
water-a1 fast
water-b1 fast
water-b2 fast
water-d1 fast
preview fast
full test time
```

若 CI 可运行，则给 Hosted 数据。

---

## 30.4 Coverage Preservation

给出旧测试 → 新测试路径 mapping。

所有删除的 case 必须说明：

```text
why redundant
which remaining case covers same property
```

不允许简单写：

```text
removed because slow
```

---

## 30.5 Full Validation

最终至少执行一次：

```text
Debug Full
```

如果当前阶段/环境允许：

```text
Release Full
ASAN Full
```

如有已知失败，保留第一失败证据，不要 retry-until-pass。

---

# 31. Agent 最终报告格式

最终回答必须包含：

```text
1. Current-state audit
2. Files changed
3. New test architecture
4. CTest labels
5. Test presets
6. Build profiles
7. CLI split
8. Research validation isolation
9. CI routing
10. Coverage mapping
11. Timing before/after
12. Tests executed
13. Remaining risks
14. Recommended next step
```

---

# 32. 非目标

本任务不负责：

- 修改 Water 声音；
- 选择 A1/B1/B2/D1 winner；
- 改算法；
- 修改物理模型；
- 修改 perceptual target；
- 修改用户 UI；
- 修改 Host automation contract；
- 修改 production routing；
- 修改已接受的 Water research conclusion。

如果测试重构暴露 production bug：

1. 记录 finding；
2. 提供最小复现；
3. 不借测试重构顺手大改 DSP；
4. 如必须修复才能完成重构，应单独标记修改并说明原因。

---

# 33. 最终核心原则

本次重构最重要的判断标准：

> **不是减少测试，而是减少无关测试被重复执行。**

FRAZIL 应从：

```text
One giant validation path
```

转变为：

```text
Fast regression
    +
Module regression
    +
Explicit full validation
```

必须做到：

```text
保留完整验证能力
+
提高日常开发速度
+
清晰模块责任
+
降低测试重复
+
保持 research evidence 严谨性
```

最终理想结构：

```text
                ┌────────── Core Fast ──────────┐
Change ─────────┼──────── Water Module Fast ─────┤
                └────── Preview Fast (if needed)┘
                              │
                              ▼
                        Normal PR Gate


Explicit Stage Gate
        │
        ▼
 Module Full Validation
        │
        ▼
 Debug / Release / ASAN
        │
        ▼
 Native / Convergence
        │
        ▼
 Performance / Listening
        │
        ▼
 Evidence Publication
```

这才是 FRAZIL 当前阶段应采用的测试体系。

---

# 34. Agent 执行优先级

严格按以下优先级推进：

```text
P0
- inventory
- labels
- presets
- smoke dependency 解耦
- 保证 full 行为不变

P1
- render CLI 拆分
- B1 CLI 拆分
- D1 CLI 拆分

P2
- B1/A1/B2 large matrix 拆 fast/full
- performance 与 correctness 分离

P3
- D1 native/convergence/remediation 移出普通 PR

P4
- Preview logical groups

P5
- CI path/module routing

P6
- physical directory cleanup
```

在 P0 未完成并验证前，不要开始大规模删除或移动测试。

---

# 35. Definition of Done

只有同时满足以下条件，才能认为此次测试重构完成：

- [ ] `frazil_smoke` 不再隐式构建全部 Water research。
- [ ] 所有 CTest 有明确模块/层级分类。
- [ ] 存在 fast/core/module/full 运行入口。
- [ ] 默认 PR 不再执行 listening/performance/convergence/evidence 全路径。
- [ ] B1 CLI 不再生成 listening study 作为普通 smoke。
- [ ] D1 native/convergence/remediation 不再属于普通 fast。
- [ ] B2 correctness 中不再混入 performance gate。
- [ ] Full validation 仍能覆盖原有重要合同。
- [ ] 有旧→新 coverage mapping。
- [ ] `tests/README.md` 与 `docs/TESTING.md` 同步。
- [ ] 提供重构前/后实际耗时。
- [ ] 无 DSP 声音行为改变。
- [ ] 无 Host/state/parameter contract 意外改变。
- [ ] Full Debug 至少执行一次并记录结果。
- [ ] 所有未解决失败被明确保留，不以重复运行掩盖。

---

**执行原则：先解耦路径，再优化测试；先保证覆盖不丢，再追求速度。**
