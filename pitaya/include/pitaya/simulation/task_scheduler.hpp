#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <numeric>
#include <vector>

#include "pitaya/simulation/simulation_task.hpp"

namespace pitaya {

/**
 * @brief Schedules simulation tasks based on their individual execution rates.
 */
class task_scheduler {
public:
    /**
     * @brief Helper to convert duration_t to internal precise chrono duration.
     */
    static std::chrono::nanoseconds to_ns(duration_t d);

    /**
     * @brief Structure to group tasks that run at the same rate.
     */
    struct task_group {
        std::size_t normalized_ticks{};
        std::vector<std::shared_ptr<simulation_task>> tasks{};
    };

    /**
     * @brief Register a task with the scheduler.
     * @param task Pointer to the task to register.
     */
    void register_task(std::shared_ptr<simulation_task> task);

    /**
     * @brief Register a lambda as a task.
     * @param sampling_time The execution rate of the lambda.
     * @param func The function to execute.
     */
    void register_lambda(duration_t sampling_time, std::function<void()> func);

    /**
     * @brief Initialize all tasks and prepare the schedule.
     */
    void initialize();

    /**
     * @brief Stop all tasks.
     */
    void stop();

    /**
     * @brief Advance the simulation by a specified number of steps.
     * @param current_tick The starting tick count.
     * @param steps Number of steps to execute.
     */
    void run_steps(std::size_t current_tick, std::size_t steps);

    /**
     * @brief Get the base tick period (the fastest sampling rate).
     * @return Base duration.
     */
    duration_t get_base_period() const;

private:
    /**
     * @brief Normalizes task rates and builds the optimized task groups.
     */
    void rebuild_schedule();

    void sort_task_buffer_and_update_base_period();

    void build_task_groups_by_normalized_ticks();

    void sort_task_groups_by_interval();

    duration_t _base_period{0.0};
    std::vector<std::shared_ptr<simulation_task>> _tasks_buffer{};
    std::vector<task_group> _groups{};
};

}  // namespace pitaya
