#include "PluginEditor.h"

#include "ParameterLayout.h"

#if FRAZIL_ENABLE_DEVELOPER_UI

#include <array>
#include <cmath>

namespace {
const auto kBackground = juce::Colour(0xff101820);
const auto kPanel = juce::Colour(0xff172631);
const auto kPanelBorder = juce::Colour(0xff2d4654);
const auto kText = juce::Colour(0xffe8f0f2);
const auto kMutedText = juce::Colour(0xff9db4bc);
const auto kAccent = juce::Colour(0xff5ed0ba);

constexpr std::array<const char*, 6> kSliderParameterIds{
    frazil::plugin::parameterIds::parallelBalance, frazil::plugin::parameterIds::waterAmount,
    frazil::plugin::parameterIds::iceAmount,       frazil::plugin::parameterIds::inputGain,
    frazil::plugin::parameterIds::globalMix,       frazil::plugin::parameterIds::outputGain};

constexpr std::array<const char*, 6> kSliderLabels{
    "Parallel Balance", "Water Amount", "Ice Amount", "Input Gain", "Global Mix", "Output Gain"};

constexpr std::array<const char*, 9> kHostParameterIds{
    frazil::plugin::parameterIds::waterEnabled, frazil::plugin::parameterIds::iceEnabled,
    frazil::plugin::parameterIds::routingMode,  frazil::plugin::parameterIds::parallelBalance,
    frazil::plugin::parameterIds::waterAmount,  frazil::plugin::parameterIds::iceAmount,
    frazil::plugin::parameterIds::inputGain,    frazil::plugin::parameterIds::globalMix,
    frazil::plugin::parameterIds::outputGain};

void styleButton(juce::Button& button) {
    button.setColour(juce::TextButton::buttonColourId, kPanelBorder);
    button.setColour(juce::TextButton::textColourOffId, kText);
    button.setColour(juce::TextButton::textColourOnId, kBackground);
    button.setTooltip("Explicit developer action; not a preset or experiment state file.");
}

juce::String comparisonModeName(frazil::plugin::DeveloperComparisonMode mode) {
    return mode == frazil::plugin::DeveloperComparisonMode::dry ? "Dry" : "Processed";
}

juce::String waterModelName(frazil::plugin::DeveloperWaterModel model) {
    return model == frazil::plugin::DeveloperWaterModel::resonant ? "Resonant" : "Fluid";
}
} // namespace

