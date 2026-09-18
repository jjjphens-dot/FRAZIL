#pragma once

#include "ResearchListeningCalibration.h"
#include "ResearchMappingAdapter.h"
#include "SessionMetadata.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

namespace frazil::water::preview {

enum class ChangeOrigin { soundLeadUI, engineeringUI, mapper, sessionLoad, reset };
enum class MappingStatus { unmapped, mapped, custom };

struct ControlOwnership final {
    ChangeOrigin origin{ChangeOrigin::reset};
    std::uint64_t revision{};
};

struct ResearchSessionState final {
    PreviewSettings engineering;
    ProtectMemory protectMemory;
    WaterExperimentState water;
    MonitorMode monitor{MonitorMode::processed};
    double monitorGainDb{-18.0};
    MappingStatus modelMapping{MappingStatus::mapped};
    bool customEngineering{};
    // Legacy imports retain raw targets until the user explicitly adopts research mapping.
    juce::String mappingRevision{ResearchWaterMacroMapper::revision.data()};
    std::array<MappingStatus, 3> macroMappings{MappingStatus::mapped, MappingStatus::mapped,
                                               MappingStatus::mapped};
    MappingStatus listeningCalibration{MappingStatus::mapped};
    double auditionETrimDb{18.0};
    std::array<ControlOwnership, kControls.size()> ownership{};
    ControlOwnership lastChange;
    SourceMetadata source;
    BuildMetadata build;
    std::optional<BuildMetadata> importedBuild;
    ResearchSessionState() {
        for (const auto macro : {MacroId::size, MacroId::motion, MacroId::decay})
            applyResearchMacro(engineering, water, macro);
        ResearchListeningCalibration::apply(engineering);
    }
};

inline bool sameResearchContext(const ResearchSessionState& a, const ResearchSessionState& b) {
    return a.water == b.water && a.monitor == b.monitor && a.monitorGainDb == b.monitorGainDb &&
           a.auditionETrimDb == b.auditionETrimDb && a.source == b.source &&
           a.mappingRevision == b.mappingRevision && a.macroMappings == b.macroMappings &&
           a.listeningCalibration == b.listeningCalibration && a.modelMapping == b.modelMapping &&
           a.protectMemory.fluidTopology == b.protectMemory.fluidTopology &&
           a.protectMemory.lastNonzeroDepth == b.protectMemory.lastNonzeroDepth &&
           a.protectMemory.difference.low == b.protectMemory.difference.low &&
           a.protectMemory.difference.high == b.protectMemory.difference.high &&
           a.protectMemory.logRatio.low == b.protectMemory.logRatio.low &&
           a.protectMemory.logRatio.high == b.protectMemory.logRatio.high;
}

constexpr const char* originName(ChangeOrigin origin) noexcept {
    switch (origin) {
    case ChangeOrigin::soundLeadUI:
        return "Sound Lead UI";
    case ChangeOrigin::engineeringUI:
        return "Engineering UI";
    case ChangeOrigin::mapper:
        return "Mapper";
    case ChangeOrigin::sessionLoad:
        return "Session Load";
    case ChangeOrigin::reset:
        return "Reset";
    }
    return "Unknown";
}

inline bool moduleActive(ControlGroup group, int composition) noexcept {
    if (composition < 0 || composition >= static_cast<int>(kModes.size()))
        return false;
    if (group == ControlGroup::modal)
        return composition == 1;
    if (group == ControlGroup::protect)
        return composition != 8;
    if (composition == 1 || composition == 8)
        return false;
    const std::string_view mode(kModes[static_cast<std::size_t>(composition)]);
    const char component = group == ControlGroup::bubble    ? 'a'
                           : group == ControlGroup::droplet ? 'b'
                                                            : 'd';
    return mode.find(component) != std::string_view::npos;
}

// Sole message-thread state owner. Views render snapshots and submit commands; no widget-to-widget
// links or DSP objects. The coordinator installs one synchronous observer and detaches it before
// destroying views. Audio processing only receives a validated applied value copy.
class ResearchSessionModel final {
  public:
    std::function<void()> onChange;
    const ResearchSessionState& draft() const noexcept {
        return draft_;
    }
    const ResearchSessionState& applied() const noexcept {
        return applied_;
    }
    std::uint64_t revision() const noexcept {
        return revision_;
    }

