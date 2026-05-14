#include <gtest/gtest.h>

#include <cmath>

#include "pitaya/dual_sogi_srf_pll.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/three_phase_waveform_generator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class DualSogiSrfPllSimTest
/// @brief Simulation tests for the Dual-SOGI Synchronous Reference Frame PLL.
///
/// These tests verify the robustness of the Dual-SOGI PLL against unbalanced
/// grid conditions and its ability to track frequency transients.
class DualSogiSrfPllSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        pll.configure_nominal_frequency(frequency_t{50.0f});
        pll.configure_pi_controller(0.5f, duration_t{5.5e-3f});
        pll.configure_sequence_extractor(1.414f, duration_t{0.02f});
        pll.reset();

        gen.set_signal_frequency(50.0f);
        gen.set_fundamental_positive_sequence_signal_amplitude(1.0f);

        sim.register_lambda(duration_t{100e-6}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();

            // Scenario 1: Unbalanced Grid (Negative Sequence) at t=0.4s
            if (current_time >= 0.4 && current_time < 0.4001) {
                gen.set_fundamental_negative_sequence_signal_amplitude(0.2f);
            }

            // Scenario 2: Frequency Step 50Hz -> 55Hz at t=0.8s
            if (current_time >= 0.8 && current_time < 0.8001) {
                gen.set_frequency_step(5.0f, static_cast<float>(current_time));
            }

            gen.update();
            v_abc = to_dimension_abc<voltage_pu_t>(gen.get_signal_abc());
            v_pos_seq_ref = to_dimension_abc<voltage_pu_t>(gen.get_fundamental_signal_abc());

            pll.update(mojito::to_alphabeta(v_abc));
        });

        sim.register_signal(plot_metadata{.name = "v_abc", .row = 1, .col = 1}, [this]() { return v_abc; });
        sim.register_signal(plot_metadata{.name = "v_pos_seq_pll", .row = 1, .col = 1},
            [this]() { return pll.get_positive_sequence(); });
        sim.register_signal(plot_metadata{.name = "measured_angle", .row = 1, .col = 1},
            [this]() { return pll.get_estimated_angle().get_pu(); });
        sim.register_signal(plot_metadata{.name = "freq_estimated", .row = 2, .col = 1},
            [this]() { return pll.get_estimated_frequency().value(); });
        sim.register_signal(
            plot_metadata{.name = "is_locked", .row = 3, .col = 1}, [this]() { return pll.is_locked() ? 1.0f : 0.0f; });
    }

    simulator sim = make_gtest_simulator();

    three_phase_waveform_generator gen{100e-6};
    dual_sogi_srf_pll pll{duration_t{100e-6}};

    abc<voltage_pu_t> v_abc{voltage_pu_t{0.0f}, voltage_pu_t{0.0f}, voltage_pu_t{0.0f}};
    abc<voltage_pu_t> v_pos_seq_ref{voltage_pu_t{0.0f}, voltage_pu_t{0.0f}, voltage_pu_t{0.0f}};
};

/// @test RobustnessSimulation
/// @brief Verifies PLL performance under unbalance and frequency steps.
///
/// Ensures the PLL maintains a lock during an unbalanced grid event and
/// accurately tracks a 5Hz frequency step.
TEST_F(DualSogiSrfPllSimTest, RobustnessSimulation)
{
    // Assertions
    // 1. Initial lock at 50Hz
    sim.register_assertion(make_near_assert<float>(
        "initial_lock", [this]() { return pll.get_estimated_frequency().value(); }, 50.0f, 0.5f,
        time_range(duration_t{0.2}, duration_t{0.3})));

    // 2. Unbalance robustness (should stay at 50Hz despite negative sequence)
    sim.register_assertion(make_near_assert<float>(
        "unbalance_robustness", [this]() { return pll.get_estimated_frequency().value(); }, 50.0f, 0.5f,
        time_range(duration_t{0.5}, duration_t{0.7})));

    // 3. Frequency step tracking (should follow to 55Hz)
    sim.register_assertion(make_near_assert<float>(
        "frequency_step_tracking", [this]() { return pll.get_estimated_frequency().value(); }, 55.0f, 0.5f,
        time_range(duration_t{1.2}, duration_t{1.3})));

    sim.initialize();
    sim.simulate_for(duration_t(1.5));
}

}  // namespace
