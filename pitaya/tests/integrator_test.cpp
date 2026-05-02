#include "pitaya/integrator.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;

TEST(IntegratorTest, IntegratesConstantInput)
{
    // Verify that a constant input integrates correctly.
    integrator<double> integ;
    const double ts = 0.001;  // 1 ms sampling time
    integ.configure(ts);
    integ.reset();

    const size_t num_samples = 1000;  // simulate for 1 second
    const double input = 1.0;
    double expected = 0.0;

    for (size_t i = 0; i < num_samples; ++i) {
        expected += input * ts;
        double output = integ.update(input);
        EXPECT_NEAR(output, expected, 1e-6);
    }

    EXPECT_NEAR(integ.get_output(), expected, 1e-6);
}

TEST(IntegratorTest, SetValueAndReset)
{
    // Verify that setting an initial value and then resetting works.
    integrator<double> integ;
    const double ts = 0.001;
    integ.configure(ts);

    integ.reset(5.0);
    EXPECT_NEAR(integ.get_output(), 5.0, 1e-6);

    // Update with an input value and check the integration result.
    double output = integ.update(2.0);
    EXPECT_NEAR(output, 5.0 + 2.0 * ts, 1e-6);

    integ.reset();
    EXPECT_NEAR(integ.get_output(), 0.0, 1e-6);
}

TEST(IntegratorTest, AngleWrappingTest)
{
    // Use the integrator to simulate angular integration.
    // For an angular velocity input of 2π * 50 rad/s (i.e. 50 rotations per second)
    // integrating over exactly 1 second should yield exactly 50 full rotations.
    using mojito::angle_wrapped;
    using mojito::angle_t;
    using mojito::angular_frequency_t;

    integrator<angle_wrapped> angle_integrator{};
    const double ts = 0.001;  // sampling time: 1 ms
    angle_integrator.configure(ts);
    angle_integrator.reset();

    const angular_frequency_t angular_velocity = angular_frequency_t{static_cast<float>(2.0 * mojito::pi * 50.0)};  // 50 rotations per second
    const size_t num_samples = 1000;                  // simulate for 1 second

    for (size_t i = 0; i < num_samples; ++i) {
        angle_integrator.update(angular_velocity);
    }

    auto integrated_angle = angle_integrator.get_output();  // Will wrap 2π to zero
    EXPECT_NEAR((integrated_angle - angle_t{0.01f}).get_radians(), 2.0 * mojito::pi - 0.01, 1e-3);
    EXPECT_NEAR((integrated_angle + angle_t{0.01f}).get_radians(), 0.01, 1e-3);  // Wrapping around 2π and zero

    // Now simulate a non-integer number of rotations.
    // For example, simulate for 1.01 seconds.
    angle_integrator.reset();
    size_t new_samples = static_cast<size_t>(1.01 / ts);
    for (size_t i = 0; i < new_samples; ++i) {
        angle_integrator.update(angular_velocity);
    }
    integrated_angle = angle_integrator.get_output();  // ~2π * 50 * 1.01

    // Expected remainder after wrapping to [0, 2π)
    double expected_remainder = std::fmod(2.0 * mojito::pi * 50.0 * 1.01, 2.0 * mojito::pi);
    EXPECT_NEAR(integrated_angle.get_radians(), expected_remainder, 1e-5);
}

}  // namespace
