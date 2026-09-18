#pragma once

#include "dsp/primitives/RandomSource.h"

namespace frazil::water::research {
// Experiment identifiers are stable seed domains, not Host parameters or persisted state.
enum class RandomDomain : std::uint64_t { bubble = 1, droplet = 2, flow = 3, modalMotion = 4 };

struct ResearchConfig final {
    double sampleRateHz{48000.0};
    RandomSource::Seed baseSeed{20260916u};

    [[nodiscard]] RandomSource::Seed seedFor(RandomDomain domain) const noexcept {
        return RandomSource::deriveInstanceSeed(baseSeed, static_cast<std::uint64_t>(domain));
    }
};

} // namespace frazil::water::research
