#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Time delay for a rising edge signal (On-Delay Timer).
///
/// This class implements a classic on-delay timer. When the input signal goes
/// from false to true, an internal timer begins counting. The output will only
/// become true if the input remains continuously true for a specified duration.
/// If the input goes false at any point, the timer resets, and the output
/// immediately becomes false.
class on_delay {
public:
    on_delay(real_t delay_s, double sampling_time);

    /// Updates the timer logic with the current input value.
    ///
    /// This method should be called once per execution cycle. It evaluates the
    /// input and updates the internal state of the timer.
    void update(bool input);

    /// Gets the current output state of the timer.
    bool get_output() const;

    /// Forcibly resets the timer to its initial, off state.
    ///
    /// The internal counter is reset, and the output is set to false.
    void reset();

    /// Configures or changes the delay duration of the timer.
    ///
    /// Calling this method also resets the timer to its initial state.
    void configure(real_t delay_s);

private:
    double _sampling_time;      ///< System sampling time in seconds.
    int _samples_required;      ///< The number of consecutive samples required to meet the delay.
    int _sample_counter{0};     ///< The current count of consecutive true input samples.
    bool _output_state{false};  ///< The current boolean output of the timer.
};

}  // namespace pitaya
