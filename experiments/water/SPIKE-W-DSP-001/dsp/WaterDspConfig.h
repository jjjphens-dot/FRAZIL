#pragma once

#include "dsp/primitives/RandomSource.h"

namespace frazil::water::research {
// Experiment identifiers are stable seed domains, not Host parameters or persisted state.
enum class RandomDomain : std::uint64_t {
    bubble = 1,
    droplet = 2,
    flow = 3,
    modalMotion = 4,
    dropletActivity = 5,
    bubbleA1 = 6, // Named historical domain; no stream is renumbered.
    dropletB1Identity = 7,
    dropletB1Admission = 8,
    dropletB1Jitter = 9, // Reserved, not consumed by B1 v1.
    flowD1 = 10 // Independent reduced-path trajectory; append-only identity.
};

struct ResearchConfig final {
    double sampleRateHz{48000.0};
    RandomSource::Seed baseSeed{20260916u};

    [[nodiscard]] RandomSource::Seed seedFor(RandomDomain domain) const noexcept {
        return RandomSource::deriveInstanceSeed(baseSeed, static_cast<std::uint64_t>(domain));
    }
};

} // namespace frazil::water::research
