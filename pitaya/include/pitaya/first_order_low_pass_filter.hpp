#pragma once

#include <cassert>
#include <cmath>
#include <mojito/mojito.hpp>
#include <type_traits>

#include "pitaya/types.hpp"

namespace pitaya {

/// First-order low-pass filter.
template <typename T>
class first_order_low_pass_filter {
public:
    explicit first_order_low_pass_filter(double sampling_period)
        : _prev_input_internal(0.0), _output_internal(0.0), _ts(sampling_period)
    {
    }

    /// @param cutoff_freq Must be positive and less than Nyquist frequency (fs/2).
    void configure(real_t cutoff_freq)
    {
        const double cutoff_freq_double = static_cast<double>(cutoff_freq);

        // Check for negative frequency
        if (cutoff_freq_double <= 0.0) {
            assert(false && "Cutoff frequency must be positive");
            return;
        }

        // Check Nyquist criterion
        const double nyquist_freq = 0.5 / _ts;
        if (cutoff_freq_double >= nyquist_freq) {
            assert(false && "Cutoff frequency must be less than Nyquist frequency (fs/2)");
            return;
        }

        // Convert Hz to radians/second and pre-warp (all in double precision)
        const double omega_c = 2.0 * mojito::pi * cutoff_freq_double;
        const double omega_w = 2.0 / _ts * std::tan(omega_c * _ts * 0.5);

        // Compute filter coefficients (stored as double)
        const double denom = omega_w * _ts + 2.0;
        _b0 = omega_w * _ts / denom;
        _b1 = _b0;  // Symmetric for first-order lowpass
        _a1 = (omega_w * _ts - 2.0) / denom;
    }

    static constexpr T calculate_cutoff_frequency(T time_constant)
    {
        const double tc_double = static_cast<double>(static_cast<real_t>(time_constant));
        if (tc_double <= 0.0) {
            return T(0.0f);
        }
        const double result = 1.0 / (2.0 * mojito::pi * tc_double);
        return T(static_cast<float>(result));
    }

    /// Calculate time to reach steady state within a specified percentage.
    static constexpr T calculate_settling_time(T cutoff_freq, T percent_of_final_value)
    {
        const double fc_double = static_cast<double>(static_cast<real_t>(cutoff_freq));
        const double percent_double = static_cast<double>(static_cast<real_t>(percent_of_final_value));

        if (fc_double <= 0.0 || percent_double <= 0.0 || percent_double >= 100.0) {
            return T(0.0f);
        }

        // Calculate time constant (τ = 1/(2πfc))
        const double time_constant = 1.0 / (2.0 * mojito::pi * fc_double);

        // Calculate settling time: t = -τ * ln(1 - p/100)
        const double result = -time_constant * std::log(1.0 - percent_double / 100.0);
        return T(static_cast<float>(result));
    }

    T update(T input)
    {
        // Convert input to double for internal calculations
        const double input_double = static_cast<double>(static_cast<real_t>(input));

        // Perform all arithmetic in double precision
        const double output_double = -_a1 * _output_internal + _b0 * input_double + _b1 * _prev_input_internal;

        // Update internal state (double precision)
        _prev_input_internal = input_double;
        _output_internal = output_double;

        // Convert back to output type
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(output_double)};
        }
        else {
            return static_cast<T>(output_double);
        }
    }

    T get_output() const
    {
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(_output_internal)};
        }
        else {
            return static_cast<T>(_output_internal);
        }
    }

    void reset(real_t value = 0.0f)
    {
        const double value_double = static_cast<double>(value);
        _prev_input_internal = value_double;
        _output_internal = value_double;
    }

    /// Calculate magnitude response at given frequency.
    T get_magnitude_response(T freq) const
    {
        const double freq_double = static_cast<double>(static_cast<real_t>(freq));
        const double omega = 2.0 * mojito::pi * freq_double;
        const double omega_t = omega * _ts;
        const double cos_omega_t = std::cos(omega_t);

        // Compute numerator and denominator (all in double precision)
        const double num = _b0 * _b0 + _b1 * _b1 + 2.0 * _b0 * _b1 * cos_omega_t;
        const double den = 1.0 + _a1 * _a1 + 2.0 * _a1 * cos_omega_t;

        const double result = std::sqrt(num / den);
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(result)};
        }
        else {
            return static_cast<T>(result);
        }
    }

    T get_magnitude_response_db(T freq) const
    {
        const double mag = static_cast<double>(static_cast<real_t>(get_magnitude_response(freq)));
        const double result = 20.0 * std::log10(mag);
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(result)};
        }
        else {
            return static_cast<T>(result);
        }
    }

    /// Calculate phase response at given frequency.
    T get_phase_response(T freq) const
    {
        const double freq_double = static_cast<double>(static_cast<real_t>(freq));
        const double omega = 2.0 * mojito::pi * freq_double;
        const double omega_t = omega * _ts;
        const double sin_omega_t = std::sin(omega_t);
        const double cos_omega_t = std::cos(omega_t);

        // Compute numerator phase
        const double num_imag = -(_b0 - _b1) * sin_omega_t;
        const double num_real = (_b0 + _b1) + (_b0 - _b1) * cos_omega_t;
        const double num_phase = std::atan2(num_imag, num_real);

        // Compute denominator phase
        const double den_imag = -_a1 * sin_omega_t;
        const double den_real = 1.0 + _a1 * cos_omega_t;
        const double den_phase = std::atan2(den_imag, den_real);

        const double result = num_phase - den_phase;
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(result)};
        }
        else {
            return static_cast<T>(result);
        }
    }

    T get_phase_response_degrees(T freq) const
    {
        const double phase_rad = static_cast<double>(static_cast<real_t>(get_phase_response(freq)));
        const double result = phase_rad * 180.0 / mojito::pi;
        if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{static_cast<float>(result)};
        }
        else {
            return static_cast<T>(result);
        }
    }

    // Getter methods
    double get_sampling_time() const { return _ts; }
    double get_feedback_coeff_a1() const { return _a1; }
    double get_feedforward_coeff_b0() const { return _b0; }
    double get_feedforward_coeff_b1() const { return _b1; }

private:
    // All coefficients and internal state in double precision
    double _a1{0.0};                   // Feedback coefficient (denominator)
    double _b0{0.0};                   // Current input coefficient (numerator)
    double _b1{0.0};                   // Previous input coefficient (numerator)
    double _prev_input_internal{0.0};  // Previous input sample x[n-1]
    double _output_internal{0.0};      // Previous output sample y[n-1]
    double _ts{0.0};                   // Sampling period in seconds
};

}  // namespace pitaya
