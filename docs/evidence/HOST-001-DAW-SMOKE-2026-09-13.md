# HOST-001 primary DAW evidence — confirmed 2026-09-14

本记录最初保存 2026-09-13 的窄范围 DAW smoke 观察；2026-09-14，Sound & Host Lead / 用户按
[`HOST-001 / M1 acceptance index`](HOST-001_ACCEPTANCE_INDEX.md) 中列出的完整 primary-host case scope，
确认 Ableton Live 与 FL Studio 的全部适用检查均已执行且无异常。原始截图、DAW 工程和输出 WAV 未留存；
这是附件形式限制，不是本轮人工验证的失败或阻塞项。

## Traceability

- Observation owner and Sound/Host decision: Sound & Host Lead / user; `Passed` confirmation received
  2026-09-14，exact per-case observation times were not captured.
- Environment: Windows x64；exact OS build 与 audio-driver identity 未记录。
- Hosts: Ableton Live 12 Suite `12.4.2`；FL Studio 2025 `25.1.4.4951`。
- Artifact: FRAZIL 0.1.0 Debug VST3，来自 closeout candidate `1d45683`；Live 使用已配置的可扫描
  VST3 目录，FL Studio 使用 Windows 默认 VST3 位置。机器特定绝对路径不写入 tracked evidence。
- Primary matrix: H1 48 kHz/128/mono；H2 48 kHz/128/stereo；H3 44.1 kHz/128/stereo；
  H4 96 kHz/128/stereo；H5 48 kHz/32/stereo；H6 48 kHz/256/stereo；H7 48 kHz/1024/stereo。
- Scope boundary: Water/Ice 尚为 pass-through/未实现声音产品处理；本证据验证 Host surface、状态、
  automation、lifecycle 和 render smoke，不声称 Water/Ice 听感或未来 M4 routing crossfade 已完成。

## Passed primary-host cases

以下各项均由用户在 Live 与 FL Studio 中确认完成且无异常：

| Case | Result | Recorded observation |
|---|---|---|
| Group A / H1-H2 | `Passed` | 完整 rescan 后只有一个预期插件条目；真实 mono/stereo instance、editor lifecycle、bus 和全部九参数的名称、顺序、默认值、范围、choice/automation visibility 符合当前合同。 |
| Automation — gain / H2 | `Passed` | `input.gain` 与 `output.gain` 的慢速、快速及最终值录制、编辑、回放正常，无异常 click/zipper。 |
| Automation — continuous values / H2 | `Passed` | `global.mix`、`parallel.balance`、`water.amount`、`ice.amount` 使用独立 lane，endpoint/final value 与 inactive-value retention 正常。 |
| Automation — discrete / H2 | `Passed` | `water.enabled`、`ice.enabled` 的 Bool 组合及 `routing.mode` 三个 choice 在重复变化后保持注册并达到最终值；仅验证当前 M1 value/identity path，不外推未来 routing crossfade。 |
| Save/reopen / H2 | `Passed` | 九参数 distinctive values、routing、inactive values 与 automation lanes 在工程重开后恢复，无 missing parameter、order-change 或 state warning。 |
| DAW render / H2 | `Passed` | 固定短输入的 bounce 完成，可解码、输出有限且未观察到 lifecycle 异常；所用设置、channel/frame/length 数值未另行抄录，输出 WAV 未留存。 |
| Boundary / H3-H7 | `Passed` | 各配置完成 reconfigure/prepare、playback、parameter write、代表性 gain change 与 output observation，无异常。 |
| Developer UI / Host restore boundary / H2 | `Passed` | editor 打开时 Host restore 可清除 developer override 并回到 Processed；editor reopen/automation 与 stale developer edit 的 ownership 边界无异常。 |

每个表项同时适用于上述两个 primary host。用户的最终确认取代本记录先前的 `Verified`/`Not run`
临时分类；Engineering Lead 随后在精确 PR #27 HEAD `01d590f` 上正式批准 artifact identity、协议覆盖和合同边界。

## Evidence form and limitations

- 未保存截图、DAW 工程、Host log 或输出 WAV；用户明确选择不把这些可选原始附件作为本轮验收交付物。
- exact OS build、audio driver、逐 case 精确时间和 render 的数值 metadata 未记录。测试采用 acceptance index
  冻结的 H1-H7 配置，结果和可观察属性由 Sound & Host Lead 明确签认为通过。
- Deterministic/byte-level render 仍由既有 `RENDER-001` harness 证明；本记录只证明真实 DAW bounce smoke。
- REAPER 是 secondary/lightweight host，本轮延期且不形成支持声明；它不阻塞 `CODING_PLAN v1.3`
  明确列出的 primary-target DAW M1 Exit 条件。
- Engineering Lead 已正式批准精确 PR #27 HEAD `01d590f`，且 PR #27 已合入 `main@3438593`；因此两个
  primary host 达到本记录范围内的 `Development Validated`，HOST-001 关闭且 M1 Joint Exit 获批。
  `Officially Supported` 不由本记录授予。
