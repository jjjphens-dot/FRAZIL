# [M0][HOST-000] 初始平台与 DAW 兼容性矩阵

> 状态：**Product targets frozen by Sound & Host Lead; Engineering Lead review pending; HOST-001 evidence pending**
> Implementation DRI：Sound & Host Lead
> Required reviewer：Engineering Lead
> Initial HOST-000 audit baseline：`origin/main` observed `b91f619`（历史事实，2026-09-07）；本次 integration audit observed `origin/main` `12d36a4`；live remote HEAD 必须由 Git 命令确认

Sound & Host Lead 已冻结 v1 的平台、格式和宿主目标。本文档同时记录当前证据状态，因此“目标已冻结”不等于“兼容性已通过”。HOST-000 定义 HOST-001 的验收目标；HOST-001 负责实际 pluginval/DAW smoke evidence，不构成循环依赖，也不需要完成后 HOST-000 才能合入。

当前产品目标：Windows 11 x64、64-bit VST3；Ableton Live 12 Suite `12.4.2` 为 primary development DAW；FL Studio 2025 `25.1.4.4951` 为 primary validation DAW；REAPER 为 secondary/lightweight validation host，但精确版本仍待安装/发现。JUCE Standalone 仅用于开发、调试和生命周期 smoke。

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

## 2. Support intent 与 evidence status

Support intent 与 evidence status 是两个独立维度，不得合并书写。例如 Ableton 可以是 `Primary development target`，同时当前 evidence 仍为 `Not run`。

### 2.1 Support intent

| Support intent | 含义 | 当前 HOST-000 目标 |
|---|---|---|
| Official v1 target | 团队决定在 v1 正式支持该平台/格式/宿主范围；不表示兼容性测试已经通过 | Windows 11 x64 + Windows VST3 64-bit；三个宿主角色按第 5 节记录 |
| Primary development target | 主要开发工作流宿主，不等于已完成兼容性验收 | Ableton Live 12 Suite `12.4.2` |
| Primary validation target | 主要独立验证宿主，不等于已完成兼容性验收 | FL Studio 2025 `25.1.4.4951` |
| Secondary validation target | 轻量级交叉验证宿主；版本锁定前不能写成已验证 | REAPER，精确版本 TBD |
| Development-only tool | 仅支持开发、调试或生命周期检查，不承担 DAW 兼容性 | JUCE Standalone |
| Out of scope / Not supported | 当前 v1 不提供生产目标或支持承诺 | AU、AAX、macOS、Linux |

### 2.2 Evidence status

| Evidence status | 适用含义 |
|---|---|
| Verified | 已观察到环境或事实，但不等于测试通过 |
| Passed | 按记录步骤完成且结果满足预期 |
| Failed | 已执行且结果不满足预期 |
| Blocked | 有明确阻塞原因，无法完成执行 |
| Planned | 已定义目标/步骤，但尚未执行 |
| Not run | 本次任务明确未执行 |
| Unknown | 当前没有足够证据判断 |

当前没有任何宿主可标为 `Development Validated` 或 `Passed`。正式支持分类仍需 Engineering Lead review，实际证据由 HOST-001 补充。

### 2.3 Compatibility classification

| Compatibility classification | Required condition | Current HOST-000 result |
|---|---|---|
| Officially Supported | Support intent 已冻结、Engineering Lead 完成技术 review，并有足够的可复现 HOST-001 evidence | None; no host is officially supported yet |
| Development Validated | 已按记录版本、设置和步骤完成真实运行验证，并保留 artifact/version、结果和 reviewer | None; current DAW cases are `Not run` |
| Best Effort / Not Formally Supported | 可能工作，但没有完整支持承诺或完整验证证据 | 已发现的 Ableton/FL Studio 在本轮；REAPER 在精确版本锁定前；其他未测 host |

因此，`Official v1 target` 是产品支持意图，不能直接改写为 `Officially Supported`；`Development Validated` 是证据分类，必须由 HOST-001 真实运行结果支撑。

## 3. Platform / format matrix

### 3.1 Observed machine

| 维度 | 实际发现 | 证据状态 |
|---|---|---|
| OS | Windows 11 家庭版中文版，version `10.0.26100`，build `26100` | Verified by local read-only OS query |
| OS architecture | x64 | Verified |
| CPU | Intel(R) Core(TM) Ultra 9 275HX | Verified |
| CPU architecture | 64-bit / x64 | Verified |
| Logical processors | 24 | Verified |

### 3.2 Frozen platform and format matrix

| Dimension | Frozen decision | Evidence status |
|---|---|---|
| Platform | Windows 11 x64 | Target frozen; release evidence pending |
| Format | VST3 64-bit | Target frozen; format/DAW evidence pending |
| Primary development DAW | Ableton Live 12 Suite `12.4.2` | Installed/discovered; DAW smoke `Not run` |
| Primary validation DAW | FL Studio 2025 `25.1.4.4951` | Installed/discovered; DAW smoke `Not run` |
| Secondary validation host | REAPER | `Planned`; exact version TBD; not validated |
| Development host | JUCE Standalone | Development-only; not DAW compatibility evidence |
| AU/AAX/macOS/Linux | Out of v1 scope | Not supported / Not run |