FRAZILAudioProcessorEditor::FRAZILAudioProcessorEditor(FRAZILAudioProcessor& processor)
    : AudioProcessorEditor(processor), processor_(processor) {
    setSize(1000, 720);
    setResizable(true, true);
    setResizeLimits(820, 680, 1440, 960);

    configureLabel(titleLabel_, "FRAZIL / DEV-UI-001", true);
    configureLabel(subtitleLabel_,
                   "Developer Control Surface  |  Debug/ASAN only  |  not Production UI");
    configureLabel(hostParametersLabel_, "HOST PARAMETER PATH", true);
    configureLabel(experimentLabel_, "WATER EXPERIMENT CONTROLS", true);
    configureLabel(workflowLabel_, "TEMPORARY WORKFLOW", true);
    configureLabel(diagnosticsLabel_, "RUNTIME DIAGNOSTICS", true);
    configureLabel(workflowStatusLabel_, "Ready. A/B slots are empty; actions are explicit.");
    configureLabel(workflowStateLabel_, "A: Empty  |  B: Empty  |  Current: Host  |  Processed");
    workflowStateLabel_.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(titleLabel_);
    addAndMakeVisible(subtitleLabel_);
    addAndMakeVisible(hostParametersLabel_);
    addAndMakeVisible(experimentLabel_);
    addAndMakeVisible(workflowLabel_);
    addAndMakeVisible(diagnosticsLabel_);
    addAndMakeVisible(workflowStatusLabel_);
    addAndMakeVisible(workflowStateLabel_);

    waterEnabledButton_.setButtonText("Water Enabled");
    iceEnabledButton_.setButtonText("Ice Enabled");
    waterEnabledButton_.setTooltip("Current Host parameter: water.enabled");
    iceEnabledButton_.setTooltip("Current Host parameter: ice.enabled");
    addAndMakeVisible(waterEnabledButton_);
    addAndMakeVisible(iceEnabledButton_);
    waterEnabledButton_.onClick = [this] { updateDeveloperOverrideFromUserEdit(); };
    iceEnabledButton_.onClick = [this] { updateDeveloperOverrideFromUserEdit(); };

    routingModeBox_.addItem("Parallel", 1);
    routingModeBox_.addItem("Water -> Ice", 2);
    routingModeBox_.addItem("Ice -> Water", 3);
    routingModeBox_.setTooltip("Current Host parameter: routing.mode");
    addAndMakeVisible(routingModeBox_);
    routingModeBox_.onChange = [this] { updateDeveloperOverrideFromUserEdit(); };

    for (std::size_t index = 0; index < parameterSliders_.size(); ++index) {
        configureLabel(parameterLabels_[index], kSliderLabels[index]);
        configureSlider(parameterSliders_[index]);
        parameterSliders_[index].setTooltip(juce::String("Host parameter: ") +
                                            kSliderParameterIds[index]);
        addAndMakeVisible(parameterLabels_[index]);
        addAndMakeVisible(parameterSliders_[index]);
        parameterSliders_[index].onValueChange = [this] { updateDeveloperOverrideFromUserEdit(); };
    }

    attachHostParameterControls();

    configureLabel(waterModelLabel_, "Model");
    configureLabel(waterSizeLabel_, "Size");
    configureLabel(waterMotionLabel_, "Motion");
    waterModelBox_.addItem("Fluid", 1);
    waterModelBox_.addItem("Resonant", 2);
    waterModelBox_.setSelectedId(1, juce::dontSendNotification);
    waterModelBox_.setTooltip(
        "Developer/experiment value; not a Host parameter or current DSP control.");
    configureSlider(waterSizeSlider_);
    configureSlider(waterMotionSlider_);
    waterSizeSlider_.setRange(0.0, 1.0, 0.001);
    waterMotionSlider_.setRange(0.0, 1.0, 0.001);
    waterSizeSlider_.setValue(0.5, juce::dontSendNotification);
    waterMotionSlider_.setValue(0.5, juce::dontSendNotification);
    waterSizeSlider_.setTextValueSuffix(" normalized");
    waterMotionSlider_.setTextValueSuffix(" normalized");
    waterSizeSlider_.setTooltip(
        "Experiment-only Water Size; exported only through explicit config action.");
    waterMotionSlider_.setTooltip(
        "Experiment-only Water Motion; exported only through explicit config action.");
    addAndMakeVisible(waterModelLabel_);
    addAndMakeVisible(waterSizeLabel_);
    addAndMakeVisible(waterMotionLabel_);
    addAndMakeVisible(waterModelBox_);
    addAndMakeVisible(waterSizeSlider_);
    addAndMakeVisible(waterMotionSlider_);
    waterModelBox_.onChange = [this] {
        if (!syncingDeveloperView_) {
            currentAppliedSlot_ = -1;
            updateWorkflowSummary();
        }
    };
    waterSizeSlider_.onValueChange = [this] {
        if (!syncingDeveloperView_) {
            currentAppliedSlot_ = -1;
            updateWorkflowSummary();
        }
    };
    waterMotionSlider_.onValueChange = [this] {
        if (!syncingDeveloperView_) {
            currentAppliedSlot_ = -1;
            updateWorkflowSummary();
        }
    };

    for (auto* button :
         {static_cast<juce::Button*>(&captureAButton_), static_cast<juce::Button*>(&applyAButton_),
          static_cast<juce::Button*>(&captureBButton_), static_cast<juce::Button*>(&applyBButton_),
          static_cast<juce::Button*>(&dryButton_), static_cast<juce::Button*>(&processedButton_),
          static_cast<juce::Button*>(&returnHostButton_),
          static_cast<juce::Button*>(&resetHostButton_),
          static_cast<juce::Button*>(&resetExperimentButton_),
          static_cast<juce::Button*>(&copyConfigButton_),
          static_cast<juce::Button*>(&exportConfigButton_)}) {
        styleButton(*button);
        addAndMakeVisible(*button);
    }

    captureAButton_.onClick = [this] { captureSlot(0); };
    applyAButton_.onClick = [this] { applySlot(0); };
    captureBButton_.onClick = [this] { captureSlot(1); };
    applyBButton_.onClick = [this] { applySlot(1); };
    dryButton_.onClick = [this] {
        setComparisonMode(frazil::plugin::DeveloperComparisonMode::dry);
    };
    processedButton_.onClick = [this] {
        setComparisonMode(frazil::plugin::DeveloperComparisonMode::processed);
    };
    returnHostButton_.onClick = [this] { returnToHost(); };
    resetHostButton_.onClick = [this] { resetHostParameters(); };
    resetExperimentButton_.onClick = [this] { resetExperimentControls(); };
    copyConfigButton_.onClick = [this] { copyExperimentConfig(); };
    exportConfigButton_.onClick = [this] { exportExperimentConfig(); };

    updateComparisonButtons();
    updateWorkflowSummary();
    startTimerHz(10);
}

