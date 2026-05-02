#include "pitaya/hardware_filter_compensator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "pitaya/first_order_low_pass_filter.hpp"

using namespace pitaya;
using namespace mojito;

class HardwareFilterCompensatorTest : public ::testing::Test {
protected:
    static constexpr double sampling_time = 50e-6;
    static constexpr double fc1 = 796.0;
    static constexpr double fc2 = 159.0;
    
    hardware_filter_compensator compensator;
};

TEST_F(HardwareFilterCompensatorTest, RobustConfiguration)
{
    abc<float> input{1.0f, -0.5f, -0.5f};

    // Case 1: Invalid f1, requested 1
    compensator.configure(frequency_t{0.0f}, frequency_t{100.0f}, 1);
    abc<float> output = compensator.update(input, frequency_t{50.0f});
    EXPECT_NEAR(output.a(), input.a(), 1e-6);

    // Case 2: Invalid f1, valid f2, requested 2
    compensator.configure(frequency_t{0.0f}, frequency_t{100.0f}, 2);
    output = compensator.update(input, frequency_t{100.0f});
    float phase_45 = std::atan(1.0f);
    EXPECT_NEAR(output.a(), mojito::to_abc(mojito::to_alphabeta(input).rotate_counter_clockwise(angle_wrapped::from_radians(angle_t{phase_45}))).a(), 1e-6);

    // Case 3: Valid f1, invalid f2, requested 2
    compensator.configure(frequency_t{100.0f}, frequency_t{0.0f}, 2);
    output = compensator.update(input, frequency_t{100.0f});
    EXPECT_NEAR(output.a(), mojito::to_abc(mojito::to_alphabeta(input).rotate_counter_clockwise(angle_wrapped::from_radians(angle_t{phase_45}))).a(), 1e-6);

    // Case 4: Negative frequencies
    compensator.configure(frequency_t{-50.0f}, frequency_t{-100.0f}, 2);
    output = compensator.update(input, frequency_t{50.0f});
    EXPECT_NEAR(output.a(), input.a(), 1e-6);

    // Case 5: Wrong filter count
    compensator.configure(frequency_t{50.0f}, frequency_t{100.0f}, 0);
    output = compensator.update(input, frequency_t{50.0f});
    EXPECT_NEAR(output.a(), input.a(), 1e-6);
}
