#include <gtest/gtest.h>

#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace std::chrono_literals;

TEST(SimulationTest, MultiRateExecution)
{
    simulator sim;

    int count_25us = 0;
    int count_100us = 0;
    int count_200us = 0;

    sim.register_lambda(duration_t(25e-6), [&]() { count_25us++; });
    sim.register_lambda(duration_t(100e-6), [&]() { count_100us++; });
    sim.register_lambda(duration_t(200e-6), [&]() { count_200us++; });

    sim.initialize();

    // Simulate for 400us
    sim.simulate_for(duration_t(400e-6));

    EXPECT_EQ(count_25us, 16);
    EXPECT_EQ(count_100us, 4);
    EXPECT_EQ(count_200us, 2);
}

TEST(SimulationTest, RunStep)
{
    simulator sim;

    int count = 0;
    sim.register_lambda(duration_t(100e-6), [&]() { count++; });

    sim.initialize();

    // Base rate is 100us.
    sim.simulate_steps(1);
    EXPECT_EQ(count, 1);
    EXPECT_NEAR(sim.get_current_simulation_time().value(), 100e-6, 1e-9);

    sim.simulate_steps(3);
    EXPECT_EQ(count, 4);
    EXPECT_NEAR(sim.get_current_simulation_time().value(), 400e-6, 1e-9);
}

TEST(SimulationTest, DataLogging)
{
    simulator sim;

    float signal = 0.0f;
    sim.register_lambda(duration_t(100e-6), [&]() { signal += 1.0f; });
    sim.register_signal("test_signal", [&]() { return signal; });

    sim.initialize();

    // Simulate for 300us (3 ticks of 100us)
    sim.simulate_for(duration_t(300e-6));

    const auto& data = sim.get_logger().get_data("test_signal");
    ASSERT_EQ(data.size(), 3);

    EXPECT_FLOAT_EQ(data[0][0], 0.0f);
    EXPECT_FLOAT_EQ(data[1][0], 1.0f);
    EXPECT_FLOAT_EQ(data[2][0], 2.0f);

    EXPECT_FLOAT_EQ(signal, 3.0f);
}

TEST(SimulationTest, SimulationTimeLogging)
{
    simulator sim;
    sim.register_lambda(duration_t(10e-6), []() {});  // Set base rate
    sim.initialize();
    sim.simulate_for(duration_t(30e-6));

    const auto& time_data = sim.get_logger().get_data("time");
    ASSERT_EQ(time_data.size(), 3);
    EXPECT_NEAR(time_data[0][0], 0.000000, 1e-7);
    EXPECT_NEAR(time_data[1][0], 0.000010, 1e-7);
    EXPECT_NEAR(time_data[2][0], 0.000020, 1e-7);
}

TEST(SimulationTest, DecoupledLoggingRate)
{
    simulator sim;

    int task_runs = 0;
    sim.register_lambda(duration_t(100e-6), [&]() { task_runs++; });

    // Log at 400us interval (every 4 ticks)
    sim.set_logging_period(duration_t(400e-6));
    sim.initialize();

    // Simulate for 1.2ms (12 ticks)
    // Should result in 12/4 = 3 samples
    sim.simulate_for(duration_t(1200e-6));

    EXPECT_EQ(task_runs, 12);
    const auto& data = sim.get_logger().get_data("time");
    EXPECT_EQ(data.size(), 3);
    EXPECT_NEAR(data[0][0], 0.0000, 1e-7);
    EXPECT_NEAR(data[1][0], 0.0004, 1e-7);
    EXPECT_NEAR(data[2][0], 0.0008, 1e-7);
}

TEST(SimulationTest, ManualStepLogging)
{
    simulator sim;

    sim.register_lambda(duration_t(100e-6), []() {});
    sim.initialize();

    // Manual steps should still trigger logging
    sim.simulate_steps(1);
    sim.simulate_steps(1);

    const auto& data = sim.get_logger().get_data("time");
    EXPECT_EQ(data.size(), 2);
    EXPECT_NEAR(data[0][0], 0.0000, 1e-7);
    EXPECT_NEAR(data[1][0], 0.0001, 1e-7);
}

// Mock structures to test coordinate frame detection
struct mock_abc {
    float a() const { return 1.0f; }
    float b() const { return 2.0f; }
    float c() const { return 3.0f; }
};

struct mock_alphabeta {
    float alpha() const { return 4.0f; }
    float beta() const { return 5.0f; }
};

struct mock_dq {
    float d() const { return 6.0f; }
    float q() const { return 7.0f; }
};

TEST(SimulationTest, CoordinateFrameLogging)
{
    simulator sim;
    sim.register_lambda(duration_t(100e-6), []() {});  // base rate

    sim.register_signal("abc", []() { return mock_abc{}; });
    sim.register_signal("alphabeta", []() { return mock_alphabeta{}; });
    sim.register_signal("dq", []() { return mock_dq{}; });

    sim.initialize();
    sim.simulate_steps(2);

    const auto& abc_data = sim.get_logger().get_data("abc");
    ASSERT_EQ(abc_data.size(), 2);  // 2 samples
    EXPECT_FLOAT_EQ(abc_data[0][0], 1.0f);
    EXPECT_FLOAT_EQ(abc_data[0][1], 2.0f);
    EXPECT_FLOAT_EQ(abc_data[0][2], 3.0f);
    EXPECT_FLOAT_EQ(abc_data[1][0], 1.0f);
    EXPECT_FLOAT_EQ(abc_data[1][1], 2.0f);
    EXPECT_FLOAT_EQ(abc_data[1][2], 3.0f);

    const auto& ab_data = sim.get_logger().get_data("alphabeta");
    ASSERT_EQ(ab_data.size(), 2);  // 2 samples
    EXPECT_FLOAT_EQ(ab_data[0][0], 4.0f);
    EXPECT_FLOAT_EQ(ab_data[0][1], 5.0f);

    const auto& dq_data = sim.get_logger().get_data("dq");
    ASSERT_EQ(dq_data.size(), 2);  // 2 samples
    EXPECT_FLOAT_EQ(dq_data[0][0], 6.0f);
    EXPECT_FLOAT_EQ(dq_data[0][1], 7.0f);
}

}  // namespace
