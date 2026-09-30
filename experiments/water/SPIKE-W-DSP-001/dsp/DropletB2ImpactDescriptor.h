#pragma once
#include "DropletB1EntrainmentModel.h"
namespace frazil::water::research {
// Captured once. Center physics is kept separate from spatial rendering parameters.
struct DropletB2Event final {
    DropletB1Event center;
    double sourceExcitation{}, mappedExcitation{}, positiveSlope{}, detuneCents{}, maximumBeatHz{};
    int polarity{1};
    double releaseMs{};
    std::uint64_t dueSample{};
};
} // namespace frazil::water::research
