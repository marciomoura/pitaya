#include <gtest/gtest.h>

#include "pitaya/interval_timer.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;

/// @class IntervalTimerSimTest
/// @brief Simulation tests for the interval timer component.
class IntervalTimerSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();

    interval_timer timer{1e-3};
};

/// @test Accumulation
/// @brief Verifies that the timer accumulates time correctly when running.
TEST_F(IntervalTimerSimTest, Accumulation)
{
    sim.register_lambda(duration_t{1e-3}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();

        if (current_time >= 0.05 && current_time < 0.0505) {
            timer.start();
        }

        if (current_time >= 0.15 && current_time < 0.1505) {
            timer.stop();
        }

        timer.update();
    });

    sim.register_signal("elapsed_us", [this]() { return timer.get_elapsed_time(); });

    // 1. Initial zero
    sim.register_assertion(make_near_assert<float>({.name = "initial_zero",
        .func = [this]() { return timer.get_elapsed_time(); },
        .expected = 0.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.0}, .end = duration_t{0.04}})}));

    // 2. Accumulating: at 0.1s (0.05s after start), should be 50,000 us
    sim.register_assertion(make_near_assert<float>({.name = "accumulating",
        .func = [this]() { return timer.get_elapsed_time(); },
        .expected = 50000.0f,
        .epsilon = 1000.0f,  // Tolerance for one sample
        .strategy = at_time({.time = duration_t{0.1}, .tolerance = duration_t{1e-3}})}));

    // 3. Stopped: after 0.15s, should stay at 100,000 us
    sim.register_assertion(make_near_assert<float>({.name = "stopped_value",
        .func = [this]() { return timer.get_elapsed_time(); },
        .expected = 100000.0f,
        .epsilon = 1.0f,
        .strategy = time_range({.start = duration_t{0.16}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
