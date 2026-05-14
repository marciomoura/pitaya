#include <gtest/gtest.h>

#include "pitaya/first_order_low_pass_filter.hpp"
#include "pitaya/pi_controller.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class PIControllerSimTest
/// @brief Closed-loop simulation tests for the PI controller.
///
/// These tests verify the PI controller's tracking performance, disturbance rejection,
/// and anti-windup capabilities by controlling a first-order plant model in a
/// simulated environment.
class PIControllerSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Default controller configuration: Kp=2.0, Ti=0.1s
        controller.configure_with_ti(2.0f, 0.1f);
        controller.set_output_limits(-2.0f, 2.0f);
        controller.enable_integrator_clamping(true);

        // Plant configuration: Cutoff = 5Hz (Tau approx 0.0318s)
        plant.configure(frequency_t{5.0f});
    }

    void register_closed_loop_task()
    {
        sim.register_lambda(duration_t{1e-3}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();

            // 1. Reference: Step from 0 to 1.0 at t=0.1s
            reference = (current_time >= 0.1) ? 1.0f : 0.0f;

            // 2. Disturbance: Step of -0.5 at t=1.0s
            float disturbance = (current_time >= 1.0) ? -0.5f : 0.0f;

            // Closed-loop logic
            error = reference - measured_output;
            controller_output = controller.update(error);
            measured_output = plant.update(controller_output + disturbance);
        });

        sim.register_signal("reference", [this]() { return reference; });
        sim.register_signal("measured_output", [this]() { return measured_output; });
        sim.register_signal("error", [this]() { return error; });
        sim.register_signal("controller_output", [this]() { return controller_output; });
        sim.register_signal("saturated", [this]() { return controller.is_output_saturated() ? 1.0f : 0.0f; });
    }

    simulator sim = make_gtest_simulator();

    pi_controller<float> controller{1e-3};
    first_order_low_pass_filter<float> plant{1e-3};

    float reference{0.0f};
    float error{0.0f};
    float controller_output{0.0f};
    float measured_output{0.0f};
};

/// @test ClosedLoopResponse
/// @brief Verifies the tracking and disturbance rejection of the closed-loop system.
///
/// Ensures the controller accurately tracks a step in the reference and
/// effectively compensates for a step disturbance in the plant output.
TEST_F(PIControllerSimTest, ClosedLoopResponse)
{
    register_closed_loop_task();

    // Assertions
    // 1. Tracking: Should reach 1.0 after step at 0.1s
    sim.register_assertion(make_near_assert<float>({.name = "tracking_steady_state",
        .func = [this]() { return measured_output; },
        .expected = 1.0f,
        .epsilon = 0.02f,
        .strategy = time_range({.start = duration_t{0.8}, .end = duration_t{0.9}})}));

    // 2. Disturbance rejection: Should return to 1.0 after disturbance at 1.0s
    sim.register_assertion(make_near_assert<float>({.name = "disturbance_rejection",
        .func = [this]() { return measured_output; },
        .expected = 1.0f,
        .epsilon = 0.02f,
        .strategy = time_range({.start = duration_t{1.8}, .end = duration_t{1.9}})}));

    sim.initialize();
    sim.simulate_for(duration_t(2.0));
}

/// @test AntiWindupSaturating
/// @brief Verifies the controller's anti-windup (clamping) during output saturation.
///
/// Forces the controller into saturation by setting a reference much higher
/// than the output limits, and ensures the output remains clamped.
TEST_F(PIControllerSimTest, AntiWindupSaturating)
{
    // Force saturation by setting a very high reference
    sim.register_lambda(duration_t{1e-3}, [this]() {
        double current_time = sim.get_current_simulation_time_seconds();
        reference = (current_time >= 0.1) ? 5.0f : 0.0f;  // 5.0 is well above 2.0 limit
        error = reference - measured_output;
        controller_output = controller.update(error);
        measured_output = plant.update(controller_output);
    });

    // Assertion: Output must not exceed the limit of 2.0
    sim.register_assertion(make_max_assert<float>({.name = "output_clamped",
        .func = [this]() { return controller_output; },
        .max = 2.01f,
        .strategy = always_active()}));

    sim.initialize();
    sim.simulate_for(duration_t(1.0));
}

}  // namespace
