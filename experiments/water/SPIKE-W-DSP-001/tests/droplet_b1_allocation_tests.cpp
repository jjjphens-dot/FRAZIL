#include "DropletB1TestSupport.h"
#include "dsp/FlowD1.h"

#include <cstdlib>
#include <memory>
#include <new>
#if defined(_WIN32)
#include <malloc.h>
#endif

// Test-only global instrumentation is required by replaceable allocation entry points.
// No such state is linked into DSP, renderer or plugin. Unlike a production mutable
// global, this observes the single-thread test scope and does not control the algorithm.
namespace {
thread_local bool observing = false;
thread_local std::size_t allocations = 0;
void record() noexcept {
    if (observing)
        ++allocations;
}
} // namespace
void* operator new(std::size_t n) {
    if (auto* p = std::malloc(n ? n : 1)) {
        record();
        return p;
    }
    throw std::bad_alloc{};
}
void* operator new[](std::size_t n) {
    return ::operator new(n);
}
void operator delete(void* p) noexcept {
    std::free(p);
}
void operator delete[](void* p) noexcept {
    std::free(p);
}
void operator delete(void* p, std::size_t) noexcept {
    std::free(p);
}
void operator delete[](void* p, std::size_t) noexcept {
    std::free(p);
}
#if defined(_WIN32)
void* operator new(std::size_t n, std::align_val_t a) {
    if (auto* p = _aligned_malloc(n ? n : 1, static_cast<std::size_t>(a))) {
        record();
        return p;
    }
    throw std::bad_alloc{};
}
void* operator new[](std::size_t n, std::align_val_t a) {
    return ::operator new(n, a);
}
void operator delete(void* p, std::align_val_t) noexcept {
    _aligned_free(p);
}
void operator delete[](void* p, std::align_val_t) noexcept {
    _aligned_free(p);
}
void operator delete(void* p, std::size_t, std::align_val_t) noexcept {
    _aligned_free(p);
}
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept {
    _aligned_free(p);
}
#endif
int main() {
    using namespace frazil::water::research;
    b1test::Checks check;
    for (double rate : {44100., 48000., 96000.}) {
        auto processor = std::make_unique<DropletB1>();
        auto pool = std::make_unique<DropletB1VoicePool>();
        DropletB1Config c;
        c[B1Parameter::radius] = 7;
        c[B1Parameter::persistence] = 4;
        c[B1Parameter::rise] = .1;
        check(processor->prepare({rate, 42}, c), "allocation fixture prepare");
        pool->prepare(rate, 16);
        const auto e = b1test::event(rate, c);
        allocations = 0;
        observing = true;
        double sum{};
        for (int n = 0; n < int(rate * 2); ++n) {
            if (n % 500 == 0)
                pool->request(e);
            if (n == 10000)
                processor->setEntrainmentProbability(.25);
            if (n == 20000)
                processor->reset();
            sum += processor->process(b1test::source(n, rate))[0] + pool->process()[0];
        }
        observing = false;
        check(allocations == 0 && std::isfinite(sum),
              "measured process/retarget/reset/steal allocation free");
        check(pool->counters().steals > 0, "allocation observer exercised overflow");
    }
    // Reuse the isolated allocation observer; no second global allocation implementation.
    for (double rate : {44100., 48000., 96000.}) {
        FlowD1 flow;
        check(flow.prepare({rate, 42}, {1, .005, .05}), "D1 allocation prepare");
        allocations = 0;
        observing = true;
        double sum = 0;
        for (int n = 0; n < int(rate * 2); ++n) {
            if (n == 20000)
                flow.reset();
            sum += flow.process({float(std::sin(n * .1)), 0}).transferred[0];
        }
        observing = false;
        check(allocations == 0 && std::isfinite(sum), "D1 process/reset allocation observation");
    }
    return check.failures ? 1 : 0;
}
