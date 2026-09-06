#include "RandomSource.h"

RandomSource::RandomSource(Seed seed) noexcept {
    reseed(seed);
}

RandomSource::Seed RandomSource::sanitiseSeed(Seed seed) noexcept {
    return seed == 0 ? 0x1u : seed;
}

void RandomSource::reseed(Seed seed) noexcept {
    state_ = sanitiseSeed(seed);
}

RandomSource::Seed RandomSource::nextUInt() noexcept {
    state_ ^= state_ << 13u;
    state_ ^= state_ >> 17u;
    state_ ^= state_ << 5u;
    return state_;
}

float RandomSource::nextUnipolar() noexcept {
    constexpr float kInverseTwentyFourBitRange = 1.0f / 16777216.0f;
    return static_cast<float>(nextUInt() >> 8u) * kInverseTwentyFourBitRange;
}

RandomSource::Seed RandomSource::deriveInstanceSeed(Seed baseSeed,
                                                    std::uint64_t instanceIndex) noexcept {
    std::uint64_t value =
        static_cast<std::uint64_t>(baseSeed) ^ (instanceIndex + 0x9e3779b97f4a7c15ull);
    value = (value ^ (value >> 30u)) * 0xbf58476d1ce4e5b9ull;
    value = (value ^ (value >> 27u)) * 0x94d049bb133111ebull;
    value ^= value >> 31u;
    return sanitiseSeed(static_cast<Seed>(value ^ (value >> 32u)));
}
