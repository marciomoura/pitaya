#include <gtest/gtest.h>

#include "pitaya/first_order_low_pass_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

class FirstOrderLowPassFilterSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        filter.configure(frequency_t{10.0f});
        sim.register_lambda(duration_t{100e-6}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();
            input = (current_time >= 0.05) ? voltage_pu_t{1.0f} : voltage_pu_t{0.0f};
            output = filter.update(input);
        });

        sim.register_signal("input", [this]() { return input.value(); });
        sim.register_signal("output", [this]() { return output.value(); });

        // Step response assertions
        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "initial_zero", [this]() { return output; }, voltage_pu_t{0.0f}, voltage_pu_t{1e-3f},
            time_range(duration_t{0.0}, duration_t{0.045})));

        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "steady_state", [this]() { return output; }, voltage_pu_t{1.0f}, voltage_pu_t{0.02f},
            time_range(duration_t{0.4}, duration_t{0.5})));

        // Tau approx 0.0159s. At t = 0.05 + 0.0159 = 0.0659, output should be approx 0.632
        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "one_tau", [this]() { return output; }, voltage_pu_t{0.632f}, voltage_pu_t{0.05f},
            at_time(duration_t{0.0659}, duration_t{100e-6})));

        sim.initialize();
    }

    simulator sim = make_gtest_simulator();
    first_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

TEST_F(FirstOrderLowPassFilterSimTest, StepResponse)
{
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.5));
}

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

TEST_F(FirstOrderLowPassFilterFrequencyTest, CutoffAttenuation)
{
    filter.configure(frequency_t{50.0f});
    gen.set_signal_frequency(50.0f);

    // At cutoff, magnitude should be approx 0.707
    sim.register_assertion(make_range_assert<voltage_pu_t>(
        "magnitude_at_cutoff", [this]() { return output; }, voltage_pu_t{-0.75f}, voltage_pu_t{0.75f},
        time_range(duration_t{0.1}, duration_t{0.2})));

    sim.initialize();
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.2));
}

TEST_F(FirstOrderLowPassFilterSimTest, NoiseRejection)
{
    filter.configure(frequency_t{10.0f});
    simulator noise_sim = make_gtest_simulator();
    white_noise_generator<voltage_pu_t> noise_gen{0.2f};
    voltage_pu_t noise_input{0.0f};
    voltage_pu_t filtered_output{0.0f};

    noise_sim.register_lambda(duration_t{100e-6}, [&]() {
        noise_input = voltage_pu_t{1.0f} + noise_gen.generate();
        filtered_output = filter.update(noise_input);
    });

    noise_sim.register_signal("input", [&]() { return noise_input.value(); });
    noise_sim.register_signal("output", [&]() { return filtered_output.value(); });

    noise_sim.register_assertion(make_range_assert<voltage_pu_t>(
        "noise_rejection", [&]() { return filtered_output; }, voltage_pu_t{0.85f}, voltage_pu_t{1.15f},
        time_range(duration_t{0.5}, duration_t{1.0})));

    noise_sim.initialize();
    gtest_exporter exporter(noise_sim);
    noise_sim.simulate_for(duration_t(1.0));
}

}  // namespace
