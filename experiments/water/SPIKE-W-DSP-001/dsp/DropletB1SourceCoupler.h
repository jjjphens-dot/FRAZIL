#pragma once
#include "SharedExcitationAnalyzer.h"

#include <cstdint>

namespace frazil::water::research {
struct DropletB1VirtualImpact final {
    std::uint64_t sourceSample{};
    std::array<double, 2> carrier{}; // Signed same-frame unit linked-RMS direction.
    double sourceExcitation{}, sourceEnergy{}, onsetStrength{};
};
// REDUCED_PHYSICAL_MODEL: frozen musical evidence, never metric impact/fluid velocity.
struct DropletB1SourceCoupler final {
    static DropletB1VirtualImpact capture(std::uint64_t sample, double novelty,
                                          const SharedExcitationAnalyzer& source) noexcept {
        DropletB1VirtualImpact result;
        result.sourceSample = sample;
        result.sourceEnergy = source.state().fastPower;
        result.sourceExcitation = std::sqrt(result.sourceEnergy);
        result.onsetStrength = novelty;
        result.carrier = source.eventCarrier(true);
        if (result.sourceExcitation > 0)
            for (auto& channel : result.carrier)
                channel /= result.sourceExcitation;
        return result;
    }
};
} // namespace frazil::water::research
