#include "pitaya/simulation/simulation_task.hpp"

#include <gtest/gtest.h>

namespace {

using namespace pitaya;

TEST(SimulationTaskTest, LambdaTaskBasic)
{
    bool initialized = false;
    bool run = false;
    bool stopped = false;

    lambda_task task(duration_t(1e-3), [&]() { run = true; }, [&]() { initialized = true; }, [&]() { stopped = true; });

    EXPECT_NEAR(task.get_sampling_time().value(), 1e-3, 1e-7);

    task.initialize();
    EXPECT_TRUE(initialized);

    task.run();
    EXPECT_TRUE(run);

    task.stop();
    EXPECT_TRUE(stopped);
}

class MockTask : public simulation_task {
public:
    duration_t get_sampling_time() const override { return duration_t(0.1); }
    void run() override { run_count++; }
    void initialize() override { init_called = true; }
    void stop() override { stop_called = true; }

    int run_count = 0;
    bool init_called = false;
    bool stop_called = false;
};

TEST(SimulationTaskTest, CustomTaskInterface)
{
    MockTask task;
    EXPECT_NEAR(task.get_sampling_time().value(), 0.1, 1e-7);

    task.initialize();
    EXPECT_TRUE(task.init_called);

    task.run();
    EXPECT_EQ(task.run_count, 1);

    task.stop();
    EXPECT_TRUE(task.stop_called);
}

}  // namespace
