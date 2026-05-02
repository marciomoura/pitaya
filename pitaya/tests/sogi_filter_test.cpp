#include "pitaya/sogi_filter.hpp"

#include <gtest/gtest.h>
#include <cmath>
#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;
using namespace mojito;

TEST(SogiFilterTest, BasicInitialization) {
    duration_t ts{50e-6f};
    sogi_filter filter(ts);
    EXPECT_NEAR(filter.get_alpha().value(), 0.0, 1e-6);
    EXPECT_NEAR(filter.get_beta().value(), 0.0, 1e-6);
}

TEST(SogiFilterTest, TrackingNominal) {
    duration_t ts{1e-4f};
    sogi_filter filter(ts);
    angular_frequency_t omega{static_cast<float>(2.0 * pi * 50.0)};
    
    // Simulate a few cycles
    for(int i=0; i<2000; ++i) {
        float t = i * ts.value();
        filter.update(voltage_pu_t{std::sin(omega.value() * t)}, omega);
    }
    
    EXPECT_NEAR(filter.get_output().magnitude().value(), 1.0, 0.02);
}

}  // namespace
