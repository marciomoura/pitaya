#pragma once

#include <cmath>
#include <mojito/mojito.hpp>

#include "pitaya/second_order_filter.hpp"

namespace pitaya {

/// Second-order band-reject (notch) filter.
template <typename T>
class second_order_band_reject_filter : public second_order_filter<T> {
public:
    using type = double;

    second_order_band_reject_filter(type sampling_period) : second_order_filter<T>(sampling_period) {}

    void configure(type center_freq, type bandwidth)
    {
        const type fs = type(1.0) / this->get_sampling_time();
        const type omega = type(2.0) * mojito::pi * center_freq / fs;
        const type quality_factor = center_freq / bandwidth;
        const type alpha = std::sin(omega) / (type(2.0) * quality_factor);

        type b0 = type(1.0);
        type b1 = -type(2.0) * std::cos(omega);
        type b2 = type(1.0);
        type a0 = type(1.0) + alpha;
        type a1 = -type(2.0) * std::cos(omega);
        type a2 = type(1.0) - alpha;

        this->_b0 = b0 / a0;
        this->_b1 = b1 / a0;
        this->_b2 = b2 / a0;
        this->_a1 = a1 / a0;
        this->_a2 = a2 / a0;
    }
};

}  // namespace pitaya
