#include <gtest/gtest.h>

#include <cmath>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"
#include "pitaya/srf_pll.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class SrfPllSimTest
/// @brief Simulation tests for the Synchronous Reference Frame PLL (SRF-PLL).
///
/// These tests verify the frequency and phase tracking capabilities of the SRF-PLL
/// under various transient conditions like frequency steps and phase jumps.
class SrfPllSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        pll.configure_nominal_frequency(frequency_t{50.0f});
        pll.configure_pi_controller(0.5, duration_t{5.5e-3});
        pll.reset({frequency_pu_t{0.0f}});

        gen.set_signal_frequency(50.0f);
        gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);

        sim.register_lambda(duration_t{100e-6}, [this]() {
            gen.update();
            v_abc = to_dimension_abc<voltage_pu_t>(gen.get_signal_abc());
            ref_angle = gen.get_fundamental_signal_angle().a();
            pll.update(mojito::to_alphabeta(v_abc));
        });

        sim.register_signal(plot_metadata{.name = "v_abc", .row = 1, .col = 1}, [this]() { return v_abc; });
        sim.register_signal(plot_metadata{.name = "measured_frequency", .row = 2, .col = 1},
            [this]() { return pll.get_estimated_frequency().value(); });
        sim.register_signal(
            plot_metadata{.name = "reference_angle", .row = 3, .col = 1}, [this]() { return ref_angle.get_pu(); });
        sim.register_signal(plot_metadata{.name = "measured_angle", .row = 3, .col = 1},
            [this]() { return pll.get_estimated_angle().get_pu(); });
    }

    simulator sim = make_gtest_simulator();

    three_phase_waveform_generator gen{100e-6};
    srf_pll pll{duration_t{100e-6}};
    abc<voltage_pu_t> v_abc{voltage_pu_t{0.0f}, voltage_pu_t{0.0f}, voltage_pu_t{0.0f}};
    angle_wrapped ref_angle{0.0f};
};

/// @test LockedResponse
/// @brief Verifies the basic locking behavior and steady-state accuracy.
///
/// Ensures the PLL locks onto the nominal 50Hz frequency and maintains
/// a low phase error.
TEST_F(SrfPllSimTest, LockedResponse)
{
    // Assertions
    // 1. Frequency lock verification
    sim.register_assertion(make_near_assert<float>({.name = "frequency_lock",
        .func = [this]() { return pll.get_estimated_frequency().value(); },
        .expected = 50.0f,
        .epsilon = 0.2f,
        .strategy = time_range({.start = duration_t{0.15}, .end = duration_t{0.2}})}));

    // 2. Phase lock verification (Angle error < 0.1 rad)
    sim.register_assertion(make_lambda_assert({.name = "angle_lock",
        .func =
            [this]() {
                float diff_rad = (pll.get_estimated_angle() - ref_angle).get_radians().value();
                if (diff_rad > mojito::pi) diff_rad -= 2.0f * mojito::pi;
                float diff = std::abs(diff_rad);
                if (diff > 0.1f) {
                    return assertion_result::fail("Angle error " + std::to_string(diff) + " rad exceeds 0.1 rad");
                }
                return assertion_result::pass();
            },
        .strategy = time_range({.start = duration_t{0.15}, .end = duration_t{0.2}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.2));
}

/// @test FrequencyStep
/// @brief Verifies the PLL's response to a sudden frequency step.
///
/// Applies a +10Hz step at 0.5s and ensures the PLL tracks it accurately
/// after a short transient.
TEST_F(SrfPllSimTest, FrequencyStep)
{
    gen.set_frequency_step({.time = 0.5f, .value = 10.0f});  // 50Hz -> 60Hz at 0.5s

    // Assertion: PLL should track 60Hz after step
    sim.register_assertion(make_near_assert<float>({.name = "frequency_after_step",
        .func = [this]() { return pll.get_estimated_frequency().value(); },
        .expected = 60.0f,
        .epsilon = 0.5f,
        .strategy = time_range({.start = duration_t{0.8}, .end = duration_t{1.0}})}));

    sim.initialize();
    sim.simulate_for(duration_t(1.0));
}

/// @test PhaseStep
/// @brief Verifies the PLL's response to a sudden phase jump.
///
/// Applies a +45 degree phase jump at 0.5s and ensures the PLL re-synchronizes
/// with the new phase angle.
TEST_F(SrfPllSimTest, PhaseStep)
{
    gen.set_angle_step(
        0.5f, angle_wrapped::from_radians(angle_t{static_cast<float>(mojito::pi / 4.0)}));  // +45 deg jump

    // Assertion: PLL should re-lock onto the phase angle after the jump
    sim.register_assertion(make_lambda_assert({.name = "angle_lock_after_jump",
        .func =
            [this]() {
                float diff_rad = (pll.get_estimated_angle() - ref_angle).get_radians().value();
                if (diff_rad > mojito::pi) diff_rad -= 2.0f * mojito::pi;
                float diff = std::abs(diff_rad);
                if (diff > 0.1f) {
                    return assertion_result::fail("Angle error " + std::to_string(diff) + " rad exceeds 0.1 rad");
                }
                return assertion_result::pass();
            },
        .strategy = time_range({.start = duration_t{0.8}, .end = duration_t{1.0}})}));

    sim.initialize();
    sim.simulate_for(duration_t(1.0));
}

}  // namespace