    std::size_t unappliedChanges() const noexcept {
        std::size_t count = draft_.engineering.mode != applied_.engineering.mode;
        for (std::size_t i = 0; i < kControls.size(); ++i)
            count += draft_.engineering.values[i] != applied_.engineering.values[i];
        count += draft_.water.size != applied_.water.size;
        count += draft_.water.motion != applied_.water.motion;
        count += draft_.water.decay != applied_.water.decay;
        count += draft_.mappingRevision != applied_.mappingRevision;
        count += draft_.macroMappings != applied_.macroMappings;
        count += draft_.listeningCalibration != applied_.listeningCalibration;
        for (const auto& spec : kProtectControls)
            count += protectValue(draft_.engineering.protect, spec.id) !=
                     protectValue(applied_.engineering.protect, spec.id);
        count += draft_.engineering.protect.gain.score != applied_.engineering.protect.gain.score;
        count += draft_.engineering.protect.topology != applied_.engineering.protect.topology;
        const auto& draftInactive =
            draft_.engineering.protect.gain.score == research::ProtectScore::difference
                ? draft_.protectMemory.logRatio
                : draft_.protectMemory.difference;
        const auto& appliedInactive =
            draft_.engineering.protect.gain.score == research::ProtectScore::difference
                ? applied_.protectMemory.logRatio
                : applied_.protectMemory.difference;
        count += draftInactive.low != appliedInactive.low;
        count += draftInactive.high != appliedInactive.high;
        count += draft_.protectMemory.fluidTopology != applied_.protectMemory.fluidTopology;
        count += draft_.protectMemory.lastNonzeroDepth != applied_.protectMemory.lastNonzeroDepth;
        return count;
    }
    bool dirty() const noexcept {
        return unappliedChanges() != 0;
    }
    bool dspDirty() const noexcept {
        auto pending = draft_.engineering.protect;
        pending.depth = applied_.engineering.protect.depth; // Live target, never requires prepare.
        return draft_.engineering.mode != applied_.engineering.mode ||
               draft_.engineering.values != applied_.engineering.values ||
               !sameProtect(pending, applied_.engineering.protect);
    }
    // Context checkpoint is the last Apply/Import/Recall, not a claim about saving to disk.
    // Monitor and trim are live in applied state, yet still change the session context.
    bool sessionDirty() const {
        return !sameResearchContext(draft_, contextCheckpoint_);
    }

    bool setEngineering(ControlId id, double value, ChangeOrigin origin) {
        const auto index = controlIndex(id);
        if (index >= kControls.size())
            return false;
        const auto& spec = kControls[index];
        if (!std::isfinite(value) || value < spec.minimum || value > spec.maximum ||
            (spec.valueType == ControlValueType::integer && value != std::floor(value)))
            return false;
        if (draft_.engineering.values[index] == value)
            return true;
        draft_.engineering.values[index] = value;
        draft_.customEngineering = true;
        if (const auto owner = macroOwner(id))
            draft_.macroMappings[static_cast<std::size_t>(*owner)] = MappingStatus::custom;
        if (ResearchListeningCalibration::owns(id))
            draft_.listeningCalibration = MappingStatus::custom;
        draft_.ownership[index] = {origin, revision_ + 1};
        changed(origin);
        return true;
    }
    bool setMacro(MacroId id, double value, ChangeOrigin origin) {
        if ((id != MacroId::size && id != MacroId::motion && id != MacroId::decay) ||
            !std::isfinite(value) || value < 0 || value > 1)
            return false;
        auto& target = id == MacroId::size     ? draft_.water.size
                       : id == MacroId::motion ? draft_.water.motion
                                               : draft_.water.decay;
        if (target == value)
            return true;
        target = value;
        if (draft_.mappingRevision == ResearchWaterMacroMapper::revision.data())
            mapMacro(id, origin);
        changed(origin); // Legacy-unmapped sessions retain raw DSP until explicit adoption.
        return true;
    }
    bool setComposition(int mode, ChangeOrigin origin) {
        if (mode < 0 || mode >= static_cast<int>(kModes.size()))
            return false;
        if (draft_.engineering.mode == mode)
            return true;
        if (mode == 1) {
            draft_.protectMemory.fluidTopology = draft_.engineering.protect.topology;
            draft_.engineering.protect.topology = research::FluidProtectTopology::whole;
        } else if (draft_.engineering.mode == 1) {
            draft_.engineering.protect.topology = draft_.protectMemory.fluidTopology;
        }
        draft_.engineering.mode = mode;
        draft_.water.model = mode == 1 ? WaterModel::resonant : WaterModel::fluid;
        draft_.modelMapping = mode <= 1 ? MappingStatus::mapped : MappingStatus::custom;
        changed(origin);
        return true;
    }
    void setModel(WaterModel model, ChangeOrigin origin) {
        if (model != WaterModel::fluid && model != WaterModel::resonant)
            return;
        setComposition(model == WaterModel::fluid ? 0 : 1, origin);
    }
    void returnModelToMapped() {
        setModel(draft_.water.model, ChangeOrigin::mapper);
    }

