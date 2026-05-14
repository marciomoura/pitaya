#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Hysteresis limiter (Schmitt trigger).
///
/// Switches state based on input crossing upper and lower thresholds.
class hysteresis_limiter {
public:
    hysteresis_limiter() = default;

    /// @param low_threshold Must be less than or equal to high_threshold.
    void configure_thresholds(real_t low_threshold, real_t high_threshold);

    void update(real_t input) noexcept;

    void reset() noexcept;

    [[nodiscard]] bool get_output() const noexcept;

private:
    real_t _low_threshold{0.0f};
    real_t _high_threshold{0.0f};
    bool _output_state{false};
};

}  // namespace pitaya
