#include "pitaya/pi_controller.hpp"

#include <gtest/gtest.h>
#include <cmath>
#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;
using namespace mojito;

TEST(PIControllerTest, ConfigureWithTi) {
    const double ts = 0.01;
    const real_t kp = 2.0f;
    const real_t ti = 4.0f; 
    pi_controller<double> controller(ts);
    controller.configure_with_ti(kp, ti);
    EXPECT_NEAR(controller.get_kp(), kp, 1e-6);
    EXPECT_NEAR(controller.get_ki(), 0.5f, 1e-6);
}

TEST(PIControllerTest, OutputLimitClamping) {
    const double ts = 0.01;
    const real_t kp = 5.0f;
    const real_t ki = 10.0f;
    pi_controller<double> controller(ts);
    controller.configure(kp, ki);
    controller.set_output_limits(-1.0f, 1.0f);
    controller.reset();

    double output;
    for (int i = 0; i < 20; ++i) {
        output = controller.update(1.0);
    }
    EXPECT_LE(output, 1.0);
    controller.reset();
    for (int i = 0; i < 20; ++i) {
        output = controller.update(-1.0);
    }
    EXPECT_GE(output, -1.0);
}

TEST(PIControllerTest, ControlInDQFrame) {
    pi_controller<dq<double>> controller(0.01);
    controller.configure_with_ti(2.0f, 1.0f);
    dq<double> output = controller.update(dq<double>{1.0, 1.0});
    EXPECT_DOUBLE_EQ(output[0], output[1]);
}

TEST(PIControllerTest, ControlInAlphaBetaFrame) {
    pi_controller<alphabeta<double>> controller(0.01);
    controller.configure_with_ti(2.0f, 1.0f);
    alphabeta<double> output = controller.update(alphabeta<double>{1.0, 1.0});
    EXPECT_DOUBLE_EQ(output[0], output[1]);
}

TEST(PIControllerTest, ControlInABCFrame) {
    pi_controller<abc<double>> controller(0.01);
    controller.configure_with_ti(2.0f, 1.0f);
    abc<double> output = controller.update(abc<double>{1.0, 1.0, 1.0});
    EXPECT_DOUBLE_EQ(output[0], output[1]);
    EXPECT_DOUBLE_EQ(output[0], output[2]);
}

}  // namespace
