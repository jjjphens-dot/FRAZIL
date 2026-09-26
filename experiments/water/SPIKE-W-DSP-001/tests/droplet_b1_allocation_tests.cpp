#include "AllocationObserver.h"
#include "DropletB1TestSupport.h"
#include "dsp/FlowD1.h"

#include <memory>

using allocationtest::allocations;
using allocationtest::observing;

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
