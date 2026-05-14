#pragma once

#include <cassert>

namespace pitaya {

/// Integrator.
template <typename T>
class integrator {
public:
    integrator() = default;

    /// @param sampling_time Must be positive.
    explicit integrator(double sampling_time) : _sampling_time(sampling_time)
    {
        assert(sampling_time > 0.0 && "Sampling time must be positive");
    }

    void configure(double sampling_time) { _sampling_time = sampling_time; }

    void reset(T value = T{}) { _value = value; }

    /// Forward Euler integration: y[k] = y[k-1] + x[k] * Ts
    template <typename InputT = T>
    T update(InputT input)
    {
        assert(_sampling_time > 0.0 && "Sampling time must be positive");

        // Update integrator using rectangular integration
        _value += static_cast<T>(_sampling_time * input);

        return _value;
    }

    double get_sampling_time() const { return _sampling_time; }
    T get_output() const { return _value; }

    operator T() const { return _value; }

private:
    double _sampling_time{};
    T _value{};
};

}  // namespace pitaya
