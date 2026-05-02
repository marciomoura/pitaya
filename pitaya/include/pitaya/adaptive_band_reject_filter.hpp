#pragma once

#include <cassert>
#include <cmath>

#include "pitaya/second_order_filter.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief An adaptive band-reject (notch) filter with two parallel filters for smooth frequency transitions.
 */
template <typename T>
class adaptive_band_reject_filter {
public:
    explicit adaptive_band_reject_filter(double sampling_period)
        : _sampling_time(sampling_period),
          _filters{second_order_band_reject_filter<T>(sampling_period),
                   second_order_band_reject_filter<T>(sampling_period)} {
        assert(sampling_period > 0.0 && "Sampling time must be positive");
    }

    void configure(real_t initial_center_freq, real_t bandwidth, real_t frequency_threshold_hz, real_t settling_time_s) {
        assert(initial_center_freq > 0.0f && "Initial center frequency must be positive");
        assert(bandwidth > 0.0f && "Bandwidth must be positive");
        assert(frequency_threshold_hz > 0.0f && "Frequency threshold must be positive");
        assert(settling_time_s >= 0.0f && "Settling time must be non-negative");

        _initial_center_freq = initial_center_freq;
        _bandwidth = bandwidth;
        _frequency_threshold_hz = frequency_threshold_hz;
        _settling_samples_required = static_cast<int>(settling_time_s / static_cast<float>(_sampling_time));

        reset();
    }

    void reset() {
        _filters[0].configure(_initial_center_freq, _bandwidth);
        _filters[1].configure(_initial_center_freq, _bandwidth);
        _filters[0].reset();
        _filters[1].reset();

        _center_freqs[0] = _initial_center_freq;
        _center_freqs[1] = _initial_center_freq;

        _active_filter_index = 0;
        _is_switching = false;
        _settling_counter = 0;
    }

    T update(T input, real_t estimated_frequency_hz) {
        const T output0 = _filters[0].update(input);
        const T output1 = _filters[1].update(input);

        if (_is_switching) {
            _settling_counter++;
            if (_settling_counter >= _settling_samples_required) {
                _active_filter_index = 1 - _active_filter_index;
                _is_switching = false;
            }
        } else {
            const real_t freq_deviation = std::abs(estimated_frequency_hz - _center_freqs[_active_filter_index]);
            if (freq_deviation > _frequency_threshold_hz) {
                _is_switching = true;
                _settling_counter = 0;
                const int inactive_index = 1 - _active_filter_index;
                _filters[inactive_index].configure(estimated_frequency_hz, _bandwidth);
                _center_freqs[inactive_index] = estimated_frequency_hz;
            }
        }

        return (_active_filter_index == 0) ? output0 : output1;
    }

private:
    double _sampling_time;
    second_order_band_reject_filter<T> _filters[2];

    real_t _initial_center_freq{50.0f};
    real_t _bandwidth{10.0f};
    real_t _frequency_threshold_hz{2.0f};
    int _settling_samples_required{0};

    real_t _center_freqs[2]{};
    int _active_filter_index{0};
    bool _is_switching{false};
    int _settling_counter{0};
};

}  // namespace pitaya
