#pragma once

#include "ControlDescriptor.h"
#include "render/ReadConfig.h"

#include <array>
#include <limits>

namespace frazil::water::preview {

using ProtectSettings = research::ProtectRenderConfig;
enum class ProtectId {
    depth,
    cap,
    attack,
    release,
    floor,
    epsilon,
    low,
    high,
    depthExponent,
    scoreExponent,
    off,
    count
};
enum class ProtectUnit { normalized, decibels, seconds, amplitude, ratio, detectorThreshold };

struct ProtectDescriptor final {
    ProtectId id;
    const char* key;
    const char* label;
    double minimum, maximum, step, initial;
    ProtectUnit unit;
    ControlLifecycle lifecycle;
    ControlVisibility visibility;
    const char* defaultSource{"PROTECT-EXP-001"};
};

// Existing research bounds only. D0 thresholds use amplitude 0..1; D1 uses dB ratio 0..100.
// Coupled thresholds and all prepared semantics are still checked by ResidualProtect::prepare.
inline constexpr std::array<ProtectDescriptor, static_cast<std::size_t>(ProtectId::count)>
    kProtectControls{{
        {ProtectId::depth, "depth", "Depth", 0, 1, .001, 0, ProtectUnit::normalized,
         ControlLifecycle::live, ControlVisibility::primary},
        {ProtectId::cap, "capDb", "Cap (dB)", 0, 12, .1, 9, ProtectUnit::decibels,
         ControlLifecycle::prepareRequired, ControlVisibility::primary},
        {ProtectId::attack, "attackSeconds", "Attack", .00025, .002, 0, .001, ProtectUnit::seconds,
         ControlLifecycle::prepareRequired, ControlVisibility::primary},
        {ProtectId::release, "releaseSeconds", "Release", .04, .2, 0, .08, ProtectUnit::seconds,
         ControlLifecycle::prepareRequired, ControlVisibility::primary},
        {ProtectId::floor, "floor", "Floor (amplitude)", 1e-8, .1, 0, 1e-4, ProtectUnit::amplitude,
         ControlLifecycle::prepareRequired, ControlVisibility::advanced},
        {ProtectId::epsilon, "epsilon", "Epsilon", std::numeric_limits<double>::denorm_min(), .1, 0,
         1e-8, ProtectUnit::amplitude, ControlLifecycle::prepareRequired,
         ControlVisibility::advanced},
        {ProtectId::low, "thresholdLow", "Threshold Low", 0, 100, 0, 1,
         ProtectUnit::detectorThreshold, ControlLifecycle::prepareRequired,
         ControlVisibility::advanced},
        {ProtectId::high, "thresholdHigh", "Threshold High", 0, 100, 0, 9,
         ProtectUnit::detectorThreshold, ControlLifecycle::prepareRequired,
         ControlVisibility::advanced},
        {ProtectId::depthExponent, "depthExponent", "Depth exponent", .1, 8, .01, 1,
         ProtectUnit::ratio, ControlLifecycle::prepareRequired, ControlVisibility::advanced},
        {ProtectId::scoreExponent, "scoreExponent", "Score exponent", .1, 8, .01, 1,
         ProtectUnit::ratio, ControlLifecycle::prepareRequired, ControlVisibility::advanced},
        {ProtectId::off, "offSeconds", "OFF transition", .001, .1, 0, .01, ProtectUnit::seconds,
         ControlLifecycle::prepareRequired, ControlVisibility::advanced},
    }};

inline double protectValue(const ProtectSettings& settings, ProtectId id) noexcept {
    const auto& c = settings.gain;
    switch (id) {
    case ProtectId::depth:
        return settings.depth;
    case ProtectId::cap:
        return c.capDb;
    case ProtectId::attack:
        return c.attackSeconds;
    case ProtectId::release:
        return c.releaseSeconds;
    case ProtectId::floor:
        return c.floor;
    case ProtectId::epsilon:
        return c.epsilon;
    case ProtectId::low:
        return c.thresholdLow;
    case ProtectId::high:
        return c.thresholdHigh;
    case ProtectId::depthExponent:
        return c.depthExponent;
    case ProtectId::scoreExponent:
        return c.scoreExponent;
    case ProtectId::off:
        return c.offSeconds;
    case ProtectId::count:
        break;
    }
    return 0;
}
inline double protectMaximum(const ProtectDescriptor& spec, research::ProtectScore score) noexcept {
    return spec.unit == ProtectUnit::detectorThreshold &&
                   score == research::ProtectScore::difference
               ? 1.0
               : spec.maximum;
}
inline bool setProtectValue(ProtectSettings& settings, ProtectId id, double value) noexcept {
    const auto index = static_cast<std::size_t>(id);
    if (index >= kProtectControls.size())
        return false;
    const auto& spec = kProtectControls[index];
    if (!std::isfinite(value) || value < spec.minimum ||
        value > protectMaximum(spec, settings.gain.score))
        return false;
    auto& c = settings.gain;
    switch (id) {
    case ProtectId::depth:
        settings.depth = value;
        break;
    case ProtectId::cap:
        c.capDb = value;
        break;
    case ProtectId::attack:
        c.attackSeconds = value;
        break;
    case ProtectId::release:
        c.releaseSeconds = value;
        break;
    case ProtectId::floor:
        c.floor = value;
        break;
    case ProtectId::epsilon:
        c.epsilon = value;
        break;
    case ProtectId::low:
        c.thresholdLow = value;
        break;
    case ProtectId::high:
        c.thresholdHigh = value;
        break;
    case ProtectId::depthExponent:
        c.depthExponent = value;
        break;
    case ProtectId::scoreExponent:
        c.scoreExponent = value;
        break;
    case ProtectId::off:
        c.offSeconds = value;
        break;
    case ProtectId::count:
        return false;
    }
    return true;
}
inline bool sameProtect(const ProtectSettings& a, const ProtectSettings& b) noexcept {
    if (a.gain.score != b.gain.score || a.topology != b.topology)
        return false;
    for (const auto& spec : kProtectControls)
        if (protectValue(a, spec.id) != protectValue(b, spec.id))
            return false;
    return true;
}

// Message-thread convenience state, excluded from renderer configs. Detector switching restores
// the matching domain, never reinterprets dB as amplitude. Initial enable recall is a UI
// convenience.
struct ProtectCalibration final {
    double low{}, high{};
};
struct ProtectMemory final {
    ProtectCalibration difference{.010, .120}, logRatio{1, 9};
    double lastNonzeroDepth{.5};
    research::FluidProtectTopology fluidTopology{research::FluidProtectTopology::whole};
};
inline bool validProtectMemory(const ProtectMemory& memory) noexcept {
    const auto valid = [](ProtectCalibration calibration, double maximum) {
        return std::isfinite(calibration.low) && std::isfinite(calibration.high) &&
               calibration.low >= 0 && calibration.low < calibration.high &&
               calibration.high <= maximum;
    };
    return valid(memory.difference, 1) && valid(memory.logRatio, 100) &&
           std::isfinite(memory.lastNonzeroDepth) && memory.lastNonzeroDepth > 0 &&
           memory.lastNonzeroDepth <= 1;
}
} // namespace frazil::water::preview
