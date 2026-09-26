#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/FlowD1Trajectory.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>

using namespace frazil::water::research;

// Offline evidence exporter: actual source/trajectory classes, no alternate DSP path.
// The Python study owns independent interpolation oracles, not this executable.
int main(int argc, char** argv) {
    if (argc != 2 || !std::filesystem::create_directory(argv[1])) {
        std::cerr << "Supply a new output directory under an existing parent.\n";
        return 1;
    }
    const std::filesystem::path root{argv[1]};
    std::ofstream spec(root / "authority.json");
    const auto bRadius = kB1Parameters[static_cast<std::size_t>(B1Parameter::radius)].minimum;
    spec << std::setprecision(17) << "{\"a1_min_radius_mm\":" << kA1RadiusMinMm.minimum
         << ",\"b1_min_radius_mm\":" << bRadius << ",\"pressure_pa\":" << BubblePhysics::kPressurePa
         << ",\"density_kg_m3\":" << BubblePhysics::kDensityKgM3
         << ",\"gamma\":" << BubblePhysics::kGamma
         << ",\"a1_f0_hz\":" << BubbleA1Model::frequency(kA1RadiusMinMm.minimum * .001)
         << ",\"b1_f0_hz\":" << BubblePhysics::minnaertFrequency(bRadius * .001)
         << ",\"sound_speed_mps\":" << FlowD1Model::referenceSoundSpeedMps
         << ",\"maximum_path_m\":" << kD1ExcessPath.maximum << "}\n";
    if (!spec)
        return 1;
    for (int rate : {44100, 48000, 96000}) {
        for (bool edge : {false, true}) {
            BubbleA1Config ac;
            DropletB1Config bc;
            if (edge) {
                ac.radiusMinMm = kA1RadiusMinMm.minimum;
                ac.radiusMaxMm = kA1RadiusMaxMm.minimum;
                ac.populationGamma = kA1PopulationGamma.maximum;
                ac.riseXi = kA1RiseXi.maximum;
                bc[B1Parameter::radius] = bRadius;
                bc[B1Parameter::rise] =
                    kB1Parameters[static_cast<std::size_t>(B1Parameter::rise)].maximum;
            }
            BubbleA1 a;
            DropletB1 b;
            FlowD1Trajectory trajectory;
            const ResearchConfig research{double(rate), 42};
            if (!a.prepare(research, ac) || !b.prepare(research, bc) ||
                !trajectory.prepare(research, {1, .005, .05}))
                return 1;
            std::ofstream output(root /
                                 (std::to_string(rate) + (edge ? "-edge.csv" : "-reference.csv")));
            output << std::setprecision(17) << "path_m,a_left,a_right,b_left,b_right\n";
            // Same physical gated source at each rate; synthetic engineering fixture,
            // never presented as a recording or musical-pad listening evidence.
            for (int n = 0; n < rate; ++n) {
                const double t = double(n) / rate;
                const bool gate = t >= .05 && std::fmod(t - .05, .1) < .02 && t < .85;
                const float x = gate ? float(.4 * std::sin(2 * std::numbers::pi * 997 * t)) : 0.f;
                const StereoFrame input{x, -.5f * x};
                const auto av = a.process(input);
                const auto bv = b.process(input);
                output << trajectory.process() << ',' << av[0] << ',' << av[1] << ',' << bv[0]
                       << ',' << bv[1] << '\n';
            }
            if (!output || a.requested() == 0 || b.counters().admitted == 0)
                return 1;
        }
    }
    return 0;
}
