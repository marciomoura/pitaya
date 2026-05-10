#include "pitaya/simulation/task_scheduler.hpp"

#include <cassert>
#include <cmath>

namespace pitaya {

std::chrono::nanoseconds task_scheduler::to_ns(duration_t d)
{
    return std::chrono::nanoseconds(static_cast<long long>(std::round(d.value() * 1e9)));
}

void task_scheduler::register_task(std::shared_ptr<simulation_task> task)
{
    _tasks_buffer.push_back(std::move(task));
    rebuild_schedule();
}

void task_scheduler::register_lambda(duration_t sampling_time, std::function<void()> func)
{
    register_task(std::make_shared<lambda_task>(sampling_time, std::move(func)));
}

void task_scheduler::initialize()
{
    for (auto& group : _groups) {
        for (auto& task : group.tasks) {
            task->initialize();
        }
    }
}

void task_scheduler::stop()
{
    for (auto& group : _groups) {
        for (auto& task : group.tasks) {
            task->stop();
        }
    }
}

void task_scheduler::run_steps(std::size_t current_tick, std::size_t steps)
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

duration_t task_scheduler::get_base_period() const { return _base_period; }

void task_scheduler::rebuild_schedule()
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

void task_scheduler::sort_task_buffer_and_update_base_period()
{
    // Sort the task-buffer based on sampling-time, lower to higher
    std::sort(_tasks_buffer.begin(), _tasks_buffer.end(),
              [](const std::shared_ptr<simulation_task>& a, const std::shared_ptr<simulation_task>& b) {
                  return a->get_sampling_time().value() < b->get_sampling_time().value();
              });

    // The base period is the smallest sampling time among the tasks
    _base_period = _tasks_buffer[0]->get_sampling_time();
}

void task_scheduler::build_task_groups_by_normalized_ticks()
{
    _groups.clear();
    auto base_ns = to_ns(_base_period);
    for (const auto& task : _tasks_buffer) {
        auto task_ns = to_ns(task->get_sampling_time());

        std::size_t normalized = task_ns.count() / base_ns.count();
        if (normalized == 0) normalized = 1;

        auto it = std::find_if(_groups.begin(), _groups.end(),
                               [normalized](const task_group& g) { return g.normalized_ticks == normalized; });

        if (it != _groups.end()) {
            it->tasks.push_back(task);
        }
        else {
            _groups.push_back({normalized, {task}});
        }
    }
}

void task_scheduler::sort_task_groups_by_interval()
{
    std::sort(_groups.begin(), _groups.end(),
              [](const task_group& a, const task_group& b) { return a.normalized_ticks < b.normalized_ticks; });
}

}  // namespace pitaya
