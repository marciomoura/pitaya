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
    static std::chrono::nanoseconds to_ns(duration_t d)
    {
        return std::chrono::nanoseconds(static_cast<long long>(std::round(d.value() * 1e9)));
    }

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
    void register_task(std::shared_ptr<simulation_task> task)
    {
        _tasks_buffer.push_back(std::move(task));
        rebuild_schedule();
    }

    /**
     * @brief Register a lambda as a task.
     * @param sampling_time The execution rate of the lambda.
     * @param func The function to execute.
     */
    void register_lambda(duration_t sampling_time, std::function<void()> func)
    {
        register_task(std::make_shared<lambda_task>(sampling_time, std::move(func)));
    }

    /**
     * @brief Initialize all tasks and prepare the schedule.
     */
    void initialize()
    {
        for (auto& group : _groups) {
            for (auto& task : group.tasks) {
                task->initialize();
            }
        }
    }

    /**
     * @brief Advance the simulation by a specified number of steps.
     * @param current_tick The starting tick count.
     * @param steps Number of steps to execute.
     */
    void run_steps(std::size_t current_tick, std::size_t steps)
    {
        for (std::size_t i = 0; i < steps; ++i) {
            std::size_t tick = current_tick + i;
            for (auto& group : _groups) {
                if (tick % group.normalized_ticks == 0) {
                    for (auto& task : group.tasks) {
                        task->run();
                    }
                }
            }
        }
    }

    /**
     * @brief Get the base tick period (the fastest sampling rate).
     * @return Base duration.
     */
    duration_t get_base_period() const { return _base_period; }

private:
    /**
     * @brief Normalizes task rates and builds the optimized task groups.
     */
    void rebuild_schedule()
    {
        if (_tasks_buffer.empty()) {
            assert(_tasks_buffer.empty() && "No tasks registered, schedule is empty.");
            _base_period = duration_t(0.0);
            _groups.clear();
            return;
        }

        sort_task_buffer_and_update_base_period();

        build_task_groups_by_normalized_ticks();

        sort_task_groups_by_interval();
    }

    void sort_task_buffer_and_update_base_period()
    {
        // Sort the task-buffer based on sampling-time, lower to higher
        std::sort(_tasks_buffer.begin(), _tasks_buffer.end(),
                  [](const std::shared_ptr<simulation_task>& a, const std::shared_ptr<simulation_task>& b) {
                      return a->get_sampling_time().value() < b->get_sampling_time().value();
                  });

        // The base period is the smallest sampling time among the tasks
        _base_period = _tasks_buffer[0]->get_sampling_time();
    }

    void build_task_groups_by_normalized_ticks()
    {
        _groups.clear();
        auto base_ns = to_ns(_base_period);
        for (const auto& task : _tasks_buffer) {
            auto task_ns = to_ns(task->get_sampling_time());

            std::size_t normalized = task_ns.count() / base_ns.count();
            if (normalized == 0) normalized = 1;

            auto it = std::find_if(_groups.begin(), _groups.end(), [normalized](const task_group& group) {
                return group.normalized_ticks == normalized;
            });

            if (it != _groups.end()) {
                it->tasks.push_back(task);
            }
            else {
                _groups.push_back({normalized, {task}});
            }
        }
    }

    void sort_task_groups_by_interval()
    {
        std::sort(_groups.begin(), _groups.end(),
                  [](const task_group& a, const task_group& b) { return a.normalized_ticks < b.normalized_ticks; });
    }

    duration_t _base_period{0.0};
    std::vector<std::shared_ptr<simulation_task>> _tasks_buffer{};
    std::vector<task_group> _groups{};
};

}  // namespace pitaya
