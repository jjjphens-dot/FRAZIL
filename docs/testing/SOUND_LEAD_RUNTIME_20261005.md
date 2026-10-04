# Sound Lead 本机验收耗时调查（2026-10-05）

本记录回答：协作者的极长验收在 Sound Lead 本机是否出现，应该改哪一层。
这是一次有界工程调查，不构成声音、Host、Water production 或阶段退出验收。

## 基线和证据身份

- 先执行 `git fetch origin --prune`，并再次用 `git ls-remote` 核对。
- 云端最新实现为 [PR #44](https://github.com/jjjphens-dot/FRAZIL/pull/44)
  的 `392fce08d093fdc2acb37177fbb0ba314339f089`，基于 Water `ff75735`。
- 时间最新的 [文档提交 39bcb0f](https://github.com/jjjphens-dot/FRAZIL/blob/39bcb0f/docs/research/test-path-systematic-refactor/FRAZIL_PhaseA_Closeout_PhaseB_Validation_Separation_Agent_Command.md)
  是从 main 分出的任务说明，明确引用上述实现；它自身不包含最新测试重构。
- 云端 main 仍为 `3c95e47212a03d43ccf06a2484d8f3861a4f6b33`。不能以 main 的小型
  测试集或旧 Preview 的 38 项测试代表当前 Full67/Fast42。
- 复用现有干净研究工作树，在 `fix/local-validation-runtime` 分支工作；先构建最新
  native 基线。下文 Full、Fast、A1 测量的 native 源码、CTest 注册、benchmark/adaptor
  均来自 `392fce0`，未修改。本轮改动仅为工具、workflow 和文档，按用户追加要求提交到
  GitHub 新分支；不表示 PR #44 或 main 已更新/合入。

机器：Windows 11 build26100，Core Ultra 9 275HX，约31.4 GiB可见物理内存；
MSVC19.44、CMake4.4.3、Ninja1.13.2、JUCE9.0.1。使用已有 Python3.13.7 环境，
NumPy2.5.3、SciPy1.18.1、soundfile0.14.0、matplotlib3.11.1；未安装或升级依赖。
两项 research configure option 均开启，CMake 显式绑定装有这些依赖的解释器。
所有 configure/build/test 串行，safe wrapper 默认6 jobs，保留内存检查；CTest 单进程、
每项调查 deadline600秒。未通过重复运行直到通过来覆盖第一次结果。

原始命令、环境初始化脚本、输出、JUnit、首轮 LastTest.log、GitHub jobs JSON 和计时
保存在主工作树 ignored `build/cloud-validation-20261005/`。其中本机路径只留在本地。
去除机器路径后的[逐项 CTest 结果](evidence/SOUND_LEAD_RUNTIME_20261005.csv)随报告提交，
由上述4份JUnit直接提取；111行是4次运行结果，不是111个不同测试。
已有验收合同中的 testdata reproducibility 检查照常运行；未新增 checksum 留证步骤。

## 本机实测

单次 wall time；不作统计性能预算。配置/构建列为包含 VS shell 和安全 wrapper 的外层计时，
CTest 列为自身 real time。Debug 复用了旧 Preview 构建缓存；Release/ASAN 创建新构建目录，
仅构建 A1 canonical target。三者构建时间不能当作同范围的冷热构建速度比较。

| 步骤 | 耗时 | 结果 / 范围 |
| --- | ---: | --- |
| Debug configure | 4.36 s | PASS，experiment/preview ON |
| Debug Full safe build | 18.10 s | PASS，增量构建 |
| Debug Full | 393.13 s | 67/67 PASS，无 Python 崩溃或超时 |
| Debug Fast | 33.00 s | 42/42 PASS，现有独立 Fast 入口 |
| Release configure | 18.62 s | PASS，新目录 |
| Release A1 leaf safe build | 4.09 s | PASS，只构建 A1 及其依赖 |
| Release A1 canonical CTest | 11.37 s | 1/1 PASS；单项 body11.36 s |
| ASAN configure | 21.41 s | PASS，新目录 |
| ASAN A1 leaf safe build | 4.57 s | PASS，只构建 A1 及其依赖 |
| ASAN A1 canonical CTest | 214.73 s | 1/1 PASS；单项 body214.71 s，不推算完整ASAN时间 |

Debug Full 中耗时最大的项目：

| 项目 | 时间 |
| --- | ---: |
| A1 performance | 82.00 s |
| B1 performance | 39.59 s |
| 完整 renderer CLI 矩阵 | 29.41 s |
| 完整 B1 CLI 矩阵 | 22.80 s |
| Legacy performance | 22.53 s |
| D1 latency-native 验证 | 21.95 s |
| D1 native oracle | 18.57 s |
| D1 remediation / events-only parity | 18.34 s |
| B1 完整 property 矩阵 | 16.31 s |
| testdata regeneration | 13.45 s |

7 项 performance 合计150.55秒，占 Full约38.3%。去掉这些条目的算术余量仍约242.58秒，
**不是已执行的 correctness 子集，也不等于可以直接删掉 performance**。完整 render、研究、
listening-pack mechanics 和素材再生成同样有成本。Fast 本次约为 Full 的1/11.9；这是两种
既有范围的比较，不是本次补丁把同样67项加速了11.9倍。Fast 不能替代独有的完整矩阵断言。

A1 比较保留同一30格矩阵（3 rates × 5 capacities × 2 profiles），每格500 warmup +
3000 measured blocks，block128，共13,440,000 sample frames。Debug81.9988秒对
Release11.36秒，约7.2倍；ASAN214.71秒，约为Release的18.9倍、Debug的2.6倍。
这在本机复现了长计时循环被ASAN显著放大的成本，而非进程卡死或崩溃。
适配器仍检查完整 case identities、schema、finite observations；
这些 wall times 衡量验收成本，不证明任何实时 deadline 或声音质量。

实际执行的主要命令（Python/输出路径的机器部分用占位符表示；逐条串行）：

```powershell
cmake --preset windows-debug -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE:FILEPATH=<research-python>
python tools/build_safe.py --preset windows-debug-full
ctest --preset windows-debug-full --parallel 1 --timeout 600 --output-on-failure --output-junit <evidence>/debug-full-first.xml
ctest --preset windows-debug-fast --parallel 1 --timeout 600 --output-on-failure --output-junit <evidence>/debug-fast.xml
cmake --preset windows-release -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE:FILEPATH=<research-python>
python tools/build_safe.py --preset windows-release-full --target frazil_water_bubble_a1_performance
ctest --preset windows-release-full -R '^frazil_water_a1_performance$' --parallel 1 --timeout 600 --output-on-failure --output-junit <evidence>/release-a1.xml
cmake --preset windows-asan -DFRAZIL_BUILD_WATER_EXPERIMENT=ON -DFRAZIL_BUILD_WATER_PREVIEW=ON -DPython3_EXECUTABLE:FILEPATH=<research-python>
python tools/build_safe.py --preset windows-asan-full --target frazil_water_bubble_a1_performance
ctest --preset windows-asan-full -R '^frazil_water_a1_performance$' --parallel 1 --timeout 600 --output-on-failure --output-junit <evidence>/asan-a1.xml
```

每次 configure 前已先运行同 preset 的 safe wrapper `--check-only`；同一初始化的 MSVC
shell 使用 UTF-8 codepage65001。基线 Full 首次日志已复制保存，再执行 Fast，未覆盖原始证据。

## 与协作者和 Hosted 历史分开解释

[旧实现 07bf7b4 的执行记录](TEST_PATH_EXECUTION.md)保留协作者本地首次失败：
Debug443.66秒、Release86.39秒、ASAN1039.08秒，涉及 Python write/access violation、
TypeError 等。不同机器、编译器、Python、代码修订且包含失败，不能直接按秒数计算提速。
本机最新 Debug Full67/67 通过、testdata 再生成13.45秒通过，只说明此次未复现；
不能据此宣布升级 Python3.13 修复了对方环境，也不关闭历史 finding。

通过 `gh api .../actions/runs/37143829885/jobs` 核对
[历史 Hosted run](https://github.com/jjjphens-dot/FRAZIL/actions/runs/37143829885)，
从首个 job 开始到最后结束为119分34秒。ASAN job 单独53分15秒；这不是纯测试时间。

| 配置 | Full build step | Full test step | 自动 Python diagnostic step |
| --- | ---: | ---: | ---: |
| Debug | 525 s | 790 s | 265 s |
| Release | 1275 s | 174 s | 185 s |
| ASAN | 608 s | 2013 s | 438 s |

以上为 GitHub step wall time，另含配置、依赖、Core/module jobs、上传等；不要与 CTest
自身计时混用。旧自动诊断总计888秒（14分48秒）。**最新 Phase A 已取消 Full 后自动诊断，
并改成一次显式请求只跑一个配置**；这些节省不能重复记为本次新修复。
本轮未完整重放两小时历史链，也未对 Hosted/协作者机器做硬件归因。

## 本次有界改动

1. `tools/build_safe.py` 为7个现有 canonical benchmark executable 开放严格 allowlist 的
   `--target`，保留全部内存/并发限制，并打印实际 target。真实需求是比较同一个慢项目时
   无需先构建整套插件、Preview 和其它实验。没有新增 benchmark、缩短循环或改阈值。
   [可复现命令](TEST_PATH_MATRIX.md#bounded-runtime-investigation)复用原 CTest/adaptor。
2. `.github/workflows/validation.yml` 的 concurrency 从唯一的 `github.run_id` 改为稳定的
   `github.ref`。同 ref 显式运行串行，保留 `cancel-in-progress: false`；每日 PR 组独立。
   这防止同分支重复并发，不缩短单个本地 test，也不保证跨分支全局互斥。
   GitHub 默认只保留一个 pending，后来的请求可替换旧 pending；不是可靠任务队列。
   [官方语义](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#concurrency)。
3. 修正文档与当前实现的冲突：ENVIRONMENT/TESTING 仍描述旧 `ci.yml` 的
   `flow_d1_latency=true`、`flow_d1_convergence=true` 入口和研究前跑 Debug/Full，
   已同步为 `validation.yml` 单 study、选定 helper 的实际路径；未重写历史 evidence。

2026-10-05 实查默认分支为 main，其 tree 无 `validation.yml`，Actions API 只列现有 FRAZIL CI。
根据 [workflow_dispatch 条件](https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows#workflow_dispatch)，
新工作流的 Hosted activation 仍 **BLOCKED / NOT RUN**。本地回归、YAML/文本检查不是 Hosted
执行证据。本轮只按用户要求提交/推送新分支，没有 dispatch、merge、修改分支保护或联系协作者。

## 下一步优先级

1. 日常反馈采用现有 planner → Core/受影响 module Fast；保留原始失败输出，只在失败类别和
   hypothesis 明确时调用一次有界诊断。不要把每次 Final Validation 自动扩成三配置 Full。
2. 按已有 Phase B/C 计划分离 correctness、memory-safety、Release performance。
   先逐项审计独有断言，尤其 B2 performance 内的 sine/noise finite 检查、product denormal、
   prepare/lifecycle 和完整 case validation；迁入对应 correctness/memory 入口并建立映射，
   再让长 timing loops 只进入显式 Release 测量。不能用 `-LE performance` 就声称等价覆盖。
3. Full 仍作为保留全部资产的显式入口；研究、listening pack、素材 regeneration 按各自变更
   或验收目标运行。研究单独耗时不应被算成日常 debug 的固定前置成本。
4. Phase D 再处理 CI module union，避免多个依赖 module 重复构建/执行相同 shared entries。
   Hosted 冷构建、缓存命中和配置时间单独留证；本轮不从本机增量构建推算云端节省。
5. benchmark adapter 当前捕获全部 stdout，完成才显示；长时间安静不等于死锁。可后续加入
   有界进度/首个失败保留和超时策略，避免计时内 I/O；本轮仅从外层使用600秒deadline，
   不更改 canonical measurement。

## 验证、质量和文档审查

- Contract Review：读取实际392fce0的测试路径、scheduler、safe-build、研究/性能入口和
  latest39bcb0f任务边界。命中 Build/CI 文档同步范围；无生产 DSP、参数、state、routing、
  latency、随机合同、正式性能预算或 milestone gate 变化，不触发产品 Joint Gate。
- Implementation：仅上述窄工具、workflow、直接相关文档。新增本报告及逐项CSV由跨机器耗时复现需求驱动；
  无新 production abstraction、依赖、preset 或测试矩阵。
- Functional Validation：Full67、Fast42 和 A1跨配置结果如上；安全工具回归与 workflow6项
  首次通过，包含 benchmark target 下的低内存拒绝、任意 target拒绝、同ref分组检查。
- Code Quality Review：独立复查 diff、allowlist边界、内存失败先于 build、显式 job上限、
  测试原始矩阵未动、无后台重跑、取消组作用域及日志 target可读性。
- Comment & Documentation Pass：同步 AGENTS、CODING_PLAN、ENVIRONMENT、GITHUB_WORKFLOW、
  TESTING、PROJECT_STATUS、tools README、test-path matrix/execution 和本报告。
  Reviewed/no update：README（现有基础命令有效）、CODE_STANDARDS（质量/实时规范未变）、
  DOCUMENT_GOVERNANCE（现有 Build/CI gate适用）、MODULE_INDEX（模块职责及Phase B-D状态未变）、
  CMake/presets（注册/资产不变）。跨文档统一为本地候选已实现、Hosted未激活、Phase B-D未完成。
- Final Validation：下表全部PASS。随后只收口报告和逐项CSV；没有再次重复完整native矩阵。

| 实际命令 | 最终结果 |
| --- | --- |
| `python tools/test_build_safe.py` | PASS，含新leaf入口和mock低内存/未知内存拒绝 |
| `python tools/test_validation_workflow.py` | 6/6 PASS |
| `python tools/test_plan_validation.py` | 12/12 PASS |
| `python tools/test_test_impact.py` | 15/15 PASS |
| `python tools/test_python_test_ab.py` | 5/5 PASS；合成timeout/cancel测试，不是实际Python环境A/B |
| `python tools/test_check_test_paths.py` | 15/15 PASS |
| `python tools/check_test_paths.py --build-closure` | PASS；实际CTest/Ninja元数据，Full67/Fast42保留 |
| `python tools/check_portability.py` | PASS，含新报告/CSV，不提交本机路径 |
| `python tools/check_markdown_links.py` | PASS |
| `python -m py_compile tools/build_safe.py tools/test_build_safe.py tools/test_validation_workflow.py` | PASS |
| `git diff --cached --check` | PASS |

对实际工具/workflow路径运行现有 `plan_validation.py`，其保守结果为tooling + Core/all-module
Fast，不要求performance/diagnostics；既有Full67/Fast42和最终工具/closure检查覆盖本次调查
所需范围。工作流只做本地回归与文本检查，未声称Hosted执行或调度实测通过。

未执行：完整 Release/ASAN Full、最新 Hosted run、协作者原机重测、真实 Python A/B（本轮没有
相关失败）、pluginval/DAW/音频设备/主观听测。没有产品或音频行为改动，旧 pluginval/DAW 结果
不贴成当前提交新验收。未宣称 Full67 等于完整 memory-safety 或 Release acceptance。
