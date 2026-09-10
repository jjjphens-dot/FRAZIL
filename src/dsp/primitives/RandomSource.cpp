#include "RandomSource.h"

namespace {
constexpr RandomSource::Seed kNonZeroSeed = 0x1u;
constexpr unsigned kXorshiftLeftA = 13u;
constexpr unsigned kXorshiftRight = 17u;
constexpr unsigned kXorshiftLeftB = 5u;
constexpr std::uint64_t kSplitMixGamma = 0x9e3779b97f4a7c15ull;
constexpr std::uint64_t kSplitMixMultiplierA = 0xbf58476d1ce4e5b9ull;
constexpr std::uint64_t kSplitMixMultiplierB = 0x94d049bb133111ebull;
constexpr unsigned kSplitMixShiftA = 30u;
constexpr unsigned kSplitMixShiftB = 27u;
constexpr unsigned kSplitMixShiftC = 31u;
constexpr unsigned kSeedFoldShift = 32u;
} // namespace

RandomSource::RandomSource(Seed seed) noexcept {
    reseed(seed);
}

RandomSource::Seed RandomSource::sanitiseSeed(Seed seed) noexcept {
    return seed == 0 ? kNonZeroSeed : seed;
}

void RandomSource::reseed(Seed seed) noexcept {
    state_ = sanitiseSeed(seed);
}

RandomSource::Seed RandomSource::nextUInt() noexcept {
    // Preserve the xorshift32 sequence used by the deterministic render and unit fixtures.
    state_ ^= state_ << kXorshiftLeftA;
    state_ ^= state_ >> kXorshiftRight;
    state_ ^= state_ << kXorshiftLeftB;
    return state_;
}

float RandomSource::nextUnipolar() noexcept {
    constexpr float kInverseTwentyFourBitRange = 1.0f / 16777216.0f;
    return static_cast<float>(nextUInt() >> 8u) * kInverseTwentyFourBitRange;
}

RandomSource::Seed RandomSource::deriveInstanceSeed(Seed baseSeed,
                                                    std::uint64_t instanceIndex) noexcept {
    // SplitMix-style mixing only decorrelates per-instance seeds; it is not persistent state.
    std::uint64_t value = static_cast<std::uint64_t>(baseSeed) ^ (instanceIndex + kSplitMixGamma);
    value = (value ^ (value >> kSplitMixShiftA)) * kSplitMixMultiplierA;
    value = (value ^ (value >> kSplitMixShiftB)) * kSplitMixMultiplierB;
    value ^= value >> kSplitMixShiftC;
    return sanitiseSeed(static_cast<Seed>(value ^ (value >> kSeedFoldShift)));
}
