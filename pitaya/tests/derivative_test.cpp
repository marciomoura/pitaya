#include "pitaya/derivative.hpp"

#include <gtest/gtest.h>

using namespace pitaya;

TEST(DerivativeTest, InitialStateIsZero)
{
    derivative deriv(0.01);
    EXPECT_FLOAT_EQ(deriv.get_output(), 0.0f);
}

TEST(DerivativeTest, CalculatesCorrectDerivative)
{
    constexpr double sampling_time = 0.1;
    derivative deriv(sampling_time);

    // First update, previous value is 0
    deriv.update(1.0f);
    EXPECT_FLOAT_EQ(deriv.get_output(), (1.0f - 0.0f) / static_cast<float>(sampling_time));  // 10.0

    // Second update
    deriv.update(1.5f);
    EXPECT_FLOAT_EQ(deriv.get_output(), (1.5f - 1.0f) / static_cast<float>(sampling_time));  // 5.0

    // Third update with negative change
    deriv.update(1.2f);
    EXPECT_NEAR(deriv.get_output(), (1.2f - 1.5f) / static_cast<float>(sampling_time), 1e-6);  // -3.0
}

TEST(DerivativeTest, ResetWorksCorrectly)
{
    derivative deriv(0.1);
    deriv.update(5.0f);
    ASSERT_NE(deriv.get_output(), 0.0f);

    deriv.reset(10.0f);
    EXPECT_FLOAT_EQ(deriv.get_output(), 0.0f);

    // Next update uses the reset value as previous input
    deriv.update(12.0f);
    EXPECT_FLOAT_EQ(deriv.get_output(), (12.0f - 10.0f) / 0.1f);  // 20.0
}

TEST(DerivativeTest, ConfigureSamplingTime)
{
    derivative deriv(0.1);
    deriv.update(1.0f);
    EXPECT_FLOAT_EQ(deriv.get_output(), 10.0f);

    deriv.configure_sampling_time(0.05);
    deriv.update(1.5f);                                          // Previous value was 1.0
    EXPECT_FLOAT_EQ(deriv.get_output(), (1.5f - 1.0f) / 0.05f);  // 10.0
}

TEST(DerivativeTest, InitialValueWorksCorrectly)
{
    derivative deriv(0.1, 5.0f);  // Initial value of 5.0
    deriv.update(6.0f);
    EXPECT_FLOAT_EQ(deriv.get_output(), (6.0f - 5.0f) / 0.1f);  // 10.0
}
