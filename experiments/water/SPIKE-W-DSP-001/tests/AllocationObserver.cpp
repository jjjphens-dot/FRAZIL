#include "AllocationObserver.h"

#include <cstdlib>
#include <new>
#if defined(_WIN32)
#include <malloc.h>
#endif

// Test-only global instrumentation is required by replaceable allocation entry points.
// No such state is linked into DSP, renderer or plugin. Unlike a production mutable
// global, this observes the single-thread test scope and does not control the algorithm.
namespace allocationtest {
thread_local bool observing = false;
thread_local std::size_t allocations = 0;
void record() noexcept {
    if (observing)
        ++allocations;
}
} // namespace allocationtest
void* operator new(std::size_t n) {
    if (auto* p = std::malloc(n ? n : 1)) {
        allocationtest::record();
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
        allocationtest::record();
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