FRAZILAudioProcessorEditor::~FRAZILAudioProcessorEditor() = default;

void FRAZILAudioProcessorEditor::configureLabel(juce::Label& label, const juce::String& text,
                                                bool heading) {
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, heading ? kAccent : kMutedText);
    label.setFont(juce::FontOptions(heading ? 12.0f : 11.0f));
    label.setJustificationType(heading ? juce::Justification::centredLeft
                                       : juce::Justification::centred);
}

void FRAZILAudioProcessorEditor::configureSlider(juce::Slider& slider) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 92, 22);
    slider.setColour(juce::Slider::rotarySliderFillColourId, kAccent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, kPanelBorder);
    slider.setColour(juce::Slider::textBoxTextColourId, kText);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, kBackground);
    slider.setColour(juce::Slider::textBoxOutlineColourId, kPanelBorder);
}

void FRAZILAudioProcessorEditor::attachHostParameterControls() {
    if (hostParameterControlsAttached())
        return;

    waterEnabledAttachment_ = std::make_unique<ButtonAttachment>(
        processor_.parameters, frazil::plugin::parameterIds::waterEnabled, waterEnabledButton_);
    iceEnabledAttachment_ = std::make_unique<ButtonAttachment>(
        processor_.parameters, frazil::plugin::parameterIds::iceEnabled, iceEnabledButton_);
    routingModeAttachment_ = std::make_unique<ComboBoxAttachment>(
        processor_.parameters, frazil::plugin::parameterIds::routingMode, routingModeBox_);
    for (std::size_t index = 0; index < parameterSliders_.size(); ++index)
        parameterAttachments_[index] = std::make_unique<SliderAttachment>(
            processor_.parameters, kSliderParameterIds[index], parameterSliders_[index]);
}

void FRAZILAudioProcessorEditor::detachHostParameterControls() {
    for (auto& attachment : parameterAttachments_)
        attachment.reset();
    routingModeAttachment_.reset();
    iceEnabledAttachment_.reset();
    waterEnabledAttachment_.reset();
}

bool FRAZILAudioProcessorEditor::hostParameterControlsAttached() const noexcept {
    return waterEnabledAttachment_ != nullptr;
}

frazil::plugin::DeveloperExperimentSnapshot
FRAZILAudioProcessorEditor::captureCurrentExperiment() const {
    frazil::plugin::DeveloperExperimentSnapshot snapshot;
    snapshot.host = processor_.getDeveloperHostParameterSnapshot();
    snapshot.water.model = waterModelBox_.getSelectedId() == 2
                               ? frazil::plugin::DeveloperWaterModel::resonant
                               : frazil::plugin::DeveloperWaterModel::fluid;
    snapshot.water.size = static_cast<float>(waterSizeSlider_.getValue());
    snapshot.water.motion = static_cast<float>(waterMotionSlider_.getValue());
    snapshot.comparisonMode = processor_.getDeveloperComparisonMode();
    return snapshot;
}

