#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Elapsed time measurement with configurable sampling time.
///
/// Incremental timer that relies on regular update() calls at the
/// configured sampling period for accurate timing.
class elapsed_timer {
public:
    /// Constructs an elapsed timer with specified sampling time.
    explicit elapsed_timer(duration_t sampling_time);

    /// Configures the sampling time.
    void configure_sampling_time(duration_t sampling_time);

    /// Updates the timer by one sampling period.
    /// When enable is false, the timer is paused.
    void update(bool enable) noexcept;

    /// Resets the timer to zero.
    void reset() noexcept;

    /// Resets the timer to a specific initial value.
    void reset(duration_t initial_time) noexcept;

    /// Gets the current elapsed time in seconds.
    duration_t get_elapsed_time() const noexcept;

    /// Checks if the elapsed time has exceeded a threshold.
    bool has_elapsed(duration_t threshold) const noexcept;

    /// Gets the remaining time until a threshold is reached.
    /// Returns zero if threshold has already been exceeded.
    duration_t get_remaining_time(duration_t threshold) const noexcept;

    /// Checks if the timer is currently enabled.
    bool is_enabled() const noexcept;

private:
    duration_t _sampling_time;
    duration_t _elapsed_time;
    bool _enabled;
};

}  // namespace pitaya
