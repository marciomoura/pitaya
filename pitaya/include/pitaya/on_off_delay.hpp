#pragma once

#include "pitaya/off_delay.hpp"
#include "pitaya/on_delay.hpp"

namespace pitaya {

/// Time delay for both rising and falling edge signals.
///
/// This class implements a timer that delays both the activation (on-delay)
/// and deactivation (off-delay) of its output.
/// - When the input goes from false to true, the output becomes true only after
///   the input has remained true for the configured on-delay duration.
/// - When the input goes from true to false, the output becomes false only after
///   the input has remained false for the configured off-delay duration.
class on_off_delay {
public:
    explicit on_off_delay(double sampling_time);

    /// Configures the on and off delay durations.
    void configure(real_t on_delay_s, real_t off_delay_s);

    /// Updates the timer logic with the current input value.
    void update(bool input);

    /// Forcibly resets the timer to its initial, off state.
    void reset();

    /// Gets the current output state of the timer.
    [[nodiscard]] bool get_output() const;

private:
    double _sampling_time;
    on_delay _on_timer;
    off_delay _off_timer;
};

}  // namespace pitaya
