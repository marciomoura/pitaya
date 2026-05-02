#pragma once

#include <cmath>
#include <type_traits>

#include <mojito/mojito.hpp>
#include "pitaya/types.hpp"

namespace pitaya {

/**
 * Base class for second-order IIR (Infinite Impulse Response) digital filters.
 */
template <typename T>
class second_order_filter {
public:
    using type = double;

protected:
    /**
     * Protected constructor for derived classes
     * @param sampling_period Sampling period in seconds
     */
    second_order_filter(type sampling_period) : _ts(sampling_period) {}

    // Compute angular frequency in radians/second from Hz
    static constexpr type compute_omega(type freq) { return type(2.0) * mojito::pi * freq; }

public:
    /**
     * Process input sample through filter
     * @param input Current input sample x[n]
     * @return Filtered output sample y[n]
     */
    T update(T input)
    {
        // Convert input to double for internal calculations
        const double input_double = static_cast<double>(static_cast<real_t>(input));

        // Implement difference equation
        const double output_double = -_a1 * _y1_internal - _a2 * _y2_internal + _b0 * input_double + _b1 * _x1_internal + _b2 * _x2_internal;

        // Update state variables
        _x2_internal = _x1_internal;
        _x1_internal = input_double;
        _y2_internal = _y1_internal;
        _y1_internal = output_double;

        // Convert back to output type
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(output_double)};
        } else {
            return static_cast<T>(output_double);
        }
    }

    /**
     * Get the last output sample
     */
    T get_output() const { 
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(_y1_internal)};
        } else {
            return static_cast<T>(_y1_internal);
        }
    }

    /**
     * Get magnitude response at given frequency
     */
    type get_magnitude_response(type freq) const
    {
        const type omega = compute_omega(freq);
        const type omega_t = omega * _ts;
        const type cos_omega_t = std::cos(omega_t);
        const type cos_2omega_t = std::cos(type(2.0) * omega_t);

        const type num = _b0 * _b0 + _b1 * _b1 + _b2 * _b2 + type(2.0) * (_b0 * _b1 + _b1 * _b2) * cos_omega_t +
                         type(2.0) * _b0 * _b2 * cos_2omega_t;

        const type den = type(1.0) + _a1 * _a1 + _a2 * _a2 + type(2.0) * (_a1 + _a1 * _a2) * cos_omega_t +
                         type(2.0) * _a2 * cos_2omega_t;

        return std::sqrt(num / den);
    }

    /**
     * Get magnitude response in decibels
     */
    T get_magnitude_response_db(type freq) const { 
        double res = 20.0 * std::log10(get_magnitude_response(freq));
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(res)};
        } else {
            return static_cast<T>(res);
        }
    }

    /**
     * Get phase response at given frequency
     */
    type get_phase_response(type freq) const
    {
        const type omega = compute_omega(freq);
        const type omega_t = omega * _ts;
        const type sin_omega_t = std::sin(omega_t);
        const type sin_2omega_t = std::sin(type(2.0) * omega_t);
        const type cos_omega_t = std::cos(omega_t);
        const type cos_2omega_t = std::cos(type(2.0) * omega_t);

        const type num_real = _b0 + _b1 * cos_omega_t + _b2 * cos_2omega_t;
        const type num_imag = -(_b1 * sin_omega_t + _b2 * sin_2omega_t);
        const type num_phase = std::atan2(num_imag, num_real);

        const type den_real = type(1.0) + _a1 * cos_omega_t + _a2 * cos_2omega_t;
        const type den_imag = -(_a1 * sin_omega_t + _a2 * sin_2omega_t);
        const type den_phase = std::atan2(den_imag, den_real);

        return num_phase - den_phase;
    }

    /**
     * Get phase response in degrees
     */
    type get_phase_response_degrees(type freq) const { return get_phase_response(freq) * type(180.0) / mojito::pi; }

    /**
     * Reset filter state variables
     */
    void reset() { _x1_internal = _x2_internal = _y1_internal = _y2_internal = 0.0; }

    /**
     * Reset filter state variables to a specific initial value
     */
    void reset(T initial_value) { 
        double val = static_cast<double>(static_cast<real_t>(initial_value));
        _x1_internal = _x2_internal = _y1_internal = _y2_internal = val; 
    }

    // Getter methods
    type get_sampling_time() const { return _ts; }
    type get_feedback_coeff_1() const { return _a1; }
    type get_feedback_coeff_2() const { return _a2; }
    type get_feedforward_coeff_0() const { return _b0; }
    type get_feedforward_coeff_1() const { return _b1; }
    type get_feedforward_coeff_2() const { return _b2; }

protected:
    // Filter coefficients
    type _a1{};  // Feedback coefficient 1
    type _a2{};  // Feedback coefficient 2
    type _b0{};  // Feedforward coefficient 0
    type _b1{};  // Feedforward coefficient 1
    type _b2{};  // Feedforward coefficient 2

    // Sampling period
    type _ts;

    // State variables (double precision)
    double _x1_internal{0.0};
    double _x2_internal{0.0};
    double _y1_internal{0.0};
    double _y2_internal{0.0};
};

/**
 * Second-order low-pass filter implementation.
 */
template <typename T>
class second_order_low_pass_filter : public second_order_filter<T> {
public:
    using type = double;

    second_order_low_pass_filter(type sampling_period) : second_order_filter<T>(sampling_period) {}

    void configure(type cutoff_freq, type damping_ratio)
    {
        const type fs = type(1.0) / this->get_sampling_time();
        const type omega = type(2.0) * mojito::pi * cutoff_freq / fs;
        const type cos_omega = std::cos(omega);
        const type sin_omega = std::sin(omega);
        const type alpha = sin_omega / (type(2.0) * damping_ratio);

        type b0 = (type(1.0) - cos_omega) / type(2.0);
        type b1 = type(1.0) - cos_omega;
        type b2 = (type(1.0) - cos_omega) / type(2.0);
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

/**
 * Second-order band-reject (notch) filter implementation.
 */
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

/**
 * Second-order high-pass filter implementation.
 */
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
