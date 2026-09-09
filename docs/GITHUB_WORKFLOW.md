# FRAZIL GitHub 工作流

> 远端：`https://github.com/jjjphens-dot/FRAZIL`  
> 团队：2 人  
> 模型：轻量 GitHub Flow，`main` 始终可构建、可测试。

## Modification Policy

稳定的分支、PR、review、CI 和发布流程属于 CONTROLLED 内容，修改必须有治理 issue/review，并同步 `AGENTS.md` 与相关计划文档。当前远端 Issues、labels、Projects、Actions 或 branch protection 状态不写入本文件；这些事实只进入 `docs/PROJECT_STATUS.md`，并附实际核验时间和证据。

## 1. 首次接入

首次接入必须在仓库 owner 明确授权后执行。执行前审计待提交文件，确认没有构建产物、工具二进制、下载归档、生成音频、凭据或私人路径；自动化 agent 不得在没有该明确授权时 push。

建议顺序：

```powershell
Set-Location <repo-root>
git init -b main
git remote add origin https://github.com/jjjphens-dot/FRAZIL.git
git status --short
git add .
git status --short
git commit -m "chore(repo): bootstrap FRAZIL project"
git push -u origin main
```

在 `git add .` 后、commit 前必须确认没有：`build/`、`.venv/`、`tools/bin/`、`tools/downloads/`、`testdata/rendered/`、DAW cache、个人凭据或绝对路径私有配置。`external/JUCE` 的入库方式必须先按第 2 节决定。

## 2. JUCE 与可复现依赖

当前工程要求 `external/JUCE/CMakeLists.txt`。M0 已选择固定 commit 下载并应用仓库内补丁：

1. `tools/bootstrap_dependencies.ps1` 拉取 JUCE 9.0.1 的固定 commit；
2. 脚本幂等应用 `tools/patches/JUCE-9.0.1-msvc-toolchain.patch`；
3. 所有共享 Windows preset 使用 portable tool discovery；本地 toolchain 或 build directory 差异只写入 ignored `CMakeUserPresets.json`，本地与 CI 只通过 preset 名称和运行环境区分。

不允许“依赖开发者机器上恰好存在的 external/JUCE”。`tools/bin` 和下载包不入库，bootstrap 文档固定下载地址和 revision。

## 3. Issue taxonomy

以下是计划中的 label taxonomy，不代表 GitHub metadata 自动存在。自定义 `type:*`、`area:*`、`priority:*`、`status:*` 等只有在 `GH-001` 实际创建并核验后才能使用；在此之前只能视为目标规则。

### 类型 label

```text
type:feature  type:bug  type:experiment  type:test  type:docs  type:build
```

### 范围 label

```text
area:host  area:app  area:water  area:ice  area:routing  area:ui  area:state  area:ci
```

### 优先级与状态

```text
priority:P0  priority:P1  priority:P2
status:blocked  needs:listening  needs:DAW  breaking-change
```

Milestone 使用 `M0` 至 `M7`，issue title 使用 Coding Plan 的稳定 ID，例如：

```text
[M1][PARAM-001] Extract static ParameterLayout and lock core IDs
[M4][ROUTE-004] Implement Water -> Ice serial processing
```

### 3.1 Work item ownership contract

Issue 进入 Ready 前必须记录：

```text
Stable ID / Milestone:
Implementation DRI:
Acceptance DRI:
Scope / Non-goals:
Engineering acceptance:
Sound / Host acceptance:
```

cross-module、production DSP、contract/ownership-sensitive、Host handoff 或 milestone-gate 工作还应按风险记录
write ownership、Allowed/Forbidden paths、双方 inputs/outputs、handoff condition、Joint Gate、相关
contract/ADR 和 documentation impact。普通 bounded task 可省略这些字段或写 `N/A — bounded scope`。

Acceptance DRI 发现 implementation finding 后默认交回 Implementation DRI 修复；当前 scope 内的 typo、
test/docs 或 trivial integration correction 可以直接完成。只有 substantial implementation responsibility
换人时才记录 DRI Transfer；transfer 不自动改变已有 PR creator 或后续 push-account rule。

## 4. Board 与 WIP

```text
Backlog -> Ready -> In Progress -> Code Review -> Listening/DAW Test -> Done
```

- 每人最多 1 个 In Progress，团队 WIP <= 2；
- blocked issue 写清阻塞条件和解除 owner，不长期占用 In Progress；
- experiment 通过 gate 后新建 production issue，不直接把探索 PR 改名合并；
- milestone exit issue 汇总该阶段 proof artifacts 与未关闭风险。

## 5. 分支与 PR

