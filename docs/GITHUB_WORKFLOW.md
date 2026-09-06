# FRAZIL GitHub 工作流

> 远端：`https://github.com/jjjphens-dot/FRAZIL`  
> 团队：2 人  
> 模型：轻量 GitHub Flow，`main` 始终可构建、可测试。

## 1. 首次接入

首次接入必须在仓库 owner 明确授权后执行。执行前审计待提交文件，确认没有构建产物、工具二进制、下载归档、生成音频、凭据或私人路径；自动化 agent 不得在没有该明确授权时 push。

建议顺序：

```powershell
Set-Location F:\coding\FRAZIL
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
3. 本地 F: preset 与 portable CI preset 分离。

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

PR 必需检查：

- portable Windows configure/build/test；
- formatting/lint（建立后）；
- unit/DSP/integration tests；
- 变更影响对应的 render/pluginval 检查；
- 文档链接/状态一致性。

音频 render 和大型日志不要直接塞入 Git 历史；使用 GitHub Actions artifact 或 release asset，并在 PR 记录 manifest/hash。

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

### PR required

- Windows portable Debug configure/build/CTest；
- 参数/状态/unit tests；
- source format 检查；
- 对改动路径触发相关 DSP/render smoke。

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

CI 不应使用本机 F: 盘绝对 compiler/SDK 路径。保留 `windows-debug` 等本地 presets，同时新增继承同一 cache contract 的 portable CI preset。

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
