#include "pitaya/three_phase_pll.hpp"

#include <cassert>
#include <cmath>

namespace pitaya {

three_phase_pll::three_phase_pll(mojito::duration_t sampling_time)
    : _sampling_time(sampling_time),
      _frequency_pi_controller(sampling_time.value()),
      _frequency_integrator(sampling_time.value()),
      _lock_timer(0.1f, sampling_time.value()),
      _filter_frequency_estimation(sampling_time.value()),
      _omega_reject_filter(sampling_time.value())
{
    assert(sampling_time.value() > 0.0 && "Sampling time must be positive");

    configure_nominal_frequency(mojito::frequency_t{50.0f});
    configure_pi_controller(default_pi_kp, mojito::duration_t{default_pi_ti});
    configure_frequency_limits(mojito::frequency_t{30.0f}, mojito::frequency_t{70.0f});
    configure_lock_detection(mojito::duration_t{0.1f}, mojito::voltage_pu_t{0.05f});

    _filter_frequency_estimation.configure(10.0f, 0.707);
    _omega_reject_filter.configure(50.0f, 10.0f, 2.0f, 0.5f);

    reset();
}

void three_phase_pll::configure_pi_controller(real_t kp, mojito::duration_t ti)
{
    _frequency_pi_controller.configure_with_ti(kp, static_cast<real_t>(ti.value()));
    _frequency_pi_controller.enable_integrator_clamping(true);
}

void three_phase_pll::configure_frequency_limits(mojito::frequency_t min_freq, mojito::frequency_t max_freq)
{
    const real_t freq_limit_min_pu = (min_freq.value() - _nominal_frequency.value()) / _nominal_frequency.value();
    const real_t freq_limit_max_pu = (max_freq.value() - _nominal_frequency.value()) / _nominal_frequency.value();
    _frequency_pi_controller.set_output_limits(freq_limit_min_pu, freq_limit_max_pu);
}

void three_phase_pll::configure_lock_detection(mojito::duration_t lock_duration, mojito::voltage_pu_t q_error_threshold)
{
    _lock_q_error_threshold = q_error_threshold;
    _lock_timer.configure(static_cast<real_t>(lock_duration.value()));
}

void three_phase_pll::configure_nominal_frequency(mojito::frequency_t nominal_frequency)
{
    _nominal_frequency = nominal_frequency;
    reset();
}

void three_phase_pll::reset()
{
    _frequency_pi_controller.reset();
    _frequency_integrator.reset();
    _omega_reject_filter.reset();
    _lock_timer.reset();

    _voltage_dq = {};
    _estimated_frequency = _nominal_frequency;
    _frequency_integrator.reset(mojito::angle_wrapped::from_radians(mojito::angle_t{0.0f})); // Start at 0
}

void three_phase_pll::update(const mojito::alphabeta<mojito::voltage_pu_t>& voltage_alphabeta)
{
    _voltage_dq = mojito::to_dq(voltage_alphabeta, _frequency_integrator.get_output());

    const real_t frequency_deviation_pu = _frequency_pi_controller.update(_voltage_dq.q().value());

    const real_t filtered_frequency_deviation_pu = _omega_reject_filter.update(
        frequency_deviation_pu, is_locked() ? _estimated_frequency.value() : _nominal_frequency.value());

    const bool is_lock_condition_met = std::abs(_voltage_dq.q().value()) < _lock_q_error_threshold.value();
    _lock_timer.update(is_lock_condition_met);

    const real_t frequency_deviation_hz = filtered_frequency_deviation_pu * _nominal_frequency.value();
    _estimated_frequency = mojito::frequency_t{_nominal_frequency.value() + frequency_deviation_hz};

    _frequency_integrator.update(2.0 * mojito::pi * _estimated_frequency.value());
    _filter_frequency_estimation.update(_estimated_frequency.value());
}

mojito::frequency_t three_phase_pll::get_estimated_frequency() const
{
    return mojito::frequency_t{_filter_frequency_estimation.get_output()};
}

mojito::frequency_pu_t three_phase_pll::get_estimated_frequency_pu() const
{
    return mojito::frequency_pu_t{get_estimated_frequency().value() / _nominal_frequency.value()};
}

mojito::angle_wrapped three_phase_pll::get_estimated_angle() const
{
    const auto actual_frequency = is_locked() ? _estimated_frequency.value() : _nominal_frequency.value();
    return _frequency_integrator.get_output() - mojito::angle_wrapped::from_radians(mojito::angle_t{static_cast<float>(2.0 * mojito::pi * actual_frequency * _sampling_time.value())});
}

const mojito::dq<mojito::voltage_pu_t>& three_phase_pll::get_voltage_dq() const { return _voltage_dq; }

bool three_phase_pll::is_locked() const { return _lock_timer.get_output(); }

mojito::alphabeta<mojito::voltage_pu_t> three_phase_pll::get_estimated_alphabeta() const
{
    return mojito::to_alphabeta(_voltage_dq, get_estimated_angle());
}

}  // namespace pitaya
