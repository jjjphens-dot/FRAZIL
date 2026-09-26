#pragma once
#include <cstddef>

// Linked only into isolated allocation-measurement executables.
namespace allocationtest {
extern thread_local bool observing;
extern thread_local std::size_t allocations;
} // namespace allocationtest
