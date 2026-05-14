#include <gtest/gtest.h>

#include "pitaya/linear_ramp.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;

/// @class LinearRampSimTest
/// @brief Simulation tests for the linear ramp component.
class LinearRampSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();

    linear_ramp<float> ramp{duration_t{1e-3}};
    float output{0.0f};
};

/// @test RampTracking
/// @brief Verifies that the ramp correctly interpolates and finishes.
TEST_F(LinearRampSimTest, RampTracking)
{
    ramp.configure({.initial = 0.0f, .final = 10.0f, .duration = duration_t{0.1}});

    sim.register_lambda(duration_t{1e-3}, [this]() { output = ramp.update(true); });

    sim.register_signal("output", [this]() { return output; });
    sim.register_signal("is_finished", [this]() { return ramp.is_finished() ? 1.0f : 0.0f; });

    // 1. Halfway point: at 0.05s, output should be 5.0
    sim.register_assertion(make_near_assert<float>({.name = "halfway",
        .func = [this]() { return output; },
        .expected = 5.0f,
        .epsilon = 0.15f,  // A bit loose due to discrete steps
        .strategy = at_time({.time = duration_t{0.05}, .tolerance = duration_t{1e-3}})}));

    // 2. Finished state: after 0.1s, output should be 10.0 and is_finished should be true
    sim.register_assertion(make_near_assert<float>({.name = "final_value",
        .func = [this]() { return output; },
        .expected = 10.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.11}, .end = duration_t{0.15}})}));

    sim.register_assertion(make_lambda_assert({.name = "finished_flag",
        .func =
            [this]() {
                return ramp.is_finished() ? assertion_result::pass() : assertion_result::fail("Ramp not finished");
            },
        .strategy = time_range({.start = duration_t{0.11}, .end = duration_t{0.15}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.15));
}

}  // namespace
