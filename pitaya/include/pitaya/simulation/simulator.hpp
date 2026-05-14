#pragma once

#include <functional>
#include <memory>
#include <string>

#include "pitaya/simulation/assertion.hpp"
#include "pitaya/simulation/data_logger.hpp"
#include "pitaya/simulation/data_logger_utils.hpp"
#include "pitaya/simulation/task_scheduler.hpp"

namespace pitaya {

/// Main simulation orchestrator.
class simulator {
public:
    simulator();

    /// Register a task with the simulation.
    void register_task(std::shared_ptr<simulation_task> task);

    /// Register a lambda task with the simulation.
    void register_lambda(duration_t sampling_time, std::function<void()> func);

    /// Register a test assertion with the simulation.
    void register_assertion(std::unique_ptr<simulation_assertion> assertion);

    /// Set a callback to be called when an assertion fails.
    void on_assertion_failure(std::function<void(const std::string&)> callback);

    /// Register a signal for logging with explicit metadata.
    template <typename Func>
    void register_signal(plot_metadata metadata, Func func)
    {
        std::string name = metadata.name;
        logger_utils::register_signal(_logger, std::move(metadata), std::move(func));
    }

    /// Register a signal for logging.
    /// Uses logger_utils to automatically handle coordinate frames and quantities.
    template <typename Func>
    void register_signal(std::string name, Func func)
    {
        register_signal(plot_metadata{.name = std::move(name)}, std::move(func));
    }

    /// Register a raw signal callback with explicit metadata.
    void register_raw_signal(plot_metadata metadata, std::function<void(float*)> func, std::size_t dimension)
    {
        _logger.register_signal(std::move(metadata), std::move(func), dimension);
    }

    /// Register a raw signal callback.
    void register_raw_signal(std::string name, std::function<void(float*)> func, std::size_t dimension)
    {
        register_raw_signal(plot_metadata{.name = std::move(name)}, std::move(func), dimension);
    }

    /// Set the period at which signals are logged.
    /// Must be a multiple of the base simulation period.
    void set_logging_period(duration_t period);

    /// Enable or disable automated logging during run_step.
    void enable_logging(bool enable);

    /// Initialize the simulation.
    void initialize();

    /// Stop the simulation.
    void stop();

    /// Run the simulation for a specified number of steps.
    void simulate_steps(std::size_t steps);

    /// Run the simulation for a specified duration.
    void simulate_for(duration_t duration);

    /// Get the current simulation time as a duration.
    duration_t get_current_simulation_time() const;

    /// Get the current simulation time in seconds.
    double get_current_simulation_time_seconds() const;

    const data_logger& get_logger() const { return _logger; }

private:
    /// Advance the simulation by a single step.
    void run_step();

    task_scheduler _scheduler;
    data_logger _logger;
    std::vector<std::unique_ptr<simulation_assertion>> _assertions;
    std::function<void(const std::string&)> _on_assertion_failure;
    std::size_t _current_tick{0};

    duration_t _logging_period{0.0};
    std::size_t _logging_ratio{1};
    bool _logging_enabled{true};
};

}  // namespace pitaya
