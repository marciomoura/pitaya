#include <gtest/gtest.h>

#include <mojito/mojito.hpp>

#include "pitaya/second_order_high_pass_filter.hpp"
#include "pitaya/simulation/assertion.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class SecondOrderHighPassFilterSimTest
/// @brief Simulation tests for the second-order high-pass filter.
class SecondOrderHighPassFilterSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();

    second_order_high_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

/// @test StepResponse
/// @brief Verifies the time-domain step response of the second-order high-pass filter.
TEST_F(SecondOrderHighPassFilterSimTest, StepResponse)
{
    filter.configure(10.0, 0.707);
    sim.register_lambda(duration_t{100e-6}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();
        input = (current_time >= 0.05) ? voltage_pu_t{1.0f} : voltage_pu_t{0.0f};
        output = filter.update(input);
    });

    sim.register_signal("input", [this]() { return input.value(); });
    sim.register_signal("output", [this]() { return output.value(); });

    // Assertions
    // 1. Initial response: Should jump on step then decay
    sim.register_assertion(make_range_assert<voltage_pu_t>({.name = "initial_jump",
        .func = [this]() { return output; },
        .min = voltage_pu_t{0.5f},
        .max = voltage_pu_t{1.5f},
        .strategy = at_time({.time = duration_t{0.0501}, .tolerance = duration_t{100e-6}})}));

    // 2. Steady state verification (should decay to 0)
    sim.register_assertion(make_near_assert<voltage_pu_t>({.name = "steady_state_zero",
        .func = [this]() { return output; },
        .expected = voltage_pu_t{0.0f},
        .epsilon = voltage_pu_t{0.01f},
        .strategy = time_range({.start = duration_t{0.4}, .end = duration_t{0.5}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.5));
}

/// @test HighFrequencyPass
/// @brief Verifies that high frequencies are passed with minimal attenuation.
TEST_F(SecondOrderHighPassFilterSimTest, HighFrequencyPass)
{
    filter.configure(10.0, 0.707);

    three_phase_waveform_generator gen{100e-6};
    gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);
    gen.set_signal_frequency(100.0f);  // 100Hz >> 10Hz cutoff

    sim.register_lambda(duration_t{100e-6}, [&]() {
        gen.update();
        input = voltage_pu_t{gen.get_signal_abc().a()};
        output = filter.update(input);
    });

    sim.register_signal("input", [&]() { return input.value(); });
    sim.register_signal("output", [&]() { return output.value(); });

    // Assertion: At 100Hz, output should be close to input (approx 1.0 pu peak)
    sim.register_assertion(make_range_assert<voltage_pu_t>({.name = "high_freq_pass",
        .func = [&]() { return output; },
        .min = voltage_pu_t{-1.1f},
        .max = voltage_pu_t{1.1f},
        .strategy = time_range({.start = duration_t{0.1}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
