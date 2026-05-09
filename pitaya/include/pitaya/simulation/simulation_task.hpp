#pragma once

#include <chrono>
#include <functional>

#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief Interface for a task that can be executed within the simulation framework.
 */
class simulation_task {
public:
    virtual ~simulation_task() = default;

    /**
     * @brief Get the sampling time of this task.
     * @return Sampling time as a duration.
     */
    virtual duration_t get_sampling_time() const = 0;

    /**
     * @brief Run the task logic.
     */
    virtual void run() = 0;

    /**
     * @brief Initialize the task before simulation starts.
     * Default implementation does nothing.
     */
    virtual void initialize() {}

    /**
     * @brief Stop the task when simulation ends.
     * Default implementation does nothing.
     */
    virtual void stop() {}
};

/**
 * @brief A simulation task that wraps a lambda or function object.
 */
class lambda_task : public simulation_task {
public:
    lambda_task(duration_t sampling_time, std::function<void()> task_func)
        : _sampling_time(sampling_time), _task_func(std::move(task_func))
    {
    }

    duration_t get_sampling_time() const override { return _sampling_time; }

    void run() override
    {
        if (_task_func) {
            _task_func();
        }
    }

private:
    duration_t _sampling_time;
    std::function<void()> _task_func;
};

}  // namespace pitaya
