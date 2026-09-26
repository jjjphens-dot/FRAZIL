#include "dsp/BubbleA1.h"
#include "dsp/DropletB1.h"
#include "dsp/FlowD1.h"
#include "dsp/FlowD1Trajectory.h"
#include "dsp/FlowModulator.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <string>

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
        for (int profile = 0; profile < 4; ++profile) {
            const bool edge = profile % 2 != 0;
            const bool overlap = profile >= 2;
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
            FlowD1 transfer;
            FlowModulator historical;
            const ResearchConfig research{double(rate), 42};
            if (!a.prepare(research, ac) || !b.prepare(research, bc) ||
                !trajectory.prepare(research, {1, .005, .05}) ||
                !transfer.prepare(research, {1, .005, .05}) || !historical.prepare(research))
                return 1;
            const std::string label = (overlap ? (edge ? "-sustained" : "-overlap") : "") +
                                      std::string(edge ? "-edge" : "-reference");
            std::ofstream output(root / (std::to_string(rate) + label + ".csv"));
            std::ofstream audit(root / (std::to_string(rate) + label + "-audit.csv"));
            output << std::setprecision(17) << "path_m,a_left,a_right,b_left,b_right\n";
            audit << std::setprecision(17)
                  << "input_left,input_right,d0_left,d0_right,d1_left,d1_right\n";
            std::ofstream events(root / (std::to_string(rate) + label + "-events.csv"));
            events << std::setprecision(17)
                   << "source,id,time_s,radius_m,frequency_hz,identity_token,admitted,due_time_s\n";
            std::size_t simultaneous{};
            // Same physical gated source at each rate; synthetic engineering fixture,
            // never presented as a recording or musical-pad listening evidence.
            for (int n = 0; n < rate; ++n) {
                const double t = double(n) / rate;
                const bool gate =
                    t >= .05 && std::fmod(t - .05, .1) < (overlap ? .04 : .02) && t < .85;
                // Additional fixture changes excitation only: a quiet sustained bed
                // keeps A1 active while louder attacks produce delayed B1 events.
                const double level =
                    overlap && t >= .05 && t < .85 ? (gate ? .9 : .4) : (gate ? .4 : 0.);
                const float x = float(level * std::sin(2 * std::numbers::pi * 997 * t));
                const StereoFrame input{x, -.5f * x};
                const auto oldA = a.requested();
                const auto oldAAccepted = a.pool().counters().accepted;
                const auto oldEligible = b.counters().eligible;
                const auto oldAdmitted = b.counters().admitted;
                const auto av = a.process(input);
                const auto bv = b.process(input);
                // Read-only event provenance; no extra RNG draws or source updates.
                if (a.requested() != oldA) {
                    const auto& e = a.lastRequestedEvent();
                    events << "A1," << a.requested() << ',' << t << ',' << e.physics.radiusMeters
                           << ',' << e.physics.frequencyHz << ',' << e.bin << ','
                           << (a.pool().counters().accepted != oldAAccepted) << ",\n";
                }
                if (b.counters().eligible != oldEligible) {
                    const auto& e = b.lastEligible();
                    events << "B1," << e.eligibleId << ',' << t << ','
                           << e.physics.equivalentRadiusMeters << ',' << e.physics.frequencyHz
                           << ',' << e.randomRank << ',' << (b.counters().admitted != oldAdmitted)
                           << ',' << double(e.dueSample) / rate << '\n';
                }
                const StereoFrame cluster{float(double(av[0]) + bv[0] + 0.),
                                          float(double(av[1]) + bv[1] + 0.)};
                const auto dv = transfer.process(cluster).transferred;
                const auto d0 = historical.process(input);
                simultaneous += av[0] != 0.f && bv[0] != 0.f;
                output << trajectory.process() << ',' << av[0] << ',' << av[1] << ',' << bv[0]
                       << ',' << bv[1] << '\n';
                audit << input[0] << ',' << input[1] << ',' << d0[0] << ',' << d0[1] << ',' << dv[0]
                      << ',' << dv[1] << '\n';
            }
            if (!output || !audit || !events || a.requested() == 0 || b.counters().admitted == 0 ||
                (overlap && !edge && simultaneous == 0))
                return 1;
        }
    }
    return 0;
}
