#pragma once

#include "pitaya/types.hpp"

namespace pitaya {

/// Microsecond-precision timer for measuring time intervals.
///
/// Accumulates elapsed time based on a fixed sampling period.
class interval_timer {
public:
    /// @param sampling_time Must be positive.
    explicit interval_timer(double sampling_time);

    void start();

    /// @return Elapsed time in microseconds.
    real_t stop();

    /// Accumulate elapsed time. Call once per control cycle.
    void update();

    bool is_running() const;

    /// @return Elapsed time in microseconds.
    real_t get_elapsed_time() const;

    void reset();

    /// @param sampling_time Must be positive.
    void configure(double sampling_time);

private:
    double _sampling_time;
    real_t _elapsed_time_us{0.0};
    bool _is_running{false};
};

}  // namespace pitaya
