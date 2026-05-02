#include "pitaya/srf_pll.hpp"

#include <gtest/gtest.h>
#include <cmath>
#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;
using namespace mojito;

TEST(SrfPllTest, BasicInit) {
    duration_t ts{1e-4f};
    srf_pll pll(ts);
    EXPECT_NEAR(pll.get_estimated_frequency().value(), 0.0, 0.01);
}

TEST(SrfPllTest, TrackingNominal) {
    duration_t ts{1e-4f};
    srf_pll pll(ts);
    frequency_t nominal{50.0f};
    pll.configure_nominal_frequency(nominal);
    pll.reset(frequency_pu_t{1.0f});
    
    angular_frequency_t omega{static_cast<float>(2.0 * pi * 50.0)};
    
    for(int i=0; i<2000; ++i) {
        float t = i * ts.value();
        alphabeta<voltage_pu_t> v_in{voltage_pu_t{std::cos(omega.value() * t)}, voltage_pu_t{std::sin(omega.value() * t)}};
        pll.update(v_in);
    }
    
    EXPECT_NEAR(pll.get_estimated_frequency().value(), 50.0, 0.1);
}

}  // namespace
