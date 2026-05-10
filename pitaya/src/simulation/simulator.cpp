#include "pitaya/simulation/simulator.hpp"

#include <cassert>

namespace pitaya {

simulator::simulator()
{
    // Automatically log simulation time
    register_signal("time", [this]() { return static_cast<float>(get_current_simulation_time_seconds()); });
}

void simulator::register_task(std::shared_ptr<simulation_task> task) { _scheduler.register_task(std::move(task)); }

void simulator::register_lambda(duration_t sampling_time, std::function<void()> func)
{
    _scheduler.register_lambda(sampling_time, std::move(func));
}

void simulator::set_logging_period(duration_t period) { _logging_period = period; }

void simulator::enable_logging(bool enable) { _logging_enabled = enable; }

void simulator::initialize()
{
    _scheduler.initialize();
    _current_tick = 0;

    // Calculate logging ratio
    duration_t base = _scheduler.get_base_period();
    if (base.value() > 0.0) {
        if (_logging_period.value() == 0.0 || _logging_period.value() < base.value()) {
            _logging_period = base;
        }
        auto base_ns = task_scheduler::to_ns(base);
        auto log_ns = task_scheduler::to_ns(_logging_period);
        _logging_ratio = std::max(std::size_t{1}, static_cast<std::size_t>(log_ns.count() / base_ns.count()));
    }
    else {
        _logging_ratio = 1;
    }
}

void simulator::stop() { _scheduler.stop(); }

void simulator::simulate_steps(std::size_t steps)
{
    if (steps == 0) return;

    if (_logging_enabled) {
        std::size_t expected_samples = (steps + _logging_ratio - 1) / _logging_ratio;
        auto& entries = _logger.get_entries();
        if (!entries.empty()) {
            const auto& first_entry = entries[0];
            const auto& data = _logger.get_data(first_entry->get_name());
            std::size_t current_samples = data.size();
            std::size_t current_capacity = data.capacity();

            if (current_samples + expected_samples > current_capacity) {
                _logger.allocate(current_samples + expected_samples);
            }
        }
    }

    for (std::size_t i = 0; i < steps; ++i) {
        run_step();
    }
}

void simulator::run_step()
{
    // 1. Capture state (before update)
    if (_logging_enabled && (_current_tick % _logging_ratio == 0)) {
        _logger.capture();
    }

    // 2. Execute tasks for this tick
    _scheduler.run_steps(_current_tick, 1);

    // 3. Increment tick
    _current_tick++;
}

void simulator::simulate_for(duration_t duration)
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

    simulate_steps(total_steps);
}

duration_t simulator::get_current_simulation_time() const
{
    return duration_t(_current_tick * _scheduler.get_base_period().value());
}

double simulator::get_current_simulation_time_seconds() const { return get_current_simulation_time().value(); }

}  // namespace pitaya