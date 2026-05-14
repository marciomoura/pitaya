#include <gtest/gtest.h>

#include "pitaya/second_order_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class SecondOrderFilterSimTest
/// @brief Simulation tests for the second-order filters (Low-Pass and Band-Reject).
///
/// These tests verify the dynamic behavior and frequency response of second-order filters,
/// focusing on step response (overshoot, steady-state) and notch attenuation.
class SecondOrderFilterSimTest : public ::testing::Test {
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
///
/// Ensures the filter reaches steady-state and that overshoot remains within
/// acceptable limits for a Butterworth-like (damping=0.707) response.
TEST_F(SecondOrderFilterSimTest, StepResponse)
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

/// @test BandRejectResponse
/// @brief Verifies the attenuation of a second-order band-reject (notch) filter.
///
/// Ensures that a signal at the notch frequency (100Hz) is heavily attenuated.
TEST_F(SecondOrderFilterSimTest, BandRejectResponse)
{
    second_order_band_reject_filter<voltage_pu_t> notch{100e-6};
    notch.configure(100.0, 10.0);  // Reject 100Hz

    three_phase_waveform_generator gen{100e-6};
    gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);
    gen.set_signal_frequency(100.0f);

    voltage_pu_t notch_input{0.0f};
    voltage_pu_t notch_output{0.0f};

    sim.register_lambda(duration_t{100e-6}, [&]() {
        gen.update();
        notch_input = voltage_pu_t{gen.get_signal_abc().a()};
        notch_output = notch.update(notch_input);
    });

    sim.register_signal("input", [&]() { return notch_input.value(); });
    sim.register_signal("output", [&]() { return notch_output.value(); });

    // Assertion: At 100Hz notch, output should be heavily attenuated (< 0.1 pu)
    sim.register_assertion(make_range_assert<voltage_pu_t>({.name = "notch_attenuation",
        .func = [&]() { return notch_output; },
        .min = voltage_pu_t{-0.1f},
        .max = voltage_pu_t{0.1f},
        .strategy = time_range({.start = duration_t{0.1}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
