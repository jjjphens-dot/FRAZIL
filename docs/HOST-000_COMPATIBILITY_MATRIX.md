# [M0][HOST-000] 初始平台与 DAW 兼容性矩阵

> 状态：**Proposed — 等待 Sound & Host Lead 决策与 Engineering Lead review**
> Implementation DRI：Sound & Host Lead
> Required reviewer：Engineering Lead
> 审计基线：`origin/main` observed `b91f619`（2026-09-07）

本文档只冻结 HOST-000 所需的候选范围、证据格式和后续 HOST-001 smoke 入口。它不把本机发现、历史 CI/pluginval 结果或未执行的 DAW 操作写成正式支持。

相关合同：[Coding Plan](CODING_PLAN.md)、[Testing](TESTING.md)、[Parameters](PARAMETERS.md)、[GitHub Workflow](GITHUB_WORKFLOW.md)、[Document Governance](DOCUMENT_GOVERNANCE.md)、[Collaboration Roles](COLLABORATION_ROLES.md)、[Environment](ENVIRONMENT.md)。

## 1. Scope / Non-goals

### Scope

- 初始产品目标：Windows x64 上的 VST3 音频插件；
- JUCE Standalone 作为开发、调试和基础生命周期 smoke 宿主；
- 为 `HOST-001`、`AUTO-001` 和 M6 `QA-007` 提供唯一、可复现的 DAW 角色和 evidence record 格式；
- 记录当前机器可发现的操作系统、CPU、DAW/Host 和 pluginval 工具。

### Non-goals

- 本任务不修改 C++、参数、state、routing、realtime 或 latency 合同；
- 不承诺 AU、AAX 或 macOS/Linux；
- 不把 Standalone 测试当作 DAW 兼容性验证；
- 不因为 DAW 已安装就宣称正式支持；
- 不承诺 sample-accurate Host automation；v1 仍是每个 process block 建立一次 coherent snapshot；
- 不改变 v1 Host-reported processing latency = 0 samples 的合同；
- 不执行安装、启动、插件扫描、DAW 工程保存或修改用户 DAW 配置。

## 2. Support classification

| 分类 | 含义 | 当前 HOST-000 规则 |
|---|---|---|
| Officially Supported | Sound & Host Lead 明确确认、Engineering Lead 完成技术 review，并有对应 HOST-001 evidence | 当前为空；不能从安装或历史结果推导 |
| Development Validated | 当前开发机上按记录的版本完成过可复现 smoke，证据包含步骤、设置、结果和 artifact/version | 当前为空；本轮只做发现审计，未运行 DAW smoke |
| Best Effort / Not Formally Supported | 可作为候选或兼容性尝试，但没有正式承诺或完整证据 | 本机发现的 DAW、未确认版本的其他 VST3 host、Standalone 之外的未测 host |

正式支持范围在 Sound & Host Lead 明确确认前保持 **Proposed**，Engineering Lead review 通过前保持 **Not Accepted**。

## 3. Platform / format matrix

### 3.1 Observed machine

| 维度 | 实际发现 | 证据状态 |
|---|---|---|
| OS | Windows 11 家庭版中文版，version `10.0.26100`，build `26100` | Verified by local read-only OS query |
| OS architecture | x64 | Verified |
| CPU | Intel(R) Core(TM) Ultra 9 275HX | Verified |
| CPU architecture | 64-bit / x64 | Verified |
| Logical processors | 24 | Verified |

### 3.2 Product formats and roles

| Format / host | Proposed role | Current classification | Limitation |
|---|---|---|---|
| Windows VST3 | Primary product format | Proposed target | Still needs DAW scan/load and HOST-001 evidence |
| JUCE Standalone | Development/debug/lifecycle smoke | Development tool, not DAW support | Cannot exercise DAW scanning, automation lanes, project save/reopen or DAW offline bounce semantics |
| AU | Out of scope | Not supported | No contract or validation planned |
| AAX | Out of scope | Not supported | No contract or validation planned |

## 4. DAW / Host evidence audit

“Discovered” means an installed-app entry or executable was found. It does not mean the application was launched or that FRAZIL was loaded.

