#pragma once

#include <cmath>
#include <iostream>

struct TestContext final {
    int failures{};
};

inline void expect(TestContext& context, bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++context.failures;
    }
}

inline void expectNear(TestContext& context, float actual, float expected, float tolerance,
                       const char* description) {
    expect(context, std::abs(actual - expected) <= tolerance, description);
}
