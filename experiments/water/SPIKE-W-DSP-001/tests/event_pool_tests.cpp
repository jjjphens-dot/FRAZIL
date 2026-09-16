#include "dsp/detail/EventVoicePool.h"

#include <iostream>
#include <limits>

using namespace frazil::water::research;

int main() {
    int failures{};
    const auto check = [&](bool ok) { failures += ok ? 0 : 1; };
    detail::EventVoicePool pool;
    const auto checkInactive = [&] {
        pool.trigger({1.0f, 0.0f}, 999);
        check(pool.process(.2) == StereoFrame{});
        check(pool.activeVoices() == 0 && pool.events() == 0 && pool.steals() == 0);
    };
    checkInactive();
    for (auto invalid : {std::size_t{0}, detail::EventVoicePool::kCapacity + 1,
                         std::numeric_limits<std::size_t>::max()}) {
        check(pool.prepare(48000, 250, 2800, .07, 16));
        pool.trigger({1.0f, 0.0f}, 0);
        check(pool.activeVoices() == 1);
        check(!pool.prepare(48000, 250, 2800, .07, invalid));
        checkInactive();
        pool.reset();
        checkInactive();
        for (auto valid : {std::size_t{1}, detail::EventVoicePool::kCapacity}) {
            check(pool.prepare(48000, 250, 2800, .07, valid));
            for (std::size_t i = 0; i < valid + 3; ++i)
                pool.trigger({1.0f, 0.0f}, i);
            check(pool.activeVoices() == valid && pool.steals() == 3);
            const auto output = pool.process(.2);
            check(std::isfinite(output[0]) && output[0] != 0.0f && output[1] == 0.0f);
            pool.reset();
            check(pool.activeVoices() == 0 && pool.events() == 0);
        }
    }
    std::cout << "event pool failures=" << failures << '\n';
    return failures ? 1 : 0;
}
