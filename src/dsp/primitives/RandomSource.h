#pragma once

#include <cstdint>

class RandomSource final {
  public:
    using Seed = std::uint32_t;

    static constexpr Seed kDefaultSeed = 0x6d2b79f5u;

    explicit RandomSource(Seed seed = kDefaultSeed) noexcept;

    void reseed(Seed seed) noexcept;
    Seed nextUInt() noexcept;
    float nextUnipolar() noexcept;

    // Derives per-instance seeds without shared state or dependence on construction order.
    static Seed deriveInstanceSeed(Seed baseSeed, std::uint64_t instanceIndex) noexcept;

  private:
    static Seed sanitiseSeed(Seed seed) noexcept;

    // Xorshift state, advanced only by the owning caller (normally the audio thread).
    Seed state_{};
};
