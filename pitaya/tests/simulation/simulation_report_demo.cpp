#include <gtest/gtest.h>

#include <cmath>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace std::chrono_literals;

TEST(SimulationReportDemo, SineCosine)
{
    simulator sim;

    // Register a dummy task to set the base simulation rate (e.g., 100us)
    sim.register_lambda(duration_t(100e-6), []() {});

    const float pi = 3.14159265358979323846f;
    const float frequency = 50.0f;  // 50 Hz
    const float omega = 2.0f * pi * frequency;

    // Register signals to log
    sim.register_signal("sine", [&]() -> float {
        float t = static_cast<float>(sim.get_current_simulation_time_seconds());
        return std::sin(omega * t);
    });

    sim.register_signal("cosine", [&]() -> float {
        float t = static_cast<float>(sim.get_current_simulation_time_seconds());
        return std::cos(omega * t);
    });

    // Register a mock coordinate frame
    struct abc_signals {
        float a() const { return std::sin(w * t); }
        float b() const { return std::sin(w * t - 2.0f * pi / 3.0f); }
        float c() const { return std::sin(w * t + 2.0f * pi / 3.0f); }
        float w;
        float t;
        float pi;
    };

    sim.register_signal("three_phase", [&]() -> abc_signals {
        return abc_signals{omega, static_cast<float>(sim.get_current_simulation_time_seconds()), pi};
    });

    sim.initialize();

    // Use gtest_exporter to automatically save data to SimulationReportDemo_SineCosine.csv
    gtest_exporter exporter(sim);

    // Simulate for 40ms (2 cycles at 50Hz)
    sim.simulate_for(duration_t(0.04));
}

}  // namespace