void FRAZILAudioProcessorEditor::syncHostControls(
    const frazil::plugin::DeveloperHostParameterSnapshot& snapshot) {
    const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
    waterEnabledButton_.setToggleState(snapshot.rawValues[static_cast<std::size_t>(
                                           frazil::plugin::DeveloperHostParameter::waterEnabled)] >=
                                           0.5f,
                                       juce::dontSendNotification);
    iceEnabledButton_.setToggleState(snapshot.rawValues[static_cast<std::size_t>(
                                         frazil::plugin::DeveloperHostParameter::iceEnabled)] >=
                                         0.5f,
                                     juce::dontSendNotification);
    const auto routing = static_cast<int>(std::lround(snapshot.rawValues[static_cast<std::size_t>(
        frazil::plugin::DeveloperHostParameter::routingMode)]));
    routingModeBox_.setSelectedItemIndex(juce::jlimit(0, 2, routing), juce::dontSendNotification);

    constexpr std::array indices{frazil::plugin::DeveloperHostParameter::parallelBalance,
                                 frazil::plugin::DeveloperHostParameter::waterAmount,
                                 frazil::plugin::DeveloperHostParameter::iceAmount,
                                 frazil::plugin::DeveloperHostParameter::inputGainDb,
                                 frazil::plugin::DeveloperHostParameter::globalMix,
                                 frazil::plugin::DeveloperHostParameter::outputGainDb};
    for (std::size_t index = 0; index < parameterSliders_.size(); ++index)
        parameterSliders_[index].setValue(
            snapshot.rawValues[static_cast<std::size_t>(indices[index])],
            juce::dontSendNotification);
}

void FRAZILAudioProcessorEditor::syncExperimentControls(
    const frazil::plugin::DeveloperWaterExperimentSnapshot& snapshot) {
    const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
    waterModelBox_.setSelectedId(
        snapshot.model == frazil::plugin::DeveloperWaterModel::resonant ? 2 : 1,
        juce::dontSendNotification);
    waterSizeSlider_.setValue(snapshot.size, juce::dontSendNotification);
    waterMotionSlider_.setValue(snapshot.motion, juce::dontSendNotification);
}

void FRAZILAudioProcessorEditor::activateDeveloperOverride(
    const frazil::plugin::DeveloperHostParameterSnapshot& snapshot, int slotIndex) {
    const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
    processor_.setDeveloperHostParameterOverride(snapshot);
    detachHostParameterControls();
    syncHostControls(snapshot);
    currentAppliedSlot_ = slotIndex;
    updateWorkflowSummary();
}

void FRAZILAudioProcessorEditor::applyExperimentSnapshot(
    const frazil::plugin::DeveloperExperimentSnapshot& snapshot, int slotIndex) {
    activateDeveloperOverride(snapshot.host, slotIndex);
    syncExperimentControls(snapshot.water);
    setComparisonMode(snapshot.comparisonMode);
    updateWorkflowSummary();
}

void FRAZILAudioProcessorEditor::updateDeveloperOverrideFromUserEdit() {
    if (syncingDeveloperView_ || !processor_.isDeveloperHostParameterOverrideActive())
        return;

    auto snapshot = processor_.getDeveloperHostParameterSnapshot();
    snapshot
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::waterEnabled)] =
        waterEnabledButton_.getToggleState() ? 1.0f : 0.0f;
    snapshot
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::iceEnabled)] =
        iceEnabledButton_.getToggleState() ? 1.0f : 0.0f;
    snapshot
        .rawValues[static_cast<std::size_t>(frazil::plugin::DeveloperHostParameter::routingMode)] =
        static_cast<float>(routingModeBox_.getSelectedItemIndex());

    constexpr std::array indices{frazil::plugin::DeveloperHostParameter::parallelBalance,
                                 frazil::plugin::DeveloperHostParameter::waterAmount,
                                 frazil::plugin::DeveloperHostParameter::iceAmount,
                                 frazil::plugin::DeveloperHostParameter::inputGainDb,
                                 frazil::plugin::DeveloperHostParameter::globalMix,
                                 frazil::plugin::DeveloperHostParameter::outputGainDb};
    for (std::size_t index = 0; index < parameterSliders_.size(); ++index)
        snapshot.rawValues[static_cast<std::size_t>(indices[index])] =
            static_cast<float>(parameterSliders_[index].getValue());

    processor_.setDeveloperHostParameterOverride(snapshot);
    currentAppliedSlot_ = -1;
    updateWorkflowSummary();
    setWorkflowStatus(
        "Edited the effective Developer override; Host/APVTS remains underlying. Use Return Host "
        "to resume Host values.");
}

