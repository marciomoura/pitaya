#include <gtest/gtest.h>
#include <pitaya/pitaya.hpp>

TEST(PitayaTest, SumPositiveNumbers) {
    EXPECT_EQ(pitaya::sum(1, 2), 3);
}

TEST(PitayaTest, SumNegativeNumbers) {
    EXPECT_EQ(pitaya::sum(-1, -1), -2);
}

TEST(PitayaTest, SumFloats) {
    EXPECT_NEAR(pitaya::sum(1.5, 2.5), 4.0, 1e-9);
}