分支：`feat/`、`fix/`、`test/`、`docs/`、`build/`、`experiment/`。PR 应小于一个完整 milestone，优先让接口/测试先落地，再接入算法。

合并策略建议 Squash merge，使 `main` 每个 commit 对应一个完整 issue。PR title 使用可读的 Conventional Commit 风格；实验 PR 可保留 draft，未经 gate 不合并生产 target。

PR validation is impact-based。所有 PR 至少检查 changed-file sanity、凭据/私有路径/禁止生成物、与变更直接
相关的 documentation/link/path consistency、scope/ownership consistency，以及与风险相称的 review。

- Code / build / dependency PR：运行相关 configure、build、CTest 和 portability validation；
- App / DSP PR：运行相关 build、unit/property tests；只有声音或 routing behavior 受影响时才要求对应 render
  regression，并在合同要求时提供 listening evidence；
- Plugin / Host integration PR：按影响运行 plugin integration tests 和 pluginval；真实 DAW matrix 只用于
  HOST work item、milestone/release gate 或明确 compatibility task；
- Pure docs / template / governance PR：只要求 Markdown/YAML validity、link/path reference、stale-rule search、
  实际受影响合同的跨文档一致性和 `git diff` sanity。此类 PR 不要求无关 Windows build、CTest、ASAN、
  pluginval、render、DAW、listening 或 performance run。

Formatting/lint、Code Quality Review、Comment & Documentation Pass、module README 和 `MODULE_INDEX.md` 也只在
对应 source、interface、module 或 documentation facts 受影响时执行。render/pluginval 始终只由相关 behavior
变化触发，不是所有 PR 的固定 gate。

