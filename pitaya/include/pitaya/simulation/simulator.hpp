#pragma once

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "pitaya/simulation/data_logger.hpp"
#include "pitaya/simulation/data_logger_utils.hpp"
#include "pitaya/simulation/task_scheduler.hpp"

namespace pitaya {

/**
 * @brief Main simulation orchestrator.
 */
class simulator {
public:
    simulator();

    /**
     * @brief Register a task with the simulation.
     */
    void register_task(std::shared_ptr<simulation_task> task);

    /**
     * @brief Register a lambda task with the simulation.
     */
    void register_lambda(duration_t sampling_time, std::function<void()> func);

    /**
     * @brief Register a signal for logging.
     * Uses logger_utils to automatically handle coordinate frames and quantities.
     */
    template <typename Func>
    void register_signal(std::string name, Func func)
    {
        logger_utils::register_signal(_logger, std::move(name), std::move(func));
    }

    /**
     * @brief Register a raw signal callback.
     */
    void register_raw_signal(std::string name, std::function<void(float*)> func, std::size_t dimension)
    {
        _logger.register_signal(std::move(name), std::move(func), dimension);
    }

    /**
     * @brief Set the period at which signals are logged.
     * Must be a multiple of the base simulation period.
     */
    void set_logging_period(duration_t period);

    /**
     * @brief Enable or disable automated logging during run_step.
     */
    void enable_logging(bool enable);

    /**
     * @brief Initialize the simulation.
     */
    void initialize();

    /**
     * @brief Stop the simulation.
     */
    void stop();

    /**
     * @brief Run the simulation for a specified number of steps.
     * @param steps Number of base-rate steps to execute.
     */
    void simulate_steps(std::size_t steps);

    /**
     * @brief Run the simulation for a specified duration.
     * @param duration Total time to simulate.
     */
    void simulate_for(duration_t duration);

    /**
     * @brief Get the current simulation time as a duration.
     */
    duration_t get_current_simulation_time() const;

    /**
     * @brief Get the current simulation time in seconds.
     */
    double get_current_simulation_time_seconds() const;

    const data_logger& get_logger() const { return _logger; }

private:
    /**
     * @brief Advance the simulation by a single step.
     */
    void run_step();

    task_scheduler _scheduler;
    data_logger _logger;
    std::size_t _current_tick{0};

    duration_t _logging_period{0.0};
    std::size_t _logging_ratio{1};
    bool _logging_enabled{true};
};

}  // namespace pitaya