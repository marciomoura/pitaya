#include "pitaya/derivative.hpp"

#include <cassert>

namespace pitaya {

derivative::derivative(double sampling_time, real_t initial_value)
    : _sampling_time(sampling_time), _previous_input(initial_value)
{
    assert(sampling_time > 0.0 && "Sampling time must be positive.");
}

void derivative::configure_sampling_time(double sampling_time)
{
    assert(sampling_time > 0.0 && "Sampling time must be positive.");
    _sampling_time = sampling_time;
}

void derivative::update(real_t input) noexcept
{
    assert(_sampling_time > 0.0);
    _output = (input - _previous_input) / static_cast<real_t>(_sampling_time);
    _previous_input = input;
}

void derivative::reset(real_t reset_value)
{
    _previous_input = reset_value;
    _output = 0.0f;
}

real_t derivative::get_output() const noexcept { return _output; }

}  // namespace pitaya
