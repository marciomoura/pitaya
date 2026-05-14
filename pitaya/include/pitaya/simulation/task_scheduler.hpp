#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "pitaya/simulation/simulation_task.hpp"

namespace pitaya {

/// Schedules simulation tasks based on their individual execution rates.
class task_scheduler {
public:
    /// Helper to convert duration_t to internal precise chrono duration.
    static std::chrono::nanoseconds to_ns(duration_t d);

    /// Structure to group tasks that run at the same rate.
    struct task_group {
        std::size_t normalized_ticks{};
        std::vector<std::shared_ptr<simulation_task>> tasks{};
    };

    /// Register a task with the scheduler.
    void register_task(std::shared_ptr<simulation_task> task);

    /// Register a lambda as a task.
    void register_lambda(duration_t sampling_time, std::function<void()> func);

    /// Initialize all tasks and prepare the schedule.
    void initialize();

    /// Stop all tasks.
    void stop();

    /// Advance the simulation by a specified number of steps.
    void run_steps(std::size_t current_tick, std::size_t steps);

    /// Get the base tick period (the fastest sampling rate).
    duration_t get_base_period() const;

private:
    /// Normalizes task rates and builds the optimized task groups.
    void rebuild_schedule();

    void sort_task_buffer_and_update_base_period();

    void build_task_groups_by_normalized_ticks();

    void sort_task_groups_by_interval();

    duration_t _base_period{0.0};
    std::vector<std::shared_ptr<simulation_task>> _tasks_buffer{};
    std::vector<task_group> _groups{};
};

}  // namespace pitaya
