#pragma once

#include <cmath>
#include <mojito/mojito.hpp>
#include <type_traits>

#include "pitaya/types.hpp"

namespace pitaya {

/// Base class for second-order IIR (Infinite Impulse Response) digital filters.
template <typename T>
class second_order_filter {
public:
    using type = double;

protected:
    /// @param sampling_period Sampling period in seconds
    second_order_filter(type sampling_period) : _ts(sampling_period) {}

    // Compute angular frequency in radians/second from Hz
    static constexpr type compute_omega(type freq) { return type(2.0) * mojito::pi * freq; }

public:
    /// Process input sample through filter.
    T update(T input)
    {
        // Convert input to double for internal calculations
        const double input_double = static_cast<double>(static_cast<real_t>(input));

        // Implement difference equation
        const double output_double =
            -_a1 * _y1_internal - _a2 * _y2_internal + _b0 * input_double + _b1 * _x1_internal + _b2 * _x2_internal;

        // Update state variables
        _x2_internal = _x1_internal;
        _x1_internal = input_double;
        _y2_internal = _y1_internal;
        _y1_internal = output_double;

        // Convert back to output type
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(output_double)};
        }
        else {
            return static_cast<T>(output_double);
        }
    }

    /// Get the last output sample.
    T get_output() const
    {
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(_y1_internal)};
        }
        else {
            return static_cast<T>(_y1_internal);
        }
    }

    /// Get magnitude response at given frequency.
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

    /// Get magnitude response in decibels.
    T get_magnitude_response_db(type freq) const
    {
        double res = 20.0 * std::log10(get_magnitude_response(freq));
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(res)};
        }
        else {
            return static_cast<T>(res);
        }
    }

    /// Get phase response at given frequency.
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

    /// Get phase response in degrees.
    type get_phase_response_degrees(type freq) const { return get_phase_response(freq) * type(180.0) / mojito::pi; }

    /// Reset filter state variables.
    void reset() { _x1_internal = _x2_internal = _y1_internal = _y2_internal = 0.0; }

    /// Reset filter state variables to a specific initial value.
    void reset(T initial_value)
    {
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

}  // namespace pitaya