Windows 11 x64 是当前 v1 support intent，不是对所有 Windows 11 机器的兼容性保证。clean-machine、HOST-001 和 release gate 完成前，不得扩写为“所有 Windows 11 机器已验证兼容”。

## 4. DAW / Host evidence audit

“Discovered” means an installed-app entry or executable was found. It does not mean the application was launched or that FRAZIL was loaded.

| Host | Exact version discovered | Executable evidence | Frozen role | Current status |
|---|---|---|---|---|
| Ableton Live 12 Suite | `12.4.2` | Executable discovered locally; exact machine path intentionally omitted from tracked documentation; ProductVersion/FileVersion `12.4.2` | Primary development DAW | Installed/discovered; launch, scan, enumeration, automation and save/reopen **Not run** |
| FL Studio 2025 | `25.1.4.4951` | Executable discovered locally; exact machine path intentionally omitted from tracked documentation; ProductVersion `25.1.4.4951` | Primary validation DAW | Installed/discovered; launch, scan, enumeration, automation and save/reopen **Not run** |
| REAPER | Exact version not discovered | No matching installed-app entry or executable found in the audited locations | Secondary/lightweight validation candidate | **Unknown / not installed evidence**; cannot claim validation |
| JUCE Standalone | Project target; runtime version not separately frozen | Role defined by JUCE/CMake target | Development/debug host | **Not a DAW**; no DAW compatibility claim |

### 4.1 Candidate risk comparison

| Candidate | Strength | Risk / unresolved decision |
|---|---|---|
| Ableton Live 12.4.2 | Installed exact version; representative music-production workflow | No local FRAZIL scan/load or automation evidence; license/support scope not inferred |
| FL Studio 25.1.4.4951 | Installed exact version; independent host workflow and VST3 path | No local FRAZIL scan/load or automation evidence; not a lightweight cross-check |
| REAPER | Appropriate lightweight cross-validation candidate in the existing testing guidance | Not discovered locally; exact version and availability must be supplied before validation |
| JUCE Standalone | Fast developer feedback for lifecycle/audio callback smoke | Does not replace DAW validation and cannot establish DAW compatibility |

## 5. Frozen role matrix

The following is the Sound & Host Lead product decision. It freezes support intent only; the evidence status remains independent.

| Role | Frozen target | Evidence status |
|---|---|---|
| Primary development DAW | Ableton Live 12 Suite `12.4.2` | Installed/discovered; DAW smoke `Not run` |
| Primary validation DAW | FL Studio 2025 `25.1.4.4951` | Installed/discovered; DAW smoke `Not run` |
| Secondary/lightweight validation host | REAPER, exact version TBD | `Planned`; exact version and installation pending |
| Standalone | JUCE Standalone | Development-only; not DAW compatibility evidence |

The three product roles are frozen. No host is `Development Validated` or `Officially Supported` until the required Engineering Lead review and HOST-001 evidence exist.

## 6. Verification layers

The following layers answer different questions and must not be substituted for one another.

| Layer | Tool / scope | Evidence boundary | Current status |
|---|---|---|---|
| Layer 1 — VST3 format conformance | Steinberg VST3 Validator | Checks VST3 API, component, bundle and format conformance; suitable for CI | Validator was not configured/run in this task; `Planned` |
| Layer 2 — Cross-host stress validation | Tracktion `pluginval` | Generic plugin stability/compatibility checks; PR strictness 5, nightly 7, Beta/Release 10 | Historical strictness 5 evidence exists; current artifact not run; 7/10 `Planned` |
| Layer 3 — Real DAW acceptance | Ableton `12.4.2`, FL Studio `25.1.4.4951`, REAPER exact version TBD | Scan/load, bus, parameter, automation, state, editor and offline render | HOST-001; current cases `Not run` |
| Layer 4 — Release environment | Clean Windows 11 x64 machine | Install/uninstall, standard VST3 path, versioned artifact, multi-instance and long-running behavior | M6/M7 scope; not HOST-000 evidence |

Validator PASS or pluginval PASS does not equal real DAW PASS. Standalone PASS does not equal DAW compatibility evidence.

## 7. M1 Host smoke scenarios

Each frozen host target must run the following HOST-001 cases: the primary development DAW, the primary validation DAW, and the secondary host once its exact REAPER version is locked. Each result is recorded as `Verified`, `Failed`, `Blocked` or `Not run`; “planned” is not evidence.

### 7.1 Scan and load

