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

        sim.register_signal("v_abc", [this]() { return v_abc; });
        sim.register_signal("measured_frequency", [this]() { return pll.get_estimated_frequency().value(); });
        sim.register_signal("reference_angle", [this]() { return ref_angle.get_radians(); });
        sim.register_signal("measured_angle", [this]() { return pll.get_estimated_angle().get_radians(); });

        // Assertions
        sim.register_assertion(make_near_assert<float>(
            "frequency_lock", [this]() { return pll.get_estimated_frequency().value(); }, 50.0f, 0.2f,
            time_range(duration_t{0.15}, duration_t{0.2})));

        sim.register_assertion(make_lambda_assert(
            "angle_lock",
            [this]() {
                float diff_rad = (pll.get_estimated_angle() - ref_angle).get_radians().value();
                if (diff_rad > mojito::pi) diff_rad -= 2.0f * mojito::pi;
                float diff = std::abs(diff_rad);
                if (diff > 0.1f) {
                    return assertion_result::fail("Angle error " + std::to_string(diff) + " rad exceeds 0.1 rad");
                }
                return assertion_result::pass();
            },
            time_range(duration_t{0.15}, duration_t{0.2})));
    }

    simulator sim = make_gtest_simulator();
    three_phase_waveform_generator gen{100e-6};
    srf_pll pll{duration_t{100e-6}};
    abc<voltage_pu_t> v_abc{voltage_pu_t{0.0f}, voltage_pu_t{0.0f}, voltage_pu_t{0.0f}};
    angle_wrapped ref_angle{0.0f};
};

TEST_F(SrfPllSimTest, LockedResponse)
{
    sim.initialize();
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.2));
}

TEST_F(SrfPllSimTest, FrequencyStep)
{
    gen.set_frequency_step(10.0f, 0.5f);  // 50Hz -> 60Hz at 0.5s

    sim.register_assertion(make_near_assert<float>(
        "frequency_after_step", [this]() { return pll.get_estimated_frequency().value(); }, 60.0f, 0.5f,
        time_range(duration_t{0.8}, duration_t{1.0})));

    sim.initialize();
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(1.0));
}

TEST_F(SrfPllSimTest, PhaseStep)
{
    gen.set_angle_step(
        angle_wrapped::from_radians(angle_t{static_cast<float>(mojito::pi / 4.0)}), 0.5f);  // +45 deg jump

    sim.register_assertion(make_lambda_assert(
        "angle_lock_after_jump",
        [this]() {
            float diff_rad = (pll.get_estimated_angle() - ref_angle).get_radians().value();
            if (diff_rad > mojito::pi) diff_rad -= 2.0f * mojito::pi;
            float diff = std::abs(diff_rad);
            if (diff > 0.1f) {
                return assertion_result::fail("Angle error " + std::to_string(diff) + " rad exceeds 0.1 rad");
            }
            return assertion_result::pass();
        },
        time_range(duration_t{0.8}, duration_t{1.0})));

    sim.initialize();
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(1.0));
}

}  // namespace