    void returnMacroToMapped(MacroId id) {
        if (id != MacroId::size && id != MacroId::motion && id != MacroId::decay)
            return;
        draft_.mappingRevision = ResearchWaterMacroMapper::revision.data();
        mapMacro(id, ChangeOrigin::mapper);
        changed(ChangeOrigin::mapper);
    }
    void returnAllToMapped() {
        draft_.mappingRevision = ResearchWaterMacroMapper::revision.data();
        for (const auto macro : {MacroId::size, MacroId::motion, MacroId::decay})
            mapMacro(macro, ChangeOrigin::mapper);
        // Composition is separately owned. Preserve ablations and unowned engineering controls.
        changed(ChangeOrigin::mapper);
    }
    void restoreListeningCalibration() {
        ResearchListeningCalibration::apply(draft_.engineering);
        draft_.listeningCalibration = MappingStatus::mapped;
        for (const auto id : ResearchListeningCalibration::controls)
            draft_.ownership[controlIndex(id)] = {ChangeOrigin::reset, revision_ + 1};
        changed(ChangeOrigin::reset);
    }

    bool setProtect(ProtectId id, double value, ChangeOrigin origin) {
        if (static_cast<std::size_t>(id) >= kProtectControls.size())
            return false;
        if (protectValue(draft_.engineering.protect, id) == value)
            return true;
        if (!setProtectValue(draft_.engineering.protect, id, value))
            return false;
        if (id == ProtectId::depth) {
            applied_.lastChange = {origin, revision_ + 1};
            applied_.engineering.protect.depth =
                value; // LIVE independently of other draft changes.
            if (value > 0)
                draft_.protectMemory.lastNonzeroDepth = applied_.protectMemory.lastNonzeroDepth =
                    value;
        } else if (id == ProtectId::low || id == ProtectId::high) {
            auto& calibration =
                draft_.engineering.protect.gain.score == research::ProtectScore::difference
                    ? draft_.protectMemory.difference
                    : draft_.protectMemory.logRatio;
            calibration = {draft_.engineering.protect.gain.thresholdLow,
                           draft_.engineering.protect.gain.thresholdHigh};
        }
        changed(origin);
        return true;
    }
    void setProtectEnabled(bool enabled, ChangeOrigin origin) {
        setProtect(ProtectId::depth, enabled ? draft_.protectMemory.lastNonzeroDepth : 0.0, origin);
    }
    void setDetector(research::ProtectScore score, ChangeOrigin origin) {
        if ((score != research::ProtectScore::difference &&
             score != research::ProtectScore::logRatio) ||
            score == draft_.engineering.protect.gain.score)
            return;
        draft_.engineering.protect.gain.score = score;
        const auto& calibration = score == research::ProtectScore::difference
                                      ? draft_.protectMemory.difference
                                      : draft_.protectMemory.logRatio;
        draft_.engineering.protect.gain.thresholdLow = calibration.low;
        draft_.engineering.protect.gain.thresholdHigh = calibration.high;
        changed(origin);
    }
    bool setTopology(research::FluidProtectTopology topology, ChangeOrigin origin) {
        if ((topology != research::FluidProtectTopology::whole &&
             topology != research::FluidProtectTopology::dropletExempt &&
             topology != research::FluidProtectTopology::dropletHalf) ||
            (draft_.engineering.mode == 1 && topology != research::FluidProtectTopology::whole))
            return false;
        if (topology == draft_.engineering.protect.topology)
            return true;
        draft_.engineering.protect.topology = topology;
        draft_.protectMemory.fluidTopology = topology;
        changed(origin);
        return true;
    }

