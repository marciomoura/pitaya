#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Boolean debouncer using an integrator-based approach.
///
/// This component accumulates input states over time. Instead of resetting on a
/// single mismatched sample, it increments/decrements an internal state (like a
/// saturating integrator), providing a low-pass effect for boolean signals.
///
/// - ON transition: Input must be predominantly true for on_delay_s.
/// - OFF transition: Input must be predominantly false for off_delay_s.
class boolean_debouncer {
public:
    boolean_debouncer() noexcept = default;

    /// Configures the sampling time.
    void configure_sampling_time(duration_t sampling_time) noexcept;

    /// Configures the debounce durations.
    void configure_delay(duration_t on_delay_s, duration_t off_delay_s) noexcept;

    /// Updates the debouncer state with latest input value.
    void update(bool input) noexcept;

    /// Resets the integrator and output to initial (false) state.
    void reset() noexcept;

    /// Resets the integrator and output to a specific state.
    void reset(bool initial_state) noexcept;

    /// Gets the current debounced output state.
    [[nodiscard]] bool get_output() const noexcept;

    /// Gets the internal integrator state [0.0, 1.0].
    [[nodiscard]] real_t get_integrator() const noexcept;

private:
    void update_increments() noexcept;

    real_t _sampling_time{0.0f};
    real_t _on_delay_s{0.0f};
    real_t _off_delay_s{0.0f};
    real_t _on_increment{0.0f};
    real_t _off_decrement{0.0f};
    real_t _integrator{0.0f};
    bool _output{false};
};

}  // namespace pitaya
