#include <gtest/gtest.h>

#include <cmath>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
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

        sim.register_lambda(duration_t{100e-6}, [this]() {
            double t = sim.get_current_simulation_time_seconds();
            double freq = 50.0;
            double omega = 2.0 * mojito::pi * freq;
            ref_angle =
                angle_wrapped::from_radians(angle_t{static_cast<float>(std::fmod(omega * t, 2.0 * mojito::pi))});

            v_abc = abc<voltage_pu_t>{voltage_pu_t{static_cast<float>(std::cos(omega * t))},
                voltage_pu_t{static_cast<float>(std::cos(omega * t - 2.0 * mojito::pi / 3.0))},
                voltage_pu_t{static_cast<float>(std::cos(omega * t + 2.0 * mojito::pi / 3.0))}};

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
                float diff = std::abs((pll.get_estimated_angle() - ref_angle).get_radians().value());
                if (diff > 0.1f) {
                    return assertion_result::fail("Angle error " + std::to_string(diff) + " rad exceeds 0.1 rad");
                }
                return assertion_result::pass();
            },
            time_range(duration_t{0.15}, duration_t{0.2})));

        sim.initialize();
    }

    simulator sim = make_gtest_simulator();
    srf_pll pll{duration_t{100e-6}};
    abc<voltage_pu_t> v_abc{voltage_pu_t{0.0f}, voltage_pu_t{0.0f}, voltage_pu_t{0.0f}};
    angle_wrapped ref_angle{0.0f};
};

TEST_F(SrfPllSimTest, LockedResponse)
{
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.2));
}

}  // namespace
