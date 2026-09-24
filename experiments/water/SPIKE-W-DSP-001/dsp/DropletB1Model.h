#pragma once
#include "DropletB1Config.h"
#include "physics/BubblePhysics.h"

#include <algorithm>
#include <optional>

namespace frazil::water::research {
struct DropletB1PhysicalState final {
    double equivalentRadiusMeters{}, frequencyHz{}, dampingPerSecond{}, naturalTauSeconds{};
    double physicalAmplitudeScale{}, effectiveDamping{}, renderAmplitudeScale{};
};
// Pure event-physics preparation. Source excitation is applied separately by the coupler.
struct DropletB1Model final {
    static constexpr double kReferenceRadiusMeters = .002;
    static constexpr double kMaximumLifetimeSeconds = 2;
    static DropletB1PhysicalState make(const DropletB1Config& config) noexcept {
        DropletB1PhysicalState p;
        p.equivalentRadiusMeters = config[B1Parameter::radius] * .001;
        p.frequencyHz = BubblePhysics::minnaertFrequency(p.equivalentRadiusMeters);
        p.dampingPerSecond = BubblePhysics::damping(p.equivalentRadiusMeters);
        p.naturalTauSeconds = 1 / p.dampingPerSecond;
        p.physicalAmplitudeScale = std::pow(p.equivalentRadiusMeters / kReferenceRadiusMeters, 1.5);
        p.effectiveDamping = p.dampingPerSecond / config[B1Parameter::persistence];
        p.renderAmplitudeScale =
            config[B1Parameter::amplitudePolicy] == 0 ? p.physicalAmplitudeScale : 1;
        return p;
    }
};
// PRODUCT_MAPPING, droplet-b1-offline-v1 candidate; intentionally no Motion destination.
inline std::optional<DropletB1Config> mapDropletB1(double size, double decay) noexcept {
    if (!std::isfinite(size) || !std::isfinite(decay) || size < 0 || size > 1 || decay < 0 ||
        decay > 1)
        return std::nullopt;
    DropletB1Config c;
    c[B1Parameter::radius] = .2 * std::pow(35., size);
    c[B1Parameter::persistence] = std::pow(4., 2 * decay - 1);
    return c;
}
} // namespace frazil::water::research
