#include "pitaya/simulation/simulation_task.hpp"

namespace pitaya {

duration_t lambda_task::get_sampling_time() const { return _sampling_time; }

void lambda_task::run()
{
    if (_run_func) {
        _run_func();
    }
}

void lambda_task::initialize()
{
    if (_initialize_func) {
        _initialize_func();
    }
}

void lambda_task::stop()
{
    if (_stop_func) {
        _stop_func();
    }
}

}  // namespace pitaya
