#include <gtest/gtest.h>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"
#include "pitaya/threshold_protection.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class ThresholdProtectionSimTest
/// @brief Simulation tests for the threshold protection component.
///
/// These tests verify the dynamic behavior of protection logic, including
/// warning and trip delays, noise immunity, and manual reset recovery.
class ThresholdProtectionSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // 150% trip, 120% warning, 50ms trip delay, 10ms warning delay
        protection.configure_trip_threshold(current_pu_t{1.5f}, 0.05f);
        protection.configure_warning_threshold(current_pu_t{1.2f});
        protection.configure_trip_delay(duration_t{0.05f});
        protection.configure_warning_delay(duration_t{0.01f});

        sim.register_lambda(duration_t{1e-3}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();

            // Scenario:
            // 0.0s - 0.1s: Normal (1.0 pu)
            // 0.1s - 0.105s: Short Spike (2.0 pu) -> Should not trip
            // 0.2s - 0.4s: Sustained Overcurrent (1.6 pu) -> Should trip at 0.25s
            // 0.4s - 0.6s: Recovery (1.0 pu) -> Latched, should allow reset

            if (current_time < 0.1) {
                input_current = 1.0f;
            }
            else if (current_time < 0.105) {
                input_current = 2.0f;
            }
            else if (current_time < 0.2) {
                input_current = 1.0f;
            }
            else if (current_time < 0.4) {
                input_current = 1.6f;
            }
            else {
                input_current = 1.0f;
            }

            protection.update(current_pu_t{input_current});

            if (current_time >= 0.55 && current_time < 0.551) {
                protection.reset();
            }
        });

        sim.register_signal("current", [this]() { return input_current; });
        sim.register_signal("is_warning", [this]() { return protection.is_warning() ? 1.0f : 0.0f; });
        sim.register_signal("is_tripped", [this]() { return protection.is_tripped() ? 1.0f : 0.0f; });
        sim.register_signal("can_reset", [this]() { return protection.can_reset() ? 1.0f : 0.0f; });
    }

    simulator sim = make_gtest_simulator();

    threshold_protection<current_pu_t> protection{duration_t{1e-3f}};
    float input_current{1.0f};
};

/// @test DynamicBehavior
/// @brief Verifies the protection state transitions during a fault event.
///
/// Ensures short spikes are ignored, sustained overcurrent triggers a trip
/// after the configured delay, and the trip remains latched until a reset.
TEST_F(ThresholdProtectionSimTest, DynamicBehavior)
{
    // Assertions
    // 1. Noise immunity: No trip on short spike at 0.1s
    sim.register_assertion(make_lambda_assert(
        "no_spike_trip",
        [this]() {
            return !protection.is_tripped() ? assertion_result::pass()
                                            : assertion_result::fail("Tripped on short spike");
        },
        time_range(duration_t{0.1}, duration_t{0.15})));

    // 2. Trip verification: Must trip after 50ms of sustained 1.6 pu overcurrent
    sim.register_assertion(make_lambda_assert(
        "sustained_trip",
        [this]() {
            return protection.is_tripped() ? assertion_result::pass()
                                           : assertion_result::fail("Failed to trip on sustained overcurrent");
        },
        time_range(duration_t{0.26}, duration_t{0.4})));

    // 3. Reset verification: Manual reset at 0.55s must clear the trip
    sim.register_assertion(make_lambda_assert(
        "reset_works",
        [this]() {
            return !protection.is_tripped() ? assertion_result::pass() : assertion_result::fail("Failed to reset");
        },
        time_range(duration_t{0.6}, duration_t{0.7})));

    sim.initialize();
    sim.simulate_for(duration_t(0.7));
}

}  // namespace
