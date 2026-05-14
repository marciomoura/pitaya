#include <gtest/gtest.h>

#include "pitaya/second_order_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

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

        // Assertions
        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "steady_state", [this]() { return output; }, voltage_pu_t{1.0f}, voltage_pu_t{0.01f},
            time_range(duration_t{0.4}, duration_t{0.5})));

        sim.register_assertion(make_max_assert<voltage_pu_t>(
            "max_overshoot", [this]() { return output; }, voltage_pu_t{1.05f}, always_active()));

        sim.initialize();
    }

    simulator sim = make_gtest_simulator();
    second_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

TEST_F(SecondOrderFilterSimTest, StepResponse)
{
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.5));
}

TEST_F(SecondOrderFilterSimTest, BandRejectResponse)
{
    second_order_band_reject_filter<voltage_pu_t> notch{100e-6};
    notch.configure(100.0, 10.0);  // Reject 100Hz

    three_phase_waveform_generator gen{100e-6};
    gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);
    gen.set_signal_frequency(100.0f);

    simulator notch_sim = make_gtest_simulator();
    voltage_pu_t notch_input{0.0f};
    voltage_pu_t notch_output{0.0f};

    notch_sim.register_lambda(duration_t{100e-6}, [&]() {
        gen.update();
        notch_input = voltage_pu_t{gen.get_signal_abc().a()};
        notch_output = notch.update(notch_input);
    });

    notch_sim.register_signal("input", [&]() { return notch_input.value(); });
    notch_sim.register_signal("output", [&]() { return notch_output.value(); });

    // At 100Hz, output should be heavily attenuated
    notch_sim.register_assertion(make_range_assert<voltage_pu_t>(
        "notch_attenuation", [&]() { return notch_output; }, voltage_pu_t{-0.1f}, voltage_pu_t{0.1f},
        time_range(duration_t{0.1}, duration_t{0.2})));

    notch_sim.initialize();
    gtest_exporter exporter(notch_sim);
    notch_sim.simulate_for(duration_t(0.2));
}

}  // namespace