void FRAZILAudioProcessorEditor::returnToHost() {
    const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
    processor_.clearDeveloperHostParameterOverride();
    attachHostParameterControls();
    syncHostControls(processor_.getDeveloperHostParameterSnapshot());
    currentAppliedSlot_ = -1;
    updateWorkflowSummary();
    setWorkflowStatus("Returned to current Host/APVTS values; Developer override is inactive.");
}

void FRAZILAudioProcessorEditor::ensureHostParameterAttachmentMode() {
    const auto overrideActive = processor_.isDeveloperHostParameterOverrideActive();
    if (overrideActive == hostParameterControlsAttached()) {
        if (!overrideActive)
            return;

        const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
        detachHostParameterControls();
        syncHostControls(processor_.getDeveloperHostParameterSnapshot());
        return;
    }

    if (overrideActive) {
        const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
        detachHostParameterControls();
        syncHostControls(processor_.getDeveloperHostParameterSnapshot());
    } else {
        const juce::ScopedValueSetter<bool> updating(syncingDeveloperView_, true);
        attachHostParameterControls();
        syncHostControls(processor_.getDeveloperHostParameterSnapshot());
    }
}

void FRAZILAudioProcessorEditor::setComparisonMode(frazil::plugin::DeveloperComparisonMode mode) {
    processor_.setDeveloperComparisonMode(mode);
    updateComparisonButtons();
    updateWorkflowSummary();
}

void FRAZILAudioProcessorEditor::updateComparisonButtons() {
    const auto mode = processor_.getDeveloperComparisonMode();
    dryButton_.setToggleState(mode == frazil::plugin::DeveloperComparisonMode::dry,
                              juce::dontSendNotification);
    processedButton_.setToggleState(mode == frazil::plugin::DeveloperComparisonMode::processed,
                                    juce::dontSendNotification);
}

void FRAZILAudioProcessorEditor::updateWorkflowSummary() {
    if (currentAppliedSlot_ >= 0 && !processor_.isDeveloperHostParameterOverrideActive())
        currentAppliedSlot_ = -1;

    const auto slotState = [](const ABState& state) {
        return state.captured ? "Captured" : "Empty";
    };
    juce::String current = "Host";
    if (currentAppliedSlot_ >= 0)
        current = juce::String("Developer ") +
                  juce::String::charToString(static_cast<juce_wchar>('A' + currentAppliedSlot_));
    else if (processor_.isDeveloperHostParameterOverrideActive())
        current = "Developer override";

    const auto stateBoundary = processor_.isDeveloperHostParameterOverrideActive()
                                   ? "  |  Host/APVTS remains underlying"
                                   : "";
    workflowStateLabel_.setText(
        juce::String("A: ") + slotState(abStates_[0]) + "  |  B: " + slotState(abStates_[1]) +
            "  |  Current: " + current + "  |  " +
            comparisonModeName(processor_.getDeveloperComparisonMode()) + stateBoundary +
            "  |  temporary / not serialized / not a preset",
        juce::dontSendNotification);
}

void FRAZILAudioProcessorEditor::captureSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(abStates_.size()))
        return;

    auto& slot = abStates_[static_cast<std::size_t>(slotIndex)];
    slot.state = captureCurrentExperiment();
    slot.captured = true;
    updateWorkflowSummary();
    setWorkflowStatus("Captured temporary A/B slot " +
                      juce::String::charToString(static_cast<juce_wchar>('A' + slotIndex)) +
                      " including Host and Water experiment state. It is not serialized.");
}