1. Install the current Windows VST3 artifact in a standard VST3 location (`%LOCALAPPDATA%/Programs/Common/VST3/` for development or `%PROGRAMFILES%/Common Files/VST3/` for global installation), or record the host-specific scan path used.
2. Run a complete rescan/verify scan.
3. Confirm FRAZIL appears once with the expected product/name metadata.
4. Load an instance on a mono source and a stereo source.
5. Open and close the editor; record crashes, hangs, missing buses and format errors.

### 7.2 Parameter enumeration

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

### 7.3 Automation acceptance

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

### 7.4 Save / reopen

1. Set non-default values on all nine parameters, including inactive-mode values.
2. Save the project.
3. Close the host/project and reopen it.
4. Confirm parameter values, routing choice and automation lanes are restored.
5. Record whether the host reports any missing parameter, changed order or state warning.

STATE-001 的 versioned StateModel foundation 与 unit/XML transport restore evidence 已随 PR #5 合入 `main`。这不等于真实 DAW save/reopen 已验证；STATE-002 mode-value-retention integration 仍为 pending，Ableton、FL Studio 和 REAPER 的 save/reopen evidence 仍属于 HOST-001。不得把 `origin/feat/m1-state-contract` 的当前分支状态作为 `main` 的实现事实。

### 7.5 Offline render smoke

1. Render a fixed short input through the selected host at the declared audio settings.
2. Record render completion, channel count, length, finite output and obvious lifecycle failures.
3. Treat output comparison/determinism as a later RENDER-001 responsibility; HOST-001 must not claim a full render regression from a bounce smoke alone.

## 8. Minimum Host smoke sub-matrix

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

## 9. Responsibility and evidence record

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
plugin commit SHA:
plugin artifact version:
build type:
validator/tool version:
sample rate / block size / channels:
steps:
expected result:
result: Passed | Failed | Blocked | Not run
logs / screenshot / issue link:
DRI:
reviewer:
limitations:
```

Per the current working instruction, use version/commit identifiers and evidence URLs; do not add content hashes to this matrix.

## 10. pluginval and historical evidence

### Current local availability

- A local pluginval executable was found with ProductVersion/FileVersion `1.0.4`; its exact machine path is intentionally omitted from tracked documentation.
- The executable responded to `--help`; this proves tool availability only, not FRAZIL validation.
- HOST-000 did not run pluginval against a current artifact.

### Historical repository evidence

- The `origin/main` `PROJECT_STATUS.md` records a historical Debug VST3 pluginval strictness 5 success for the M1-A/M1-B merge candidate and links the [PR #3 Hosted CI run](https://github.com/jjjphens-dot/FRAZIL/actions/runs/34044332388).
- That historical result is not reused as current HOST-001 evidence: the exact validator artifact commit is not recorded in the current status snapshot, and no DAW matrix was executed in this task.
- The [M1-A/M1-B PR](https://github.com/jjjphens-dot/FRAZIL/pull/3) is the historical review context; it does not establish Ableton or FL Studio compatibility.

## 11. External basis

The verification layers and host-format boundaries use the following primary references:

- [Steinberg VST3 Validator](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Validator.html)
- [Steinberg VST3 bundle structure](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BFormat.html)
- [Steinberg VST3 standard locations](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BLocations.html)
- [Tracktion pluginval](https://github.com/Tracktion/pluginval)
- [Ableton supported plug-in formats](https://help.ableton.com/hc/en-us/articles/5937501570460-Supported-Plug-in-Formats)
- [FL Studio plugin standards](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins_supported.htm) and [plugin scanning](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)
- [Apple Audio Unit documentation](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html) and [Avid AAX/Pro Tools compatibility](https://kb.avid.com/pkb/articles/en_US/Compatibility/en343311) are scope references only; neither is a v1 production target.

## 12. Known limitations and review state

- No DAW was launched in this read-only audit; scan/load, parameter enumeration, automation, save/reopen and offline render are **Not run**.
- Ableton Live and FL Studio are installed candidates, not validated hosts.
- REAPER was not discovered; the secondary host role is unresolved.
- Windows 11 x64 is the frozen v1 platform target; the reference machine is not evidence that every Windows 11 machine is compatible.
- Official v1 target intent is frozen by Sound & Host Lead; formal Engineering Lead review and HOST-001 evidence are pending. Do not call this `Done` or `Development Validated`.
- No ADR is required because this records the existing Windows VST3 v1 boundary and does not change architecture or public parameter semantics. A new ADR is required only if a future decision changes a locked contract or product boundary.

## 13. Review checklist

- [x] Sound & Host Lead freezes primary development DAW.
- [x] Sound & Host Lead freezes primary validation DAW.
- [x] Sound & Host Lead selects REAPER as secondary/lightweight host; exact version remains `TBD`.
- [ ] Engineering Lead reviews VST3/Standalone roles, automation wording, latency wording and evidence requirements.
- [ ] HOST-001 records actual scan/load, nine-parameter enumeration, automation, save/reopen and offline render results.
- [ ] Evidence status is updated from `Not run`/`Planned` only after reproducible HOST-001 results exist.
