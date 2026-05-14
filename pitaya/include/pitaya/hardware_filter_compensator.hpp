#pragma once

#include <mojito/mojito.hpp>

#include "pitaya/types.hpp"

namespace pitaya {

/// Compensates for up to two series-connected first-order hardware low-pass filters.
///
/// Formula: V_comp = G * R(phi) * V
///   phi = lead angle = arctan(f/fc)
///   G = gain = sqrt(1 + (f/fc)^2) -> Saturated to 1.0 to avoid noise amplification.
class hardware_filter_compensator {
public:
    hardware_filter_compensator() = default;

    /// @param num_filters Number of active filter stages (0, 1, or 2).
    void configure(
        mojito::frequency_t cutoff_f1, mojito::frequency_t cutoff_f2 = mojito::frequency_t{0.0f}, int num_filters = 1);

    void reset();

    /// Updates compensation based on current frequency.
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
