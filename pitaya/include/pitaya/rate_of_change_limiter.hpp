#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Limits the rate of change of a signal (slew rate limiter).
///
/// It ensures that the output signal ramps towards the input signal at a specified
/// maximum rate, preventing instantaneous jumps. The rate limit is symmetrical
/// for both rising and falling signals.
class rate_of_change_limiter {
public:
    explicit rate_of_change_limiter(double sampling_time);

    rate_of_change_limiter(real_t rate_limit_per_second, double sampling_time, real_t initial_value = 0.0f);

    /// Updates the limiter with a new input (target) value.
    ///
    /// This method should be called once per execution cycle. It calculates the
    /// new output by moving the current output towards the input, respecting the
    /// configured rate limit when enabled.
    void update(real_t input);

    /// Gets the current output value of the limiter.
    real_t get_output() const;

    /// Forcibly resets the output to a specific value.
    void reset(real_t reset_value = 0.0f);

    /// Configures or changes the rate limit.
    void configure(real_t rate_limit_per_second);

    /// Enables or disables the rate limiting functionality.
    ///
    /// When disabled, the input passes through directly to the output.
    void configure_enable(bool enable);

    /// Gets the current enable state of the rate limiter.
    bool is_enabled() const;

private:
    double _sampling_time;                ///< System sampling time in seconds.
    real_t _max_change_per_sample{1e6f};  ///< The maximum allowed change in one sample.
    real_t _current_output{};             ///< The current output value of the limiter.
    bool _enabled{};                      ///< Enable/disable flag for rate limiting.
};

}  // namespace pitaya
