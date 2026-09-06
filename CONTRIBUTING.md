# 参与 FRAZIL 开发

开始前请阅读 `AGENTS.md`、`docs/CODING_PLAN.md` 以及与你的改动相关的参数、测试和 ADR 文档。

## 工作项

一个 issue 应描述：问题/声音目标、范围、非目标、acceptance criteria、测试方法、是否需要听测、参数与 Host automation/state 影响、依赖和 owner。没有这些信息的工作先留在 Backlog，不直接进入实现。

## 分支与提交

```text
feat/<issue>-short-name
fix/<issue>-short-name
test/<issue>-short-name
docs/<issue>-short-name
experiment/<issue>-short-name
```

推荐提交格式：

```text
feat(routing): add serial stage mixer
fix(host): preserve inactive routing values
test(dsp): add finite-output property cases
docs(plan): define M1 exit gate
```

实验分支可以快速迭代，但生产 PR 不得直接依赖临时脚本、未固定 seed 或未授权素材。

## Pull Request

PR 保持单一目标，并使用仓库模板填写：

- 改了什么、为什么；
- 架构边界是否变化；
- Parameter ID/range/default/automation/state 是否变化；
- 音频线程和内存行为；
- 实际执行的测试；
- 涉及声音时的 render 与听测结论。

参数合同、routing、state schema、核心 DSP 和发布流程的变更，应先有 ADR 或更新既有 ADR。

## 最小验证

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

根据影响范围追加 Release、ASAN、pluginval、render regression、automation/state 和 DAW 测试。详细矩阵见 `docs/TESTING.md`。

## Review 重点

- DSP 是否绕过 Snapshot 直接读 APVTS；
- `process()` 是否分配、阻塞、I/O、访问 UI/history；
- routing 语义是否泄漏进 Water/Ice 算法；
- 参数 ID 或 state 是否破坏兼容性；
- mode 切换是否保留暂时无效参数；
- Input/Output Gain 和 Global Mix 位置是否漂移；
- 自动测试、听测与性能证据是否足以支持 Done。