void FRAZILAudioProcessorEditor::applySlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= static_cast<int>(abStates_.size()))
        return;

    const auto& slot = abStates_[static_cast<std::size_t>(slotIndex)];
    if (!slot.captured) {
        setWorkflowStatus("Slot " +
                          juce::String::charToString(static_cast<juce_wchar>('A' + slotIndex)) +
                          " is empty; capture it before applying.");
        return;
    }

    applyExperimentSnapshot(slot.state, slotIndex);
    setWorkflowStatus("Applied temporary slot " +
                      juce::String::charToString(static_cast<juce_wchar>('A' + slotIndex)) +
                      " including Host, Water experiment and comparison state.");
}

void FRAZILAudioProcessorEditor::resetHostParameters() {
    frazil::plugin::DeveloperHostParameterSnapshot defaults;
    for (std::size_t parameterIndex = 0; parameterIndex < kHostParameterIds.size();
         ++parameterIndex) {
        const auto* id = kHostParameterIds[parameterIndex];
        if (const auto* parameter = processor_.parameters.getParameter(id))
            defaults.rawValues[parameterIndex] =
                processor_.parameters.getParameterRange(id).convertFrom0to1(
                    parameter->getDefaultValue());
    }
    activateDeveloperOverride(defaults, -1);
    setWorkflowStatus(
        "Developer-reset all nine Host values to defaults; APVTS and Host were not edited.");
}

void FRAZILAudioProcessorEditor::resetExperimentControls() {
    syncExperimentControls({});
    currentAppliedSlot_ = -1;
    updateWorkflowSummary();
    setWorkflowStatus("Reset Water experiment controls; no Host parameter was changed.");
}

juce::String FRAZILAudioProcessorEditor::createExperimentConfig() const {
    juce::var root = new juce::DynamicObject();
    auto* rootObject = root.getDynamicObject();
    rootObject->setProperty("schema", "frazil.dev-experiment");
    rootObject->setProperty("schemaVersion", 1);
    rootObject->setProperty("source", "DEV-UI-001");
    rootObject->setProperty("stateBoundary", "not plugin state, preset, or Host automation");

    const auto current = captureCurrentExperiment();
    juce::var parameters = new juce::DynamicObject();
    auto* parametersObject = parameters.getDynamicObject();
    for (std::size_t index = 0; index < kHostParameterIds.size(); ++index)
        parametersObject->setProperty(kHostParameterIds[index], current.host.rawValues[index]);
    rootObject->setProperty("hostParameters", parameters);

    juce::var water = new juce::DynamicObject();
    auto* waterObject = water.getDynamicObject();
    waterObject->setProperty("model", waterModelName(current.water.model));
    waterObject->setProperty("size", current.water.size);
    waterObject->setProperty("motion", current.water.motion);
    rootObject->setProperty("waterExperiment", water);

    juce::var workflow = new juce::DynamicObject();
    auto* workflowObject = workflow.getDynamicObject();
    workflowObject->setProperty("comparisonMode", comparisonModeName(current.comparisonMode));
    workflowObject->setProperty(
        "hostStateSource",
        processor_.isDeveloperHostParameterOverrideActive() ? "developer-override" : "apvts");
    workflowObject->setProperty("temporary", true);
    rootObject->setProperty("developerWorkflow", workflow);

    const auto runtime = processor_.getDeveloperDiagnosticsSnapshot();
    juce::var runtimeObject = new juce::DynamicObject();
    auto* runtimeProperties = runtimeObject.getDynamicObject();
    runtimeProperties->setProperty("sampleRateHz", runtime.sampleRateHz);
    runtimeProperties->setProperty("preparedBlockSize", runtime.preparedBlockSize);
    runtimeProperties->setProperty("latestBlockSize", runtime.latestBlockSize);
    runtimeProperties->setProperty("channelCount", runtime.channelCount);
    rootObject->setProperty("runtime", runtimeObject);

    return juce::JSON::toString(root, false, 6);
}

