#pragma once

#include <mojito/mojito.hpp>
#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief Compensates for up to two series-connected first-order hardware low-pass filters.
 *
 * The compensation formula for a single filter is:
 *   V_comp = G * R(phi) * V
 * where phi = lead angle = arctan(f/fc)
 * and G = gain = sqrt(1 + (f/fc)^2) -> Saturated to 1.0 to avoid noise amplification.
 */
class hardware_filter_compensator {
public:
    hardware_filter_compensator() = default;

    /**
     * @brief Configures the hardware filter cut-off frequencies.
     * @param cutoff_f1 The cut-off frequency of the first filter stage.
     * @param cutoff_f2 The cut-off frequency of the second filter stage (optional).
     * @param num_filters The number of active filter stages to compensate (0, 1, or 2).
     */
    void configure(mojito::frequency_t cutoff_f1, mojito::frequency_t cutoff_f2 = mojito::frequency_t{0.0f}, int num_filters = 1);

    /**
     * @brief Resets the compensation state.
     */
    void reset();

    /**
     * @brief Updates the compensation based on the current frequency.
     * @param input The three-phase input signal to be compensated.
     * @param frequency The current estimated frequency of the system.
     * @return The compensated three-phase signal.
     */
    template <typename T>
    mojito::abc<T> update(const mojito::abc<T>& input, mojito::frequency_t frequency)
    {
        if (_num_filters <= 0 || frequency.value() <= 0.0f) {
            return input;
        }

        mojito::angle_wrapped total_phase{0.0f};
        real_t total_gain = 1.0f;

        // Stage 1
        if (_num_filters >= 1 && _cutoff_f1.value() > 0.0f) {
            const real_t ratio = frequency.value() / _cutoff_f1.value();
            total_phase = total_phase + mojito::angle_wrapped::from_radians(mojito::angle_t{std::atan(ratio)});
            total_gain *= std::sqrt(1.0f + ratio * ratio);
        }

        // Stage 2
        if (_num_filters >= 2 && _cutoff_f2.value() > 0.0f) {
            const real_t ratio = frequency.value() / _cutoff_f2.value();
            total_phase = total_phase + mojito::angle_wrapped::from_radians(mojito::angle_t{std::atan(ratio)});
            total_gain *= std::sqrt(1.0f + ratio * ratio);
        }

        // Saturate gain to 1.0 to avoid noise amplification
        const real_t gain_comp = std::min(total_gain, 1.0f);

        // Convert to alpha-beta for rotation
        const mojito::alphabeta<T> ab = mojito::to_alphabeta(input);
        const mojito::alphabeta<T> ab_scaled = gain_comp * ab;

        return mojito::to_abc(ab_scaled.rotate_counter_clockwise(total_phase));
    }

private:
    int _num_filters{0};
    mojito::frequency_t _cutoff_f1{0.0f};
    mojito::frequency_t _cutoff_f2{0.0f};
};

}  // namespace pitaya
