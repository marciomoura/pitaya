#include <gtest/gtest.h>

#include <mojito/mojito.hpp>

#include "pitaya/second_order_low_pass_filter.hpp"
#include "pitaya/simulation/assertion.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class SecondOrderLowPassFilterSimTest
/// @brief Simulation tests for the second-order low-pass filter.
class SecondOrderLowPassFilterSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        filter.configure(10.0, 0.707);
        sim.register_lambda(duration_t{100e-6}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();
            input = (current_time >= 0.05) ? voltage_pu_t{1.0f} : voltage_pu_t{0.0f};
            output = filter.update(input);
        });

        sim.register_signal("input", [this]() { return input.value(); });
        sim.register_signal("output", [this]() { return output.value(); });
    }

    simulator sim = make_gtest_simulator();

    second_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

/// @test StepResponse
/// @brief Verifies the time-domain step response of the second-order low-pass filter.
TEST_F(SecondOrderLowPassFilterSimTest, StepResponse)
{
    // Assertions
    // 1. Steady state verification
    sim.register_assertion(make_near_assert<voltage_pu_t>({.name = "steady_state",
        .func = [this]() { return output; },
        .expected = voltage_pu_t{1.0f},
        .epsilon = voltage_pu_t{0.01f},
        .strategy = time_range({.start = duration_t{0.4}, .end = duration_t{0.5}})}));

    // 2. Max overshoot verification
    sim.register_assertion(make_max_assert<voltage_pu_t>({.name = "max_overshoot",
        .func = [this]() { return output; },
        .max = voltage_pu_t{1.05f},
        .strategy = always_active()}));

    sim.initialize();
    sim.simulate_for(duration_t(0.5));
}

}  // namespace
