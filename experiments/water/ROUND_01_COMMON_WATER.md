# Round 01 — Common Water Identity

> Historical scope and source-specific evidence below are retained unchanged. Current Water
> definition acceptance, A1 versus A0/Preview and deferred work are indexed in [Water research](README.md).

> 2026-09-16 · Agent first → Human calibration · **PARTIAL FIRST PASS / NOT CALIBRATED**
> One question, four references (2 pure + 2 musical). No DSP candidate or acceptance.

## Question and evidence method

**哪些跨真实水声和音乐片段的现象，值得进入 Common Water 的人工语义校准？**

OBSERVATION：以下记录来自全文件数值、波形和 spectrogram 的直接检查，未直接听辨音频。
图表把立体声取均值；可能隐藏反相成分，且各图色标独立，不能跨图比较颜色强弱。
频谱图使用现有 analyzer 的 1024-sample window / 512 overlap（不同采样率下时间分辨率不同）。
所有时间都是当前片段内秒数，位置为图上近似读取，未裁剪或重采样原文件。

INTERPRETATION：文件名已在上轮入库时可见，无法声称来源盲化；从未获得 Sound Lead 对这些
reference 的 Fluid/Resonant/好坏标签。下面保留独立的、低信心的机制无关解释。
未听辨的 Water Salience 用 NOT ASSESSED，不以 Low 代替；Character 用 Uncertain。
这是当前访问能力的缺口，不能据此勾选“完整 perceptual first pass 已完成”。

## W-P002 — AGENT INITIAL v1

- **REFERENCE CLASS / SUBTYPE**：Pure / continuous（仅文件名来源提示）。
- **AGENT OBSERVATION — OBSERVATION**：24.5 s，48 kHz，立体声；约 1–24 s 波形具有
  持续低幅背景和分散的大幅尖峰，非单个孤立事件；spectrogram 宽带能量贯穿此区间。
- **PERCEIVED WATER SALIENCE**：NOT ASSESSED（无直接听辨）。
- **INITIAL CHARACTER**：Uncertain。
- **TEMPORAL CHARACTER — OBSERVATION**：图上多次能量突起叠在持续背景上；未测自相关，
  不能据此宣布 aperiodic 或排除听觉上的固定周期。
- **MATERIAL / MODAL CHARACTER — OBSERVATION**：低频有较亮频带；本图不分辨 liquid mode、
  环境噪声或录音空间响应，未提取 modal trajectory。
- **SOURCE RELATIONSHIP — OBSERVATION**：用户提供的真实水声录音，无被处理乐器或独立激励通道。
- **FUSED / LAYERED / AMBIGUOUS**：N/A（Pure；不能判音乐处理耦合）。
- **POSSIBLE CONFOUNDS — INTERPRETATION**：录音空间混响、背景噪声、麦克风频响及开头渐入
  均可能影响持续感；文件名 Cave 不能证明任何实际声学成因。
- **OBJECTIVE FEATURES WORTH CHECKING — ENGINEERING QUESTION**：分频带 envelope 和自相关能否
  定位重复活动，以便听评区分连续运动与机械重复？测量不能证明 Fluidity。
- **INTERPRETATION**：持续底层与局部事件可以作为“连续中仍有局部变化”的语义候选。
  这与 Fluid 意图相关，但本轮不能把该参考正式标成 Fluid 或 Common 正例。
- **CONFIDENCE / WHY**：Low（产品解释）；波形现象可见，Water 感知及其成因尚未核验。

## W-P003 — AGENT INITIAL v1

- **REFERENCE CLASS / SUBTYPE**：Pure / pour_container（仅文件名来源提示）。
- **AGENT OBSERVATION — OBSERVATION**：10.656 s，96 kHz，立体声；约 0–8.5 s 有连续背景和多个
  明显尖峰，约 8.5–10.6 s 衰弱至接近零；全文件 crest factor 约 31.41 dB。
- **PERCEIVED WATER SALIENCE**：NOT ASSESSED。
- **INITIAL CHARACTER**：Uncertain。
- **TEMPORAL CHARACTER — OBSERVATION**：约 2–6 s 可见强瞬态聚集；不同事件幅度差异明显。
- **MATERIAL / MODAL CHARACTER — OBSERVATION**：spectrogram 存在宽带竖向结构；尚未分离可追踪
  的 modal 衰减。末尾变弱不能等同于 resonator damping，可能只是动作停止或素材编辑。
- **SOURCE RELATIONSHIP — OBSERVATION**：没有激励测量或音乐 dry source，不能验证 input coupling。
- **FUSED / LAYERED / AMBIGUOUS**：N/A。
- **POSSIBLE CONFOUNDS — INTERPRETATION**：容器、接触动作、录音距离、截取/淡出均可能影响形态。
- **OBJECTIVE FEATURES WORTH CHECKING — ENGINEERING QUESTION**：逐事件 envelope 与短时频率轨迹
  能否把激励、短响应和编辑尾部区分开？不能用全段衰减拟合证明液态阻尼。
- **INTERPRETATION**：适合校准“输入动作—局部响应”的描述；事件性不必与连续性互斥。
  “倒入容器”不自动等于 Resonant，亦不要求未来 DSP 重现这些独立声音事件。
- **CONFIDENCE / WHY**：Low；只有波形/频谱证据，缺少听觉材质判断和激励因果信息。

## W-M001 — AGENT INITIAL v1

- **REFERENCE CLASS / SUBTYPE**：Musical / AMBIGUOUS（证据不足，非听辨结论）。
- **AGENT OBSERVATION — OBSERVATION**：34.5 s，48 kHz，立体声；约 0–21.5 s 波形多次接近满刻度，
  约 22 s 后幅度整体下降；后段 spectrogram 的多条横向结构比前段更易辨认。
