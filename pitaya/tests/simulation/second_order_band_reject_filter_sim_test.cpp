#include <gtest/gtest.h>

#include <mojito/mojito.hpp>

#include "pitaya/second_order_band_reject_filter.hpp"
#include "pitaya/simulation/assertion.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class SecondOrderBandRejectFilterSimTest
/// @brief Simulation tests for the second-order band-reject (notch) filter.
class SecondOrderBandRejectFilterSimTest : public ::testing::Test {
protected:
    simulator sim = make_gtest_simulator();
};

/// @test BandRejectResponse
/// @brief Verifies the attenuation of a second-order band-reject (notch) filter.
TEST_F(SecondOrderBandRejectFilterSimTest, BandRejectResponse)
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