void FRAZILAudioProcessorEditor::copyExperimentConfig() {
    juce::SystemClipboard::copyTextToClipboard(createExperimentConfig());
    setWorkflowStatus("Copied explicit experiment config to the clipboard.");
}

void FRAZILAudioProcessorEditor::exportExperimentConfig() {
    configFileChooser_ = std::make_unique<juce::FileChooser>(
        "Export FRAZIL developer experiment config",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("frazil-experiment.json"),
        "*.json");
    configFileChooser_->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
            const auto outputFile = chooser.getResult();
            if (outputFile == juce::File()) {
                setWorkflowStatus("Export cancelled.");
            } else if (auto outputStream = outputFile.createOutputStream()) {
                outputStream->setPosition(0);
                outputStream->truncate();
                outputStream->writeText(createExperimentConfig(), false, false, "\n");
                setWorkflowStatus("Exported experiment config: " + outputFile.getFileName());
            } else {
                setWorkflowStatus("Export failed: could not open the selected file.");
            }
            configFileChooser_.reset();
        });
    setWorkflowStatus("Choose a location for the explicit experiment config export.");
}

void FRAZILAudioProcessorEditor::setWorkflowStatus(const juce::String& text) {
    workflowStatusLabel_.setText(text, juce::dontSendNotification);
}

void FRAZILAudioProcessorEditor::timerCallback() {
    ensureHostParameterAttachmentMode();
    const auto diagnostics = processor_.getDeveloperDiagnosticsSnapshot();
    const auto host = processor_.getDeveloperHostParameterSnapshot();
    const auto routingIndex = static_cast<int>(std::lround(host.rawValues[static_cast<std::size_t>(
        frazil::plugin::DeveloperHostParameter::routingMode)]));
    const juce::StringArray routingNames{"Parallel", "Water -> Ice", "Ice -> Water"};
    const auto routing = routingNames[juce::jlimit(0, routingNames.size() - 1, routingIndex)];
    diagnosticsLabel_.setText(
        juce::String::formatted(
            "RUNTIME  %.1f kHz  |  prepared max %d  |  latest %d  |  %d ch  |  route %s\n"
            "Input  peak %.4f  RMS %.4f   |   Output  peak %.4f  RMS %.4f\n"
            "Finite: %s  |  Host snapshot is represented by the controls above",
            diagnostics.sampleRateHz / 1000.0f, diagnostics.preparedBlockSize,
            diagnostics.latestBlockSize, diagnostics.channelCount, routing.toRawUTF8(),
            diagnostics.inputPeak, diagnostics.inputRms, diagnostics.outputPeak,
            diagnostics.outputRms, diagnostics.finite ? "yes" : "NO"),
        juce::dontSendNotification);
    updateWorkflowSummary();
}

void FRAZILAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(kBackground);
    graphics.setColour(kAccent);
    graphics.fillRect(getLocalBounds().removeFromTop(4));

    auto content = getLocalBounds().reduced(20);
    auto body = content.withTop(92).withBottom(content.getBottom() - 50);
    auto left = body.removeFromLeft(static_cast<int>(body.getWidth() * 0.60f)).reduced(10);
    auto right = body.reduced(10);

    graphics.setColour(kPanel);
    graphics.fillRoundedRectangle(left.toFloat(), 10.0f);
    graphics.fillRoundedRectangle(right.toFloat(), 10.0f);
    graphics.setColour(kPanelBorder);
    graphics.drawRoundedRectangle(left.toFloat(), 10.0f, 1.0f);
    graphics.drawRoundedRectangle(right.toFloat(), 10.0f, 1.0f);
}