Documentation Synchronization Gate 的 canonical 规则位于 [`DOCUMENT_GOVERNANCE.md`](DOCUMENT_GOVERNANCE.md#5-documentation-synchronization-gate)。涉及 parameter/state、routing、realtime、核心 DSP、latency、random semantics、performance budget 或 release 的 PR，还必须留下可验证的 GitHub formal review；无法提交 formal review 时，第二位开发者必须在 PR comment 中写明 review scope、复现 evidence、limitations 和 decision。

### 5.1 GitHub identity 与 push-account gate

以下身份相关但不要求相同：

- Implementation DRI / Acceptance DRI：工作 ownership；
- PR creator / author：创建 GitHub PR 的账号；
- push account：执行当前 push 的 authenticated GitHub account；
- commit author / committer：commit 的真实 authorship/commit metadata；
- reviewer：承担并记录 review responsibility 的人/账号。

`PR creator != Implementation DRI`、`commit author/committer != PR creator` 或 reviewer 没有预绑定角色账号，
本身都不是违规，也不要求重建 PR、伪造 author 或 rewrite history。

GitHub identity 是 existing-PR push 的 lazy runtime gate，不是 read、review、local edit/test/commit 或每次
普通 push 的固定 preflight。

准备在当前 context 首次向某 branch push 时：

```powershell
$branch = git branch --show-current
gh pr list --head $branch --state open --json number,author,url
```

1. 如果没有 open PR，按普通 push/PR 创建流程继续；PR creator 无需等于 Implementation DRI。
2. 如果已存在 open PR，且本 repository + branch + PR + authenticated session 尚未验证，再读取 PR
   creator 并执行 `gh api user --jq .login`。
3. 只有 `current push account == existing PR creator` 才可向该 PR branch push。
4. 不一致时停止 push，报告 `Existing PR creator`、`Current authenticated account` 和“可能登录了错误
   GitHub account”；先检查/切换登录，不得跨账号继续 push。
5. 不得用改写 commit author、force push、rewrite history 或创建不必要的新 PR 规避账号不一致。

同一 repository + branch + PR + authenticated session 的成功检查可以在当前工作 context 复用。branch、
repository、PR 或 authentication/account 变化，工具报告权限/账号异常，或用户明确说明账号变化时才重新
检查。该规则同样适用于 documentation 和 production PR。

### 5.2 Reviewer 与证据

Reviewer 不需要在 PR 创建前与某个角色账号预绑定，也不因曾为相关领域提交过其它 commit、PR creator
与 Implementation DRI 不同，或 commit authorship 混合而自动失去资格。真正的 gate 是 review
independence、review scope、evidence 和 decision。

核心/高风险 PR 使用完整 review record：

```text
Reviewer:
Review scope:
Evidence reproduced:
Evidence not reproduced:
Findings:
Decision:
Formal GitHub review: APPROVE / COMMENT / REQUEST_CHANGES / NOT AVAILABLE / NOT RECORDED
Fallback review evidence: COMMENT / MANUAL REVIEW / N/A
```

GitHub 平台自身不允许 formal approval 时，可以保留明确标注的 comment/manual evidence；不得把普通
comment 写成 formal `APPROVE`。需要另一位开发者 review 的核心 PR 仍必须有真实的第二人证据；自审不能
冒充独立 review，但项目不再为此建立额外的账号身份矩阵或强制重建 PR。

完整 record 强制用于 parameter/state contract、core DSP、Water/Ice algorithm adoption、routing、realtime
boundary、latency、random semantics、formal performance budget、Beta/Release 和 milestone exit。普通 docs、
bounded test、typo、narrow tooling 或 low-risk maintenance 只需 Reviewer、Decision 和 notable
limitations/findings。

PR 描述记录 Implementation/Acceptance DRI、PR creator、reviewer 和 review type。Current push account 是
运行时状态，commit author/committer 已由 Git history 记录，不在 PR body 重复维护。

音频 render 和大型日志不要直接塞入 Git 历史；使用 GitHub Actions artifact 或 release asset，并在 PR
记录可定位的 artifact reference 与必要元数据。只有既有 corpus、发布或完整性合同明确要求时才记录 hash，
普通文档、构建或工作树验证不得为了“留证”额外计算 hash。

## 6. `main` 保护建议

首次 CI 稳定后配置：

- Require a pull request before merging；
- Require 1 approval；
- Dismiss stale approvals on new changes；
- Require review from code owners（若后续加入 CODEOWNERS）；
- Require status checks and branch up to date；
- Require conversation resolution；
- Block force pushes and deletion；
- 只允许仓库 owner 的紧急 bypass，并在事后补 issue/PR 记录。

不要在 CI 尚不能 fresh-clone 复现时把一个必然失败的 check 设为 required；先完成 `BUILD-001/002`。

## 7. CI 分层计划

### PR / change-triggered validation

- 所有 PR：changed-file/policy sanity，以及与实际变更相关的 docs/template validity 和 consistency；
- `src/**`、production test infrastructure、CMake/presets、dependency/bootstrap、CI workflow、build scripts 或
  executable tooling：Windows portable Debug configure/build/CTest、portability validation 和相关 tests；
- app/DSP/parameter/state/routing：按影响增加 unit/property/integration 与必要 render smoke；
- plugin/Host：按影响增加 plugin integration/pluginval；DAW matrix 只在 HOST、compatibility、milestone 或
  release gate 执行；
- pure docs/template/non-executable governance：不要求无关 build、CTest、RENDER-001、DSP 或 plugin jobs。

source format 检查只对其覆盖的 source path 生效。CI 即使因平台配置对 docs-only PR 自动运行额外 job，也不把
这些额外结果改写为该类 PR 的人工 acceptance requirement。

### Scheduled/nightly

- Release + ASAN；
- sample-rate/block-size 扩展矩阵；
- pluginval strictness 5；
- deterministic render regression；
- 多轮 automation/state fuzz。

### Manual/release

- clean release packaging；
- pluginval 与 Steinberg validator（接入后）；
- 目标 DAW matrix；
- artifact hash、版本、changelog、license/notice。

CI 不应使用本机盘符、compiler/SDK 安装目录或用户名路径。`windows-debug`、`windows-release`、`windows-asan` 与 `ci-windows-debug` 共享 portable tool discovery；本地差异通过 ignored `CMakeUserPresets.json` 注入，CI 不读取该文件。

## 8. Release 与版本

Pre-v1 使用 SemVer prerelease：`0.1.0-alpha.1` 等。版本来源应唯一（顶层 CMake 或生成的 version header），VST3 metadata、Standalone About、artifact 名称和 tag 保持一致。

Release checklist：

1. feature freeze；
2. M6 全 gate 通过，P0 blocker=0；
3. `CHANGELOG.md` 与迁移说明；
4. clean checkout Release build；
5. validator/DAW/state compatibility；
6. 签名/打包策略（若采用）；
7. tag + immutable GitHub Release；
8. 上传 hash 与验证环境；
9. 保留上一受支持版本用于 state regression。

## 9. 安全与大文件

- 禁止提交 token、证书私钥、签名凭据、DAW 用户目录和未授权音频；
- GitHub Actions 权限默认 `contents: read`，仅特定 release job 临时提升；
- 第三方 action 固定 major 或 commit，并定期审计；
- 测试音频在进入 Git 前确认许可与体积；超过合理体积时用 LFS 或 artifact，但先记录决策；
- Dependabot/安全扫描在基础 CI 稳定后启用，告警通过 issue triage，不自动盲目升级 JUCE。
