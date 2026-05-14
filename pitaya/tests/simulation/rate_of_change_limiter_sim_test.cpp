#include <gtest/gtest.h>

#include "pitaya/rate_of_change_limiter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;

/// @class RateOfChangeLimiterSimTest
/// @brief Simulation tests for the rate of change limiter component.
class RateOfChangeLimiterSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();

    rate_of_change_limiter limiter{1e-3};
    float input{0.0f};
    float output{0.0f};
};

/// @test RateLimiting
/// @brief Verifies that the output follows the rate limit during a step change.
TEST_F(RateOfChangeLimiterSimTest, RateLimiting)
{
    // Limit to 10.0 units/s
    limiter.configure(10.0f);
    limiter.configure_enable(true);

    sim.register_lambda(duration_t{1e-3}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();
        // Step from 0 to 10 at t=0.1s.
        // With rate 10.0 units/s, it should reach 10.0 at t=1.1s (exactly 1s ramp).
        input = (current_time >= 0.1) ? 10.0f : 0.0f;
        limiter.update(input);
        output = limiter.get_output();
    });

    sim.register_signal("input", [this]() { return input; });
    sim.register_signal("output", [this]() { return output; });

    // 1. Before step: output should be 0
    sim.register_assertion(make_near_assert<float>({.name = "initial_zero",
        .func = [this]() { return output; },
        .expected = 0.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.0}, .end = duration_t{0.09}})}));

    // 2. Ramping: at 0.6s (0.5s after step), output should be 10 * 0.5 = 5.0
    sim.register_assertion(make_near_assert<float>({.name = "ramping_halfway",
        .func = [this]() { return output; },
        .expected = 5.0f,
        .epsilon = 0.05f,
        .strategy = at_time({.time = duration_t{0.6}, .tolerance = duration_t{1e-3}})}));

    // 3. Steady state: after 1.1s (1.0s after step), output should be 10.0
    sim.register_assertion(make_near_assert<float>({.name = "steady_state",
        .func = [this]() { return output; },
        .expected = 10.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{1.11}, .end = duration_t{1.5}})}));

    sim.initialize();
    sim.simulate_for(duration_t(1.5));
}

/// @test DisabledPassThrough
/// @brief Verifies that the limiter acts as pass-through when disabled.
TEST_F(RateOfChangeLimiterSimTest, DisabledPassThrough)
{
    limiter.configure(1.0f);  // Very slow limit
    limiter.configure_enable(false);

    sim.register_lambda(duration_t{1e-3}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();
        input = (current_time >= 0.1) ? 10.0f : 0.0f;
        limiter.update(input);
        output = limiter.get_output();
    });

    sim.register_signal("input", [this]() { return input; });
    sim.register_signal("output", [this]() { return output; });

    // Assertion: output should immediately follow input
    sim.register_assertion(make_near_assert<float>({.name = "pass_through",
        .func = [this]() { return output; },
        .expected = 10.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.11}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
