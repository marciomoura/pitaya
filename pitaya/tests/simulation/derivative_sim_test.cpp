#include <gtest/gtest.h>

#include "pitaya/derivative.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;

/// @class DerivativeSimTest
/// @brief Simulation tests for the derivative component.
class DerivativeSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();

    derivative calc{1e-3};
    float input{0.0f};
    float output{0.0f};
};

/// @test RampInput
/// @brief Verifies that a linear ramp input results in a constant derivative.
TEST_F(DerivativeSimTest, RampInput)
{
    // 10.0 units/s ramp
    sim.register_lambda(duration_t{1e-3}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();
        input = static_cast<float>(10.0 * current_time);
        calc.update(input);
        output = calc.get_output();
    });

    sim.register_signal("input", [this]() { return input; });
    sim.register_signal("output", [this]() { return output; });

    // Assertion: Derivative of 10*t should be 10.0
    // Skip the very first sample where it might be zero or transient
    sim.register_assertion(make_near_assert<float>({.name = "constant_derivative",
        .func = [this]() { return output; },
        .expected = 10.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.01}, .end = duration_t{0.1}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.1));
}

/// @test ConstantInput
/// @brief Verifies that a constant input results in a zero derivative.
TEST_F(DerivativeSimTest, ConstantInput)
{
    sim.register_lambda(duration_t{1e-3}, [this]() {
        input = 5.0f;
        calc.update(input);
        output = calc.get_output();
    });

    sim.register_signal("input", [this]() { return input; });
    sim.register_signal("output", [this]() { return output; });

    sim.register_assertion(make_near_assert<float>({.name = "zero_derivative",
        .func = [this]() { return output; },
        .expected = 0.0f,
        .epsilon = 1e-3f,
        .strategy = time_range({.start = duration_t{0.01}, .end = duration_t{0.1}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.1));
}

}  // namespace
