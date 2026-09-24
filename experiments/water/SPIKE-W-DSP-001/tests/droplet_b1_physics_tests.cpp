#include "DropletB1TestSupport.h"
using namespace frazil::water::research;
int main() {
    b1test::Checks check;
    const auto frequency = [](double r) {
        return std::sqrt(3.L * 1.4L * 101325.L / 998.L) / (2 * std::numbers::pi_v<long double> * r);
    };
    const auto damping = [](double r) { return .13L / r + .0072L / (r * std::sqrt(r)); };
    for (double mm : {.2, .355, 1., 2., 4., 7.}) {
        const double r = mm * .001;
        check(b1test::near(BubblePhysics::minnaertFrequency(r), double(frequency(r))),
              "frequency independent SI oracle");
        check(b1test::near(BubblePhysics::damping(r), double(damping(r))),
              "damping independent oracle");
    }
    check(std::abs(BubblePhysics::minnaertFrequency(.000355) / 8660 - 1) < .1,
          "Phillips scale anchor, not exact fit");
    double lastFrequency = 1e10, lastDamping = 1e10;
    for (int i = 0; i <= 100; ++i) {
        const double r = (.2 + 6.8 * i / 100) * .001;
        const auto f = BubblePhysics::minnaertFrequency(r), d = BubblePhysics::damping(r);
        check(f < lastFrequency && d < lastDamping, "monotone radius physics");
        lastFrequency = f;
        lastDamping = d;
    }
    const double maximumLifetime = -std::log(1e-5) * 4 / double(damping(.007));
    check(maximumLifetime + 1. / 44100 < DropletB1Model::kMaximumLifetimeSeconds,
          "derived lifetime guard");
    for (double rate : {44100., 48000., 96000.})
        for (double mm : {.2, .355, 2., 7.})
            for (double persistence : {.25, 1., 4.})
                for (double xi : {0., .05, .1}) {
                    DropletB1Config c;
                    c[B1Parameter::radius] = mm;
                    c[B1Parameter::persistence] = persistence;
                    c[B1Parameter::rise] = xi;
                    c[B1Parameter::tail] = -100;
                    auto e = b1test::event(rate, c);
                    check(b1test::near(e.physics.physicalAmplitudeScale, std::pow(mm / 2, 1.5)),
                          "formation radius scaling");
                    DropletB1BubbleVoice voice;
                    voice.start(e, rate);
                    const double r = mm * .001, w0 = 2 * std::numbers::pi * double(frequency(r)),
                                 d = double(damping(r)) / persistence;
                    const double acc = w0 * xi * double(damping(r)),
                                 cap = std::min(w0 * std::sqrt(2.), .9 * std::numbers::pi * rate);
                    const double tc = acc > 0 ? (cap - w0) / acc : 1e100;
                    const double wr = 2 * std::numbers::pi * double(frequency(.002));
                    for (int n = 0; voice.active(); ++n) {
                        const double t = n / rate, u = std::min(t, tc);
                        const double phase = w0 * u + .5 * acc * u * u + cap * std::max(0., t - tc);
                        const double w = std::min(w0 + acc * t, cap), wp = t < tc ? acc : 0;
                        const double a = std::pow(mm / 2, 1.5) * .5 * .15;
                        const double expected = a * std::exp(-d * t) * r * r /
                                                (.002 * .002 * wr * wr) *
                                                ((d * d - w * w) * std::sin(phase) +
                                                 (wp - 2 * d * w) * std::cos(phase));
                        const auto y = voice.process();
                        check(b1test::near(y[0], expected, 2e-8) &&
                                  b1test::near(y[1], -.3 * expected, 2e-8),
                              "analytic volume acceleration / chirp cap / stereo");
                        check(n < rate * 2, "finite lifetime");
                    }
                    c[B1Parameter::emission] = 1;
                    c[B1Parameter::rise] = 0;
                    e = b1test::event(rate, c);
                    voice.start(e, rate);
                    for (int n = 0; n < 30; ++n) {
                        const auto y = voice.process();
                        check(b1test::near(y[0], e.physics.renderAmplitudeScale * .5 * .15 *
                                                     std::exp(-d * n / rate) *
                                                     std::sin(w0 * n / rate)),
                              "displacement ablation oracle");
                    }
                }
    for (double size : {0., .5, 1.})
        for (double decay : {0., .5, 1.}) {
            const auto c = mapDropletB1(size, decay);
            check(c && c->valid(), "offline map domain");
            check(b1test::near((*c)[B1Parameter::radius],
                               std::exp(std::log(.2) + size * std::log(35.))),
                  "log Size independent expression");
            check((*c)[B1Parameter::admission] == 1 && (*c)[B1Parameter::gain] == .15,
                  "mapping ownership");
        }
    return check.failures ? 1 : 0;
}
