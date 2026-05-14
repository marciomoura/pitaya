#pragma once

#include <cmath>
#include <mojito/mojito.hpp>

#include "pitaya/second_order_filter.hpp"

namespace pitaya {

/// Second-order high-pass filter.
template <typename T>
class second_order_high_pass_filter : public second_order_filter<T> {
public:
    using type = double;

    second_order_high_pass_filter(type sampling_period) : second_order_filter<T>(sampling_period) {}

    void configure(type cutoff_freq, type damping_ratio)
    {
        const type fs = type(1.0) / this->get_sampling_time();
        const type omega = type(2.0) * mojito::pi * cutoff_freq / fs;
        const type cos_omega = std::cos(omega);
        const type sin_omega = std::sin(omega);
        const type alpha = sin_omega / (type(2.0) * damping_ratio);

        type b0 = (type(1.0) + cos_omega) / type(2.0);
        type b1 = -(type(1.0) + cos_omega);
        type b2 = (type(1.0) + cos_omega) / type(2.0);
        type a0 = type(1.0) + alpha;
        type a1 = -type(2.0) * cos_omega;
        type a2 = type(1.0) - alpha;

        this->_b0 = b0 / a0;
        this->_b1 = b1 / a0;
        this->_b2 = b2 / a0;
        this->_a1 = a1 / a0;
        this->_a2 = a2 / a0;
    }
};

}  // namespace pitaya