| Host | Exact version discovered | Local executable / evidence | Proposed role | Current status |
|---|---|---|---|---|
| Ableton Live 12 Suite | `12.4.2` | `C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe`; executable ProductVersion/FileVersion `12.4.2` | Primary development DAW candidate | Installed/discovered; launch, scan, enumeration, automation and save/reopen **Not run** |
| FL Studio 2025 | `25.1.4.4951` | `D:\Arrange\FL25\FL64.exe`; executable ProductVersion `25.1.4.4951` | Primary validation DAW candidate | Installed/discovered; launch, scan, enumeration, automation and save/reopen **Not run** |
| REAPER | Exact version not discovered | No matching installed-app entry or executable found in the audited locations | Secondary/lightweight validation candidate | **Unknown / not installed evidence**; cannot claim validation |
| JUCE Standalone | Project target; runtime version not separately frozen | Role defined by JUCE/CMake target | Development/debug host | **Not a DAW**; no DAW compatibility claim |

### 4.1 Candidate risk comparison

| Candidate | Strength | Risk / unresolved decision |
|---|---|---|
| Ableton Live 12.4.2 | Installed exact version; representative music-production workflow | No local FRAZIL scan/load or automation evidence; license/support scope not inferred |
| FL Studio 25.1.4.4951 | Installed exact version; independent host workflow and VST3 path | No local FRAZIL scan/load or automation evidence; not a lightweight cross-check |
| REAPER | Appropriate lightweight cross-validation candidate in the existing testing guidance | Not discovered locally; exact version and availability must be supplied before validation |
| JUCE Standalone | Fast developer feedback for lifecycle/audio callback smoke | Does not replace DAW validation and cannot establish DAW compatibility |

## 5. Proposed role matrix

The following is a candidate assignment, not an accepted support commitment:

| Role | Candidate | Required confirmation |
|---|---|---|
| Primary development DAW | Ableton Live 12 Suite `12.4.2` | Sound & Host Lead confirms workflow ownership |
| Primary validation DAW | FL Studio 2025 `25.1.4.4951` | Sound & Host Lead confirms independent validation target |
| Secondary/lightweight validation host | REAPER, exact version TBD | Host must be installed or an exact version must be nominated |
| Standalone | JUCE Standalone | Engineering Lead confirms lifecycle/debug role; it remains outside DAW support |

Until the three DAW roles are confirmed, the classification remains Proposed and no host is Officially Supported.

## 6. M1 Host smoke scenarios

The selected primary and validation DAWs must each run the following HOST-001 cases. Each result is recorded as `Verified`, `Failed`, `Blocked` or `Not run`; “planned” is not evidence.

### 6.1 Scan and load

1. Install or point the host at the current Windows VST3 artifact.
2. Rescan plugins.
3. Confirm FRAZIL appears once with the expected product/name metadata.
4. Load an instance on a mono source and a stereo source.
5. Open and close the editor; record crashes, hangs, missing buses and format errors.

### 6.2 Parameter enumeration

Confirm the host exposes the nine current parameters without dynamic insertion/removal:

```text
water.enabled
ice.enabled
routing.mode
parallel.balance
water.amount
ice.amount
input.gain
global.mix
output.gain
```

Record display name, range/default/choice text, automation visibility and parameter order. This is an enumeration check, not a claim that all production DSP is implemented.

### 6.3 Automation acceptance

The v1 contract is:

```text
Host automation -> APVTS atomics -> one coherent snapshot per process block
                    -> continuous DSP smoothing where applicable
```

There is no sample-accurate Host automation promise. Test cases must cover:

- continuous: `input.gain`, `global.mix`, `output.gain`, `water.amount`, `ice.amount`, `parallel.balance`;
- discrete: `water.enabled`, `ice.enabled`, `routing.mode`;
- fast and slow ramps, final-value correctness, no click/zipper/NaN/Inf;
- inactive-mode values can be written, saved and restored without being cleared.

### 6.4 Save / reopen

1. Set non-default values on all nine parameters, including inactive-mode values.
2. Save the project.
3. Close the host/project and reopen it.
4. Confirm parameter values, routing choice and automation lanes are restored.
5. Record whether the host reports any missing parameter, changed order or state warning.

### 6.5 Offline render smoke

1. Render a fixed short input through the selected host at the declared audio settings.
2. Record render completion, channel count, length, finite output and obvious lifecycle failures.
3. Treat output comparison/determinism as a later RENDER-001 responsibility; HOST-001 must not claim a full render regression from a bounce smoke alone.

## 7. Minimum Host smoke sub-matrix

This is the smallest proposed manual matrix for HOST-001. It is intentionally smaller than the full automated matrix in `docs/TESTING.md`; the full matrix remains the M6/nightly target.

