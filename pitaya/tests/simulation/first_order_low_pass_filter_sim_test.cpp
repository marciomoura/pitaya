#include <gtest/gtest.h>

#include "pitaya/first_order_low_pass_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class FirstOrderLowPassFilterSimTest
/// @brief Simulation tests for the first-order low-pass filter.
///
/// These tests verify the dynamic behavior of the filter, including its step response,
/// frequency attenuation, and noise rejection capabilities using a simulated environment.
class FirstOrderLowPassFilterSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Default configuration for the filter
        filter.configure(frequency_t{10.0f});
    }

    void register_step_response_task()
    {
        sim.register_lambda(duration_t{100e-6}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();
            input = (current_time >= 0.05) ? voltage_pu_t{1.0f} : voltage_pu_t{0.0f};
            output = filter.update(input);
        });

        sim.register_signal("input", [this]() { return input.value(); });
        sim.register_signal("output", [this]() { return output.value(); });
    }

    simulator sim = make_gtest_simulator();

    first_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

/// @test StepResponse
/// @brief Verifies the time-domain step response of the filter.
///
/// Ensures the filter remains at zero initially, reaches approx 63.2% of the target
/// at one time constant (Tau), and eventually reaches steady-state.
TEST_F(FirstOrderLowPassFilterSimTest, StepResponse)
{
    register_step_response_task();

    // Assertions
    // 1. Initial zero state before step at 0.05s
    sim.register_assertion(make_near_assert<voltage_pu_t>({.name = "initial_zero",
        .func = [this]() { return output; },
        .expected = voltage_pu_t{0.0f},
        .epsilon = voltage_pu_t{1e-3f},
        .strategy = time_range({.start = duration_t{0.0}, .end = duration_t{0.045}})}));

    // 2. Steady state after approx 5-10 Tau
    sim.register_assertion(make_near_assert<voltage_pu_t>({.name = "steady_state",
        .func = [this]() { return output; },
        .expected = voltage_pu_t{1.0f},
        .epsilon = voltage_pu_t{0.02f},
        .strategy = time_range({.start = duration_t{0.4}, .end = duration_t{0.5}})}));

    // 3. Time constant (Tau) verification
    // Cutoff = 10Hz -> Tau = 1/(2*pi*10) approx 0.0159s.
    // At t = 0.05 + 0.0159 = 0.0659, output should be approx 0.632
    sim.register_assertion(make_near_assert<voltage_pu_t>({.name = "one_tau",
        .func = [this]() { return output; },
        .expected = voltage_pu_t{0.632f},
        .epsilon = voltage_pu_t{0.05f},
        .strategy = at_time({.time = duration_t{0.0659}, .tolerance = duration_t{100e-6}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.5));
}

/// @test NoiseRejection
/// @brief Verifies the filter's ability to attenuate high-frequency noise.
///
/// Applies a noisy signal centered at 1.0 pu and ensures the filtered output
/// stays within a much tighter band than the raw input noise.
TEST_F(FirstOrderLowPassFilterSimTest, NoiseRejection)
{
    filter.configure(frequency_t{10.0f});

    white_noise_generator<voltage_pu_t> noise_gen{0.2f};
    voltage_pu_t noise_input{0.0f};
    voltage_pu_t filtered_output{0.0f};

    // Define the simulation task locally
    sim.register_lambda(duration_t{100e-6}, [&]() {
        noise_input = voltage_pu_t{1.0f} + noise_gen.generate();
        filtered_output = filter.update(noise_input);
    });

    sim.register_signal("input", [&]() { return noise_input.value(); });
    sim.register_signal("output", [&]() { return filtered_output.value(); });

    // Assertion: Filtered output should be much smoother than input (within 15% band)
    sim.register_assertion(make_range_assert<voltage_pu_t>({.name = "noise_rejection",
        .func = [&]() { return filtered_output; },
        .min = voltage_pu_t{0.85f},
        .max = voltage_pu_t{1.15f},
        .strategy = time_range({.start = duration_t{0.5}, .end = duration_t{1.0}})}));

    sim.initialize();
    sim.simulate_for(duration_t(1.0));
}

/// @class FirstOrderLowPassFilterFrequencyTest
/// @brief Simulation tests for the frequency response of the filter.
class FirstOrderLowPassFilterFrequencyTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);
        sim.register_lambda(duration_t{100e-6}, [this]() {
            gen.update();
            input = voltage_pu_t{gen.get_signal_abc().a()};
            output = filter.update(input);
        });

        sim.register_signal("input", [this]() { return input.value(); });
        sim.register_signal("output", [this]() { return output.value(); });
    }

    simulator sim = make_gtest_simulator();

    three_phase_waveform_generator gen{100e-6};
    first_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

/// @test CutoffAttenuation
/// @brief Verifies the magnitude attenuation at the cutoff frequency.
///
/// At the cutoff frequency, the magnitude should be approx 0.707 (-3dB).
TEST_F(FirstOrderLowPassFilterFrequencyTest, CutoffAttenuation)
{
    filter.configure(frequency_t{50.0f});
    gen.set_signal_frequency(50.0f);

    // Assertion: At 50Hz cutoff, peak magnitude should be around 0.707
    sim.register_assertion(make_range_assert<voltage_pu_t>({.name = "magnitude_at_cutoff",
        .func = [this]() { return output; },
        .min = voltage_pu_t{-0.75f},
        .max = voltage_pu_t{0.75f},
        .strategy = time_range({.start = duration_t{0.1}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
