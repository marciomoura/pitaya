#include "pitaya/dual_sogi_srf_pll.hpp"

#include <gtest/gtest.h>
#include <cmath>
#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;
using namespace mojito;

TEST(DualSogiPllTest, BasicInit) {
    duration_t ts{1e-4f};
    dual_sogi_srf_pll pll(ts);
    // After construction/default init, frequency should be 50 Hz nominal, 
    // but the output filter in the internal PLL starts at 0.
    EXPECT_NEAR(pll.get_estimated_frequency().value(), 0.0, 1e-6);
}

TEST(DualSogiPllTest, Locking) {
    duration_t ts{1e-4f};
    dual_sogi_srf_pll pll(ts);
    angular_frequency_t omega{static_cast<float>(2.0 * pi * 50.0)};
    
    for(int i=0; i<5000; ++i) {
        float t = i * ts.value();
        alphabeta<voltage_pu_t> v_in{voltage_pu_t{std::cos(omega.value() * t)}, voltage_pu_t{std::sin(omega.value() * t)}};
        pll.update(v_in);
    }
    
    EXPECT_TRUE(pll.is_locked());
    EXPECT_NEAR(pll.get_estimated_frequency().value(), 50.0, 0.1);
}

}  // namespace
