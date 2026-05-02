#include "pitaya/hysteresis_limiter.hpp"

#include <gtest/gtest.h>

using namespace pitaya;

class HysteresisLimiterTest : public ::testing::Test {
protected:
    void SetUp() override { limiter.configure_thresholds(10.0f, 20.0f); }

    hysteresis_limiter limiter;
};

TEST_F(HysteresisLimiterTest, InitialStateIsFalse) { EXPECT_FALSE(limiter.get_output()); }

TEST_F(HysteresisLimiterTest, ResetReturnsToFalse)
{
    limiter.update(25.0f);  // Go high
    ASSERT_TRUE(limiter.get_output());

    limiter.reset();
    EXPECT_FALSE(limiter.get_output());
}

TEST_F(HysteresisLimiterTest, DoesNotTriggerBelowHighThreshold)
{
    limiter.update(19.9f);
    EXPECT_FALSE(limiter.get_output());
}

TEST_F(HysteresisLimiterTest, TriggersWhenCrossingHighThreshold)
{
    limiter.update(20.1f);
    EXPECT_TRUE(limiter.get_output());
}

TEST_F(HysteresisLimiterTest, StaysHighWhenAboveLowThreshold)
{
    limiter.update(25.0f);  // Go high
    ASSERT_TRUE(limiter.get_output());

    limiter.update(10.1f);  // Drop into hysteresis band
    EXPECT_TRUE(limiter.get_output());
}

TEST_F(HysteresisLimiterTest, GoesLowWhenCrossingLowThreshold)
{
    limiter.update(25.0f);  // Go high
    ASSERT_TRUE(limiter.get_output());

    limiter.update(9.9f);  // Drop below low threshold
    EXPECT_FALSE(limiter.get_output());
}

TEST_F(HysteresisLimiterTest, ChatteringInHysteresisBand)
{
    limiter.update(25.0f);  // Go high
    ASSERT_TRUE(limiter.get_output());

    // Jitter above low threshold
    limiter.update(11.0f);
    limiter.update(15.0f);
    limiter.update(12.0f);
    EXPECT_TRUE(limiter.get_output()) << "Output should not chatter while in the hysteresis band.";

    limiter.update(9.0f);  // Go low
    ASSERT_FALSE(limiter.get_output());

    // Jitter below high threshold
    limiter.update(18.0f);
    limiter.update(15.0f);
    limiter.update(19.0f);
    EXPECT_FALSE(limiter.get_output()) << "Output should not chatter while in the hysteresis band.";
}