void FRAZILAudioProcessorEditor::resized() {
    auto content = getLocalBounds().reduced(20);
    auto header = content.removeFromTop(62);
    titleLabel_.setBounds(header.removeFromTop(32));
    subtitleLabel_.setBounds(header);

    auto footer = content.removeFromBottom(36);
    workflowStatusLabel_.setBounds(footer);

    auto body = content;
    auto left = body.removeFromLeft(static_cast<int>(body.getWidth() * 0.60f)).reduced(22, 16);
    auto right = body.reduced(22, 16);

    hostParametersLabel_.setBounds(left.removeFromTop(24));
    auto switches = left.removeFromTop(34);
    waterEnabledButton_.setBounds(switches.removeFromLeft(switches.getWidth() / 2).reduced(2));
    iceEnabledButton_.setBounds(switches.reduced(2));
    auto route = left.removeFromTop(30);
    routingModeBox_.setBounds(route.removeFromRight(180).reduced(2));
    routingModeBox_.setTextWhenNothingSelected("Routing Mode");

    auto grid = left.reduced(0, 8);
    const auto cellWidth = grid.getWidth() / 3;
    const auto cellHeight = grid.getHeight() / 2;
    for (std::size_t index = 0; index < parameterSliders_.size(); ++index) {
        const auto row = static_cast<int>(index / 3);
        const auto column = static_cast<int>(index % 3);
        auto cell = juce::Rectangle<int>(grid.getX() + column * cellWidth,
                                         grid.getY() + row * cellHeight, cellWidth, cellHeight)
                        .reduced(4);
        parameterLabels_[index].setBounds(cell.removeFromTop(22));
        parameterSliders_[index].setBounds(cell);
    }

    experimentLabel_.setBounds(right.removeFromTop(24));
    auto modelRow = right.removeFromTop(34);
    waterModelLabel_.setBounds(modelRow.removeFromLeft(72));
    waterModelBox_.setBounds(modelRow.reduced(2));
    auto sizeRow = right.removeFromTop(90);
    waterSizeLabel_.setBounds(sizeRow.removeFromTop(22));
    waterSizeSlider_.setBounds(sizeRow);
    auto motionRow = right.removeFromTop(90);
    waterMotionLabel_.setBounds(motionRow.removeFromTop(22));
    waterMotionSlider_.setBounds(motionRow);

    workflowLabel_.setBounds(right.removeFromTop(24));
    auto workflowState = right.removeFromTop(40);
    workflowStateLabel_.setBounds(workflowState.reduced(2));
    auto workflow = right.removeFromTop(96);
    const auto buttonWidth = workflow.getWidth() / 4;
    const auto buttonHeight = workflow.getHeight() / 3;
    std::array<juce::Button*, 11> buttons{
        &captureAButton_,   &applyAButton_,      &captureBButton_,
        &applyBButton_,     &dryButton_,         &processedButton_,
        &returnHostButton_, &resetHostButton_,   &resetExperimentButton_,
        &copyConfigButton_, &exportConfigButton_};
    for (std::size_t index = 0; index < buttons.size(); ++index) {
        const auto row = static_cast<int>(index / 4);
        const auto column = static_cast<int>(index % 4);
        buttons[index]->setBounds(juce::Rectangle<int>(workflow.getX() + column * buttonWidth,
                                                       workflow.getY() + row * buttonHeight,
                                                       buttonWidth, buttonHeight)
                                      .reduced(2));
    }

    diagnosticsLabel_.setBounds(right.removeFromTop(108));
}

#else

FRAZILAudioProcessorEditor::FRAZILAudioProcessorEditor(FRAZILAudioProcessor& processor)
    : AudioProcessorEditor(processor), processor_(processor) {
    setSize(640, 360);
}

FRAZILAudioProcessorEditor::~FRAZILAudioProcessorEditor() = default;

void FRAZILAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff101820));
    graphics.setColour(juce::Colours::white);
    graphics.setFont(juce::FontOptions(28.0f));
    graphics.drawFittedText("FRAZIL", getLocalBounds().reduced(32), juce::Justification::centred,
                            1);
    graphics.setFont(juce::FontOptions(14.0f));
    graphics.drawFittedText("Production editor pending", getLocalBounds().reduced(32).withTop(190),
                            juce::Justification::centred, 1);
}

void FRAZILAudioProcessorEditor::resized() {}

#endif
