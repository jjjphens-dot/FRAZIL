#pragma once

#include "PreviewSettings.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

namespace frazil::water::preview {

enum class WaterModel { fluid, resonant };
enum class MacroId { size, motion, decay };
enum class ChangeOrigin { soundLeadUI, engineeringUI, mapper, sessionLoad, reset };
enum class MappingStatus { unmapped, mapped, custom };

struct WaterExperimentState final {
    WaterModel model{WaterModel::fluid};
    double size{.5}, motion{.5};
    // DOC-W-DECAY-001 Revision C provisional experiment baseline, never a product default.
    double decay{.5};
    bool operator==(const WaterExperimentState&) const = default;
};

struct ControlOwnership final {
    ChangeOrigin origin{ChangeOrigin::reset};
    std::uint64_t revision{};
};

// This is the entire currently justified mapping interface. No frequency/rate/time curve exists.
struct WaterMacroMapper final {
    static int composition(WaterModel model) noexcept {
        return model == WaterModel::fluid ? 0 : 1;
    }
    static constexpr MappingStatus targets(MacroId) noexcept {
        return MappingStatus::unmapped;
    }
};

struct ResearchSessionState final {
    PreviewSettings engineering;
    WaterExperimentState water;
    MonitorMode monitor{MonitorMode::processed};
    double monitorGainDb{-12.0};
    MappingStatus modelMapping{MappingStatus::mapped};
    bool customEngineering{};
    std::array<ControlOwnership, kControls.size()> ownership{};
    ControlOwnership lastChange;
};

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
        return count;
    }
    bool dirty() const noexcept {
        return unappliedChanges() != 0;
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
        changed(origin); // No engineering target is modified or reverse-mapped.
        return true;
    }
    bool setComposition(int mode, ChangeOrigin origin) {
        if (mode < 0 || mode >= static_cast<int>(kModes.size()))
            return false;
        if (draft_.engineering.mode == mode)
            return true;
        draft_.engineering.mode = mode;
        draft_.water.model = mode == 1 ? WaterModel::resonant : WaterModel::fluid;
        draft_.modelMapping = mode <= 1 ? MappingStatus::mapped : MappingStatus::custom;
        changed(origin);
        return true;
    }
    void setModel(WaterModel model, ChangeOrigin origin) {
        if (model != WaterModel::fluid && model != WaterModel::resonant)
            return;
        setComposition(WaterMacroMapper::composition(model), origin);
    }
    void returnModelToMapped() {
        setModel(draft_.water.model, ChangeOrigin::mapper);
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
        changed(ChangeOrigin::soundLeadUI);
    }
    // Coordinator must validate draft engineering config with the existing DSP authority first.
    void applyValidated() {
        applied_ = draft_;
        notify();
    }
    void reset() {
        draft_ = {};
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
        for (auto& ownership : draft_.ownership)
            ownership = {ChangeOrigin::sessionLoad, revision_ + 1};
        draft_.lastChange = {ChangeOrigin::sessionLoad, ++revision_};
        applied_ = draft_;
        notify();
    }

  private:
    void changed(ChangeOrigin origin) {
        draft_.lastChange = {origin, ++revision_};
        notify();
    }
    void notify() {
        if (onChange)
            onChange();
    }
    ResearchSessionState draft_, applied_;
    std::array<std::optional<ResearchSessionState>, 2> slots_;
    std::uint64_t revision_{};
};
} // namespace frazil::water::preview