- **PERCEIVED WATER SALIENCE**：NOT ASSESSED。
- **INITIAL CHARACTER**：Uncertain。
- **TEMPORAL CHARACTER — OBSERVATION**：可见段落层级的能量/纹理变化；没有标注水元素强弱。
- **MATERIAL / MODAL CHARACTER — OBSERVATION**：横向频谱结构可能与有音高内容有关；图本身
  不区分乐器谐波、resonance 或 Water。sample peak 约 -0.0003 dBFS，不是 true peak。
- **SOURCE RELATIONSHIP — OBSERVATION**：完整混音切片，无 stems/dry pair；来源说明包含水元素，
  但其具体来源和融合程度尚未核验。
- **FUSED / LAYERED / AMBIGUOUS**：AMBIGUOUS，pending auditory classification。
- **POSSIBLE CONFOUNDS — INTERPRETATION**：编曲/乐器变化、mastering、压缩/限制、EQ、reverb、
  delay、chorus/flanger、granular、独立 Foley 和 widening 都是待排查项，并非检测到这些效果。
- **OBJECTIVE FEATURES WORTH CHECKING — ENGINEERING QUESTION**：先人工定位真正的水元素，
  再在相近编曲条件下查 envelope/谱活动；约 22 s 转折是否只是 arrangement 变化？
- **INTERPRETATION**：目前价值是暴露混杂变量；不能把“较弱/更可见的谐波后段”定义成 Resonant，
  也不能把更响的前段定义成更强 Water。该单曲现象仅进入问题列表。
- **CONFIDENCE / WHY**：Low；多个解释同样成立，没有被处理源对照或听评。

## W-M007 — AGENT INITIAL v1

- **REFERENCE CLASS / SUBTYPE**：Musical / AMBIGUOUS（证据不足）。
- **AGENT OBSERVATION — OBSERVATION**：12 s，48 kHz，立体声；约 0.4 s 后开始明显活动，
  之后持续背景上叠加多个起伏；频谱同时有竖向结构和延续一段时间的横向结构。
- **PERCEIVED WATER SALIENCE**：NOT ASSESSED。
- **INITIAL CHARACTER**：Uncertain。
- **TEMPORAL CHARACTER — OBSERVATION**：多次局部能量变化；片段末端快速下降，不能直接当作
  自然 tail。当前未做事件密度或周期性检验。
- **MATERIAL / MODAL CHARACTER — OBSERVATION**：横向频带不足以证明 modal liquid material。
- **SOURCE RELATIONSHIP — OBSERVATION**：混音，缺少 dry/stems；不能判断 input-driven transform。
- **FUSED / LAYERED / AMBIGUOUS**：AMBIGUOUS，pending auditory classification。
- **POSSIBLE CONFOUNDS — INTERPRETATION**：乐器音高、演奏节奏、混响、延迟、调制、granular、
  水声叠层、EQ/mastering 和立体声处理都可能贡献当前图形；没有进行效果识别。
- **OBJECTIVE FEATURES WORTH CHECKING — ENGINEERING QUESTION**：在人工确认的水元素位置，
  能否找到与主体起音相关的 envelope 变化？相关性本身不能证明处理因果。
- **INTERPRETATION**：可作为第二个音乐语境检查“持续背景与局部变化”是否仍有描述价值，
  但不能靠和纯水图形相似就宣称共享 Water 特征或 FUSED。
- **CONFIDENCE / WHY**：Low；仅独立图形解释，无法隔离水元素。

## Cross-reference synthesis — AGENT INITIAL v1

| Statement type | 当前结论 | 产品处理 |
|---|---|---|
| OBSERVATION | 两个 pure 参考都有持续活动及局部尖峰；两个 musical 参考也有背景和局部起伏 | 只是通用信号共现，尚非 Water-specific |
| INTERPRETATION | “连续材质中保有局部响应”值得 Sound Lead 校准 | Common semantic candidate，Low；未提升为主条款 |
| INTERPRETATION | “固定频谱形状”无法从这组不同类型/采样率/混音参考合理推导 | 不拟合整曲谱、不制定 EQ target |
| INTERPRETATION | 事件性与连续性可能同时存在，不宜以二分法直接划分两模式 | 待人耳确认，非 mode-separation 标准 |
| ENGINEERING QUESTION | 哪些局部变化被听成水，哪些只是鼓、乐器或 Foley？ | 定位 reference 片段后再校准 |
| ENGINEERING QUESTION | 感知到的材质是否跟随主体，还是独立声层？ | 音乐 FUSED/LAYERED 核验；无 dry 时因果仍未知 |

**本轮没有新增已接受 Common Water 条款。** 产品主干仍来自最新 plan 的 Human Intent Seed。

## HUMAN CALIBRATED — pending

Sound Lead 使用 [LISTENING_LOG.md](LISTENING_LOG.md) 的四个独立记录块，填
ACCEPT / MODIFY / REJECT / UNCERTAIN。纠正写在独立 calibrated 区域，不覆盖 AGENT INITIAL v1。
当前 calibration 完成数 0/4；不得报告饱和、分类准确率或频繁正确/错误模式。

## Reproduction / artifacts

```powershell
python experiments/water/reference_intake.py --root $env:FRAZIL_REFERENCE_ROOT --ids W-P002 W-P003 W-M001 W-M007 --analyze --plots --output testdata/rendered/exp-w-001/round-01/observations.json
```

数值记录与各 ID 下 waveform.png / spectrogram.png / welch_psd.png 均为本地 ignored artifacts。
已直接检查四个 waveform 和四个 spectrogram；PSD 图已生成但未用于本轮解释。
无音频处理、自动分类或用户审美标签输入。后续 revision 应追加记录，不反写本版初始判断。
