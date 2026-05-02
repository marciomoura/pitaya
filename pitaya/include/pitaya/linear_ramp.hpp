#pragma once

#include <type_traits>

#include "pitaya/elapsed_timer.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

// Helper to extract value from quantity types (those with .value() method)
template <typename T>
inline constexpr auto get_numeric_value(const T& val) -> decltype(val.value())
{
    return val.value();
}

// Overload for raw floating-point types
inline constexpr float get_numeric_value(float val) { return val; }

inline constexpr double get_numeric_value(double val) { return val; }

/**
 * @brief A linear ramp generator that interpolates between initial and final values over a specified duration.
 *
 * This class generates a linear ramp from an initial value to a final value over a configurable duration.
 * It provides a completion status.
 *
 * Key features:
 * - Linear interpolation between initial and final values
 * - Completion status (is_finished) when duration has elapsed
 * - Handles constant output (initial == final) by waiting for duration to complete
 * - Real-time safe with deterministic execution time
 *
 * @tparam T The type of the value being ramped (e.g., frequency_t, torque_pu_t, real_t)
 */
template <typename T>
class linear_ramp {
public:
    /**
     * @brief Configuration structure for the linear ramp
     */
    struct config {
        T initial{};               ///< Initial value at start of ramp
        T final{};                 ///< Final value at end of ramp
        duration_t duration{0.0f};  ///< Duration of the ramp [s]
    };

    /**
     * @brief Constructs a linear_ramp with the specified sampling time
     * @param sampling_time The sampling period for the timer [s]
     */
    explicit linear_ramp(duration_t sampling_time) : _timer(sampling_time) {}

    /**
     * @brief Configures the ramp parameters
     * @param cfg Configuration containing initial, final, and duration
     */
    void configure(const config& cfg)
    {
        _config = cfg;

        // Reset the timer whenever we reconfigure
        _timer.reset();

        // Precompute the range for interpolation
        _range = static_cast<real_t>(get_numeric_value(_config.final) - get_numeric_value(_config.initial));
    }

    /**
     * @brief Updates the ramp and returns the current output value
     * @param enable Enable flag for the ramp timer
     * @return Current interpolated value along the ramp
     */
    T update(bool enable)
    {
        // Get elapsed time BEFORE updating the timer
        const duration_t elapsed = _timer.get_elapsed_time();

        // Handle zero or negative duration (immediate transition to final)
        if (_config.duration.value() <= 0.0) {
            _timer.update(enable);  // Still update timer for consistency
            return _config.final;
        }

        // Calculate ramp progress [0.0 to 1.0]
        real_t progress = static_cast<real_t>(elapsed.value() / _config.duration.value());

        // Clamp progress to [0, 1]
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        if (progress < 0.0f) {
            progress = 0.0f;
        }

        // Linear interpolation: output = initial + (final - initial) * progress
        auto output_value = get_numeric_value(_config.initial) + (_range * progress);

        // Update elapsed timer AFTER computing the output
        _timer.update(enable);

        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<real_t>(output_value)};
        } else {
            return static_cast<T>(output_value);
        }
    }

    /**
     * @brief Checks if the ramp has completed (duration has elapsed)
     * @return true if the ramp duration has elapsed, false otherwise
     */
    bool is_finished() const noexcept
    {
        // For zero or negative duration, consider finished immediately
        if (_config.duration.value() <= 0.0) {
            return true;
        }

        // Check if timer has elapsed the configured duration
        return _timer.has_elapsed(_config.duration);
    }

    /**
     * @brief Resets the ramp timer to start from the beginning
     */
    void reset() { _timer.reset(); }

    /**
     * @brief Gets the current elapsed time
     * @return Elapsed time since last reset [s]
     */
    duration_t get_elapsed_time() const noexcept { return _timer.get_elapsed_time(); }

    /**
     * @brief Gets the current configuration
     * @return Current ramp configuration
     */
    const config& get_config() const noexcept { return _config; }

private:
    elapsed_timer _timer;                           ///< Timer for tracking elapsed time
    config _config;                                 ///< Ramp configuration
    real_t _range{0.0f};                            ///< Precomputed range (final - initial)
};

}  // namespace pitaya
