#include "pitaya/simulation/task_scheduler.hpp"

#include <gtest/gtest.h>

namespace {

using namespace pitaya;

TEST(TaskSchedulerTest, ToNsHelper)
{
    EXPECT_EQ(task_scheduler::to_ns(duration_t(1.0)).count(), 1000000000LL);
    EXPECT_EQ(task_scheduler::to_ns(duration_t(1e-6)).count(), 1000LL);
}

TEST(TaskSchedulerTest, BasePeriodAndGroups)
{
    task_scheduler scheduler;

    int c1 = 0, c2 = 0;
    scheduler.register_lambda(duration_t(100e-6), [&]() { c1++; });
    scheduler.register_lambda(duration_t(200e-6), [&]() { c2++; });

    EXPECT_NEAR(scheduler.get_base_period().value(), 100e-6, 1e-9);

    scheduler.run_steps(0, 1);  // Tick 0: both run
    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 1);

    scheduler.run_steps(1, 1);  // Tick 1: only c1 runs
    EXPECT_EQ(c1, 2);
    EXPECT_EQ(c2, 1);

    scheduler.run_steps(2, 1);  // Tick 2: both run
    EXPECT_EQ(c1, 3);
    EXPECT_EQ(c2, 2);
}

TEST(TaskSchedulerTest, InitializeAndStop)
{
    task_scheduler scheduler;
    bool init = false, stop = false;

    class LifecycleTask : public simulation_task {
    public:
        LifecycleTask(bool& i, bool& s) : _i(i), _s(s) {}
        duration_t get_sampling_time() const override { return duration_t(1.0); }
        void run() override {}
        void initialize() override { _i = true; }
        void stop() override { _s = true; }

    private:
        bool &_i, &_s;
    };

    scheduler.register_task(std::make_shared<LifecycleTask>(init, stop));

    scheduler.initialize();
    EXPECT_TRUE(init);

    scheduler.stop();
    EXPECT_TRUE(stop);
}

}  // namespace