    void setMonitor(MonitorMode mode, double gainDb) {
        if ((mode != MonitorMode::dry && mode != MonitorMode::processed &&
             mode != MonitorMode::residual) ||
            !std::isfinite(gainDb) || gainDb < -60 || gainDb > 0)
            return;
        if (draft_.monitor == mode && draft_.monitorGainDb == gainDb)
            return;
        draft_.monitor = applied_.monitor = mode;
        draft_.monitorGainDb = applied_.monitorGainDb = gainDb;
        applied_.lastChange = {ChangeOrigin::soundLeadUI, revision_ + 1};
        changed(ChangeOrigin::soundLeadUI);
    }
    void setAuditionTrim(double decibels) {
        if (!std::isfinite(decibels) || decibels < 0 || decibels > 36 ||
            draft_.auditionETrimDb == decibels)
            return;
        draft_.auditionETrimDb = applied_.auditionETrimDb = decibels;
        applied_.lastChange = {ChangeOrigin::soundLeadUI, revision_ + 1};
        changed(ChangeOrigin::soundLeadUI);
    }
    void setSource(const SourceMetadata& source) {
        draft_.source = applied_.source = source;
        applied_.lastChange = {ChangeOrigin::soundLeadUI, revision_ + 1};
        changed(ChangeOrigin::soundLeadUI);
    }
    // Coordinator must validate draft engineering config with the existing DSP authority first.
    void applyValidated() {
        applied_ = draft_;
        contextCheckpoint_ = draft_;
        notify();
    }
    void reset() {
        const auto source = draft_.source;
        draft_ = {};
        draft_.source = source;
        changed(ChangeOrigin::reset);
    }
    void capture(std::size_t slot) {
        if (slot < slots_.size())
            slots_[slot] = applied_;
    }
    const std::optional<ResearchSessionState>& slot(std::size_t index) const {
        return slots_.at(index);
    }
    void restoreValidated(const ResearchSessionState& state) {
        draft_ = state;
        draft_.importedBuild = state.build;
        draft_.build = {};
        for (auto& ownership : draft_.ownership)
            ownership = {ChangeOrigin::sessionLoad, revision_ + 1};
        draft_.lastChange = {ChangeOrigin::sessionLoad, ++revision_};
        applied_ = draft_;
        contextCheckpoint_ = draft_;
        notify();
    }

  private:
    void mapMacro(MacroId id, ChangeOrigin origin) {
        applyResearchMacro(draft_.engineering, draft_.water, id);
        draft_.macroMappings[static_cast<std::size_t>(id)] = MappingStatus::mapped;
        for (const auto& control : kControls)
            if (macroOwner(control.id) == id)
                draft_.ownership[controlIndex(control.id)] = {origin, revision_ + 1};
    }
    void changed(ChangeOrigin origin) {
        draft_.lastChange = {origin, ++revision_};
        notify();
    }
    void notify() {
        if (onChange)
            onChange();
    }
    ResearchSessionState draft_, applied_, contextCheckpoint_;
    std::array<std::optional<ResearchSessionState>, 2> slots_;
    std::uint64_t revision_{};
};
} // namespace frazil::water::preview
