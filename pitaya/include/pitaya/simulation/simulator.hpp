#pragma once

#include <chrono>
#include <cmath>
#include <iostream>

#include "pitaya/simulation/data_logger.hpp"
#include "pitaya/simulation/task_scheduler.hpp"

namespace pitaya {

/**
 * @brief Main simulation orchestrator.
 */
class simulator {
public:
    simulator()
    {
        // Automatically log simulation time
        _logger.register_signal("time", [this]() { return static_cast<float>(get_current_simulation_time_seconds()); });
    }

    /**
     * @brief Register a task with the simulation.
     */
    void register_task(std::shared_ptr<simulation_task> task) { _scheduler.register_task(std::move(task)); }

    /**
     * @brief Register a lambda task with the simulation.
     */
    void register_lambda(duration_t sampling_time, std::function<void()> func)
    {
        _scheduler.register_lambda(sampling_time, std::move(func));
    }

    /**
     * @brief Register a signal for logging.
     */
    void register_signal(std::string name, std::function<float()> func)
    {
        _logger.register_signal(std::move(name), std::move(func));
    }

    /**
     * @brief Set the period at which signals are logged.
     * Must be a multiple of the base simulation period.
     */
    void set_logging_period(duration_t period) { _logging_period = period; }

    /**
     * @brief Enable or disable automated logging during run_step.
     */
    void enable_logging(bool enable) { _logging_enabled = enable; }

    /**
     * @brief Initialize the simulation.
     */
    void initialize()
    {
        _scheduler.initialize();
        _current_tick = 0;

        // Calculate logging ratio
        duration_t base = _scheduler.get_base_period();
        if (base.value() > 0.0 && _logging_period.value() > 0.0) {
            auto base_ns = task_scheduler::to_ns(base);
            auto log_ns = task_scheduler::to_ns(_logging_period);
            _logging_ratio = std::max(std::size_t{1}, static_cast<std::size_t>(log_ns.count() / base_ns.count()));
        }
        else {
            _logging_ratio = 1;
        }
    }

    /**
     * @brief Advance the simulation by a specified number of steps.
     * Integrated logging ensures no data points are missed when manually stepping.
     * @param steps Number of base-rate steps to execute.
     */
    void run_step(std::size_t steps = 1)
    {
        for (std::size_t i = 0; i < steps; ++i) {
            // 1. Capture state (before update)
            if (_logging_enabled && (_current_tick % _logging_ratio == 0)) {
                _logger.capture();
            }

            // 2. Execute tasks for this tick
            _scheduler.run_steps(_current_tick, 1);

            // 3. Increment tick
            _current_tick++;
        }
    }

    /**
     * @brief Run the simulation for a specified duration.
     * @param duration Total time to simulate.
     */
    void simulate_for(duration_t duration)
    {
        if (duration.value() <= 0.0) return;

        duration_t base_period = _scheduler.get_base_period();
        if (base_period.value() <= 0.0) {
            assert(duration.value() >= base_period.value() && "Simulation duration must be at least one base period");
            return;
        }

        auto duration_ns = task_scheduler::to_ns(duration);
        auto base_period_ns = task_scheduler::to_ns(base_period);

        std::size_t total_steps = duration_ns.count() / base_period_ns.count();
        std::size_t expected_samples = (total_steps + _logging_ratio - 1) / _logging_ratio;

        _logger.allocate(expected_samples);

        run_step(total_steps);
    }

    /**
     * @brief Get the current simulation time as a duration.
     */
    duration_t get_current_simulation_time() const
    {
        return duration_t(_current_tick * _scheduler.get_base_period().value());
    }

    /**
     * @brief Get the current simulation time in seconds.
     */
    double get_current_simulation_time_seconds() const { return get_current_simulation_time().value(); }

    const data_logger& get_logger() const { return _logger; }

private:
    task_scheduler _scheduler;
    data_logger _logger;
    std::size_t _current_tick{0};

    duration_t _logging_period{0.0};
    std::size_t _logging_ratio{1};
    bool _logging_enabled{true};
};

}  // namespace pitaya
