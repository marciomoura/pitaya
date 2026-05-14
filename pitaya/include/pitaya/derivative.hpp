#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Calculates the discrete-time derivative of a signal.
///
/// Computes the derivative based on the difference between the current and
/// previous input values, divided by the sampling time:
///   y[k] = (u[k] - u[k-1]) / Ts
class derivative {
public:
    /// Constructs the derivative calculator.
    /// The sampling time must be positive.
    explicit derivative(double sampling_time, real_t initial_value = 0.0f);

    /// Configures the sampling time.
    /// The sampling time must be positive.
    void configure_sampling_time(double sampling_time);

    /// Updates the calculator with a new input value and computes the derivative.
    void update(real_t input) noexcept;

    /// Resets the internal state of the calculator.
    void reset(real_t reset_value = 0.0f);

    /// Gets the most recently calculated derivative value [units/s].
    real_t get_output() const noexcept;

private:
    double _sampling_time;
    real_t _previous_input;
    real_t _output{0.0f};
};

}  // namespace pitaya
