#pragma once

#include <cassert>
#include <functional>

#include "pitaya/types.hpp"

namespace pitaya {

/// Interface for a task that can be executed within the simulation framework.
class simulation_task {
public:
    virtual ~simulation_task() = default;

    /// Get the sampling time of this task.
    virtual duration_t get_sampling_time() const = 0;

    /// Run the task logic.
    virtual void run() = 0;

    /// Initialize the task before simulation starts.
    virtual void initialize() {}

    /// Stop the task when simulation ends.
    virtual void stop() {}
};

/// Simulation task that wraps a lambda or function object.
class lambda_task : public simulation_task {
public:
    lambda_task(duration_t sampling_time,
        std::function<void()> run,
        std::function<void()> initialize = {},
        std::function<void()> stop = {})
        : _sampling_time(sampling_time),
          _run_func(std::move(run)),
          _initialize_func(std::move(initialize)),
          _stop_func(std::move(stop))
    {
        assert(_run_func && "Run function must be provided");
        assert(_sampling_time.value() > 0.0 && "Sampling time must be positive");
    }

    duration_t get_sampling_time() const override;

    void run() override;

    void initialize() override;

    void stop() override;

private:
    duration_t _sampling_time{};
    std::function<void()> _run_func{};
    std::function<void()> _initialize_func{};
    std::function<void()> _stop_func{};
};

}  // namespace pitaya
