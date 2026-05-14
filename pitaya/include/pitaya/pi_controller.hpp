#pragma once

#include <cassert>
#include <limits>

#include "pitaya/integrator.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/// Proportional-Integral (PI) controller.
template <typename T>
class pi_controller {
public:
    pi_controller() = default;
    pi_controller(double sampling_time) : _integrator(sampling_time) {}

    /// Configure the controller using Kp and continuous-time Ki.
    void configure(real_t kp, real_t ki_continuous)
    {
        _kp = kp;
        _ki = ki_continuous;
    }

    /// Configure the controller using Kp and the integral time constant.
    /// The integral time constant must be positive.
    void configure_with_ti(real_t kp, real_t ti)
    {
        assert(ti > 0.0f && "Integral time constant must be positive");
        _kp = kp;
        _ki = (ti > 0.0f) ? (kp / ti) : 0.0f;
    }

    /// Set output limits for saturation.
    void set_output_limits(real_t min, real_t max)
    {
        if (min < max) {
            _output_min = min;
            _output_max = max;
            _has_output_limits = true;
        }
    }

    /// Enable or disable output limits.
    void enable_output_limits(bool enable) { _has_output_limits = enable; }

    /// Set the anti-windup gain.
    /// The anti-windup gain must be non-negative.
    void set_antiwindup_gain(real_t kc)
    {
        assert(kc >= 0.0f && "Anti-windup gain must be non-negative");
        _kc = kc;
    }

    /// Enable or disable integrator clamping as an anti-windup method.
    void enable_integrator_clamping(bool enable) { _integrator_clamping_enabled = enable; }

    /// Enable or disable the integrator.
    void enable_integrator(bool enable) { _integrator_enabled = enable; }

    /// Check if the output was saturated in the last update.
    bool is_output_saturated() const { return _output_saturated; }

    /// Get the saturation error from the last update.
    T get_saturation_error() const { return _saturation_error; }

    /// Reset the controller state by clearing the integral term.
    void reset(T value = T{})
    {
        _integrator.reset(value);
        _output_saturated = false;
        _saturation_error = T{};
    }

    /// Preset the integrator for bumpless transfer.
    void preset_for_bumpless_transfer(T desired_output, T current_error)
    {
        T integrator_preset = desired_output - _kp * current_error;
        if (_has_output_limits) {
            integrator_preset = clamp_output(integrator_preset);
        }
        _integrator.reset(integrator_preset);
        _output_saturated = false;
        _saturation_error = T{};
    }

    /// Update the PI controller with the current error input.
    T update(T input)
    {
        T const proportional_term = _kp * input;
        T unsaturated_output = proportional_term + _integrator.get_output();

        T saturated_output = unsaturated_output;
        bool is_saturated = false;

        if (_has_output_limits) {
            saturated_output = clamp_output(unsaturated_output);
            is_saturated = !are_equal(saturated_output, unsaturated_output);
        }

        _output_saturated = is_saturated;
        _saturation_error = saturated_output - unsaturated_output;

        if (!_integrator_enabled) {
            return saturated_output;
        }

        if (_integrator_clamping_enabled && _output_saturated) {
            return saturated_output;
        }

        T const integral_term = (_ki * input + _kc * _saturation_error);
        _integrator.update(integral_term);

        return saturated_output;
    }

    // Getter methods.
    real_t get_kp() const { return _kp; }
    real_t get_ki() const { return _ki; }
    double get_sampling_time() const { return _integrator.get_sampling_time(); }
    T get_integral() const { return _integrator.get_output(); }
    bool is_integrator_enabled() const { return _integrator_enabled; }
    bool has_output_limits() const { return _has_output_limits; }
    real_t get_antiwindup_gain() const { return _kc; }

private:
    T clamp_output(const T& output) const
    {
        if constexpr (mojito::is_coordinate_frame<T>::value) {
            T result = output;
            for (size_t i = 0; i < result.size(); ++i) {
                result[i] = std::clamp(static_cast<real_t>(result[i]), _output_min, _output_max);
            }
            return result;
        }
        else if constexpr (mojito::internal::is_quantity<T>::value) {
            return T{std::clamp(output.value(), static_cast<double>(_output_min), static_cast<double>(_output_max))};
        }
        else {
            return std::clamp(output, static_cast<T>(_output_min), static_cast<T>(_output_max));
        }
    }

    bool are_equal(const T& a, const T& b) const
    {
        constexpr real_t tolerance = 1e-9f;
        if constexpr (mojito::is_coordinate_frame<T>::value) {
            for (size_t i = 0; i < a.size(); ++i) {
                if (std::abs(static_cast<real_t>(a[i] - b[i])) > tolerance) return false;
            }
            return true;
        }
        else if constexpr (mojito::internal::is_quantity<T>::value) {
            return std::abs(a.value() - b.value()) <= static_cast<double>(tolerance);
        }
        else {
            return std::abs(a - b) <= static_cast<T>(tolerance);
        }
    }

    real_t _kp{0.0f};
    real_t _ki{0.0f};
    integrator<T> _integrator;
    real_t _output_min{std::numeric_limits<real_t>::lowest()};
    real_t _output_max{std::numeric_limits<real_t>::max()};
    bool _has_output_limits{false};
    real_t _kc{1.0f};
    bool _integrator_clamping_enabled{false};
    bool _integrator_enabled{true};
    bool _output_saturated{false};
    T _saturation_error{};
};

}  // namespace pitaya