| Case | Sample rate | Block size | Channels | Purpose | Status |
|---|---:|---:|---|---|---|
| H1 | 48 kHz | 128 | mono | Primary baseline / mono path | Planned |
| H2 | 48 kHz | 128 | stereo | Primary baseline / stereo path | Planned |
| H3 | 44.1 kHz | 128 | stereo | Lower common sample rate | Planned |
| H4 | 96 kHz | 128 | stereo | Higher common sample rate | Planned |
| H5 | 48 kHz | 32 | stereo | Small block boundary | Planned |
| H6 | 48 kHz | 256 | stereo | Larger common block | Planned |
| H7 | 48 kHz | 1024 | stereo | Large block boundary | Planned |

The automated contract still lists 44.1/48/96 kHz, block sizes 32/64/128/256/512/1024, mono/stereo and Debug/Release/ASAN. HOST-001 should expand the manual cases when a host exposes a different fixed-buffer behavior.

## 8. Responsibility and evidence record

| Acceptance item | Execution DRI | Required reviewer / gate |
|---|---|---|
| DAW install/version and scan/load | Sound & Host Lead | Engineering Lead checks artifact, format and reproducibility |
| Nine-parameter enumeration | Sound & Host Lead | Engineering Lead checks registry/contract alignment |
| Automation lane, record/edit/playback | Sound & Host Lead | Engineering Lead checks block-snapshot and smoothing contract |
| Project save/reopen and state restore | Sound & Host Lead | Engineering Lead checks state compatibility and fallback scope |
| Offline render smoke | Sound & Host Lead | Engineering Lead checks settings and finite-output evidence |
| Standalone lifecycle smoke | Engineering Lead | Sound & Host Lead confirms it is not substituted for DAW evidence |

Use one record per host/case:

```text
HOST-001-<host>-<version>-<case>
date/time:
OS / architecture:
host / exact version:
format:
plugin artifact version or commit:
sample rate / block size / channels:
steps:
expected result:
actual result: Verified | Failed | Blocked | Not run
logs / screenshot / issue link:
DRI:
reviewer:
limitations:
```

Per the current working instruction, use version/commit identifiers and evidence URLs; do not add content hashes to this matrix.

## 9. pluginval and historical evidence

### Current local availability

- `E:\FRAZIL\tools\bin\pluginval.exe` was found with ProductVersion/FileVersion `1.0.4`.
- The executable responded to `--help`; this proves tool availability only, not FRAZIL validation.
- HOST-000 did not run pluginval against a current artifact.

### Historical repository evidence

- The `origin/main` `PROJECT_STATUS.md` records a historical Debug VST3 pluginval strictness 5 success for the M1-A/M1-B merge candidate and links the [PR #3 Hosted CI run](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34044332388).
- That historical result is not reused as current HOST-001 evidence: the exact validator artifact commit is not recorded in the current status snapshot, and no DAW matrix was executed in this task.
- The [M1-A/M1-B PR](https://github.com/jjjphens-dot/FRAZIL/pull/3) is the historical review context; it does not establish Ableton or FL Studio compatibility.

## 10. Known limitations and review state

- No DAW was launched in this read-only audit; scan/load, parameter enumeration, automation, save/reopen and offline render are **Not run**.
- Ableton Live and FL Studio are installed candidates, not validated hosts.
- REAPER was not discovered; the secondary host role is unresolved.
- Windows 11 x64 is observed on the reference machine, not yet an accepted product-wide OS support promise.
- Official support is **None / Not Accepted** until Sound & Host Lead confirms the candidate roles and Engineering Lead reviews technical supportability.
- No ADR is required for this proposal because it records an existing product format/latency/automation contract and does not change architecture or public parameter semantics. A new ADR is required only if the final support decision changes a locked contract or product boundary.

## 11. Review checklist

- [ ] Sound & Host Lead confirms primary development DAW.
- [ ] Sound & Host Lead confirms primary validation DAW.
- [ ] Sound & Host Lead nominates an exact secondary/lightweight host version, or explicitly accepts `TBD`.
- [ ] Engineering Lead reviews VST3/Standalone roles, automation wording, latency wording and evidence requirements.
- [ ] HOST-001 records actual scan/load, nine-parameter enumeration, automation, save/reopen and offline render results.
- [ ] Matrix is reclassified from Proposed only after the above decision and review evidence exist.
