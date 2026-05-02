#include "pitaya/srf_pll.hpp"

#include <cassert>
#include <cmath>

namespace pitaya {

srf_pll::srf_pll(mojito::duration_t sampling_time)
    : _frequency_pi_controller(sampling_time.value()),
      _phase_integrator(sampling_time.value()),
      _frequency_estimation_filter(sampling_time.value())
{
    assert(sampling_time.value() > 0.0 && "Sampling time must be positive");
    configure_frequency_estimation_filter(default_estimation_filter_cutoff_frequency);
    configure_pi_controller(0.866f, mojito::duration_t{0.00551f});
    reset();
}

void srf_pll::configure_nominal_frequency(mojito::frequency_t nominal_frequency)
{
    assert(nominal_frequency.value() >= 0.0f && "Nominal frequency must be non-negative");
    _nominal_frequency = nominal_frequency;
}

void srf_pll::configure_pi_controller(real_t kp, mojito::duration_t ti) { _frequency_pi_controller.configure_with_ti(kp, ti.value()); }

void srf_pll::configure_frequency_estimation_filter(mojito::frequency_t cutoff_frequency)
{
    _frequency_estimation_filter.configure(cutoff_frequency.value(), 0.707);
}

void srf_pll::reset() { reset(mojito::frequency_pu_t{0.0f}); }

void srf_pll::reset(mojito::frequency_pu_t initial_frequency)
{
    _dq_vector = {};
    _initial_frequency = initial_frequency;
    _estimated_frequency = mojito::to_si(_initial_frequency, _nominal_frequency);
    _frequency_pi_controller.reset();
    _phase_integrator.reset();
}

void srf_pll::preset(mojito::angle_wrapped preset_angle, mojito::frequency_pu_t preset_frequency)
{
    _initial_frequency = preset_frequency;
    _estimated_frequency = mojito::to_si(_initial_frequency, _nominal_frequency);
    _frequency_pi_controller.reset();
    _frequency_estimation_filter.reset(_estimated_frequency.value());
    _phase_integrator.reset(preset_angle);
}

void srf_pll::preset_for_bumpless_transfer(
    mojito::angle_wrapped preset_angle, mojito::frequency_pu_t preset_frequency, const mojito::alphabeta<mojito::voltage_pu_t>& input_alphabeta)
{
    _initial_frequency = preset_frequency;
    _estimated_frequency = mojito::to_si(_initial_frequency, _nominal_frequency);
    _phase_integrator.reset(preset_angle);

    _dq_vector = mojito::to_dq(input_alphabeta, preset_angle);
    const real_t current_error = _dq_vector.q().value();
    _frequency_pi_controller.preset_for_bumpless_transfer(0.0f, current_error);
    _frequency_estimation_filter.reset(_estimated_frequency.value());
}

void srf_pll::update(const mojito::alphabeta<mojito::voltage_pu_t>& input_alphabeta)
{
    _dq_vector = mojito::to_dq(input_alphabeta, _phase_integrator.get_output());
    const real_t frequency_deviation = _frequency_pi_controller.update(_dq_vector.q().value());
    const real_t raw_frequency_estimation = _nominal_frequency.value() * (_initial_frequency.value() + frequency_deviation);

    _phase_integrator.update(2.0 * mojito::pi * raw_frequency_estimation);
    _frequency_estimation_filter.update(raw_frequency_estimation);
    _estimated_frequency = mojito::frequency_t{_frequency_estimation_filter.get_output()};
}

mojito::angle_wrapped srf_pll::get_estimated_angle() const
{
    const real_t angle_compensation = 0.25f * get_estimated_angular_frequency().value() * static_cast<real_t>(_frequency_pi_controller.get_sampling_time());
    return _phase_integrator.get_output() - mojito::angle_wrapped::from_radians(mojito::angle_t{angle_compensation});
}

mojito::angle_wrapped srf_pll::get_estimated_angle_aligned_phase_a() const
{
    return get_estimated_angle() + mojito::angle_wrapped::from_radians(mojito::angle_t{static_cast<float>(mojito::pi / 2.0)});
}

mojito::frequency_t srf_pll::get_estimated_frequency() const { return _estimated_frequency; }

mojito::frequency_pu_t srf_pll::get_estimated_frequency_pu() const { return mojito::to_pu(_estimated_frequency, _nominal_frequency); }

mojito::angular_frequency_t srf_pll::get_estimated_angular_frequency() const
{
    return mojito::angular_frequency_t{static_cast<float>(2.0 * mojito::pi * _estimated_frequency.value())};
}

const mojito::dq<mojito::voltage_pu_t>& srf_pll::get_dq_vector() const { return _dq_vector; }

mojito::alphabeta<mojito::voltage_pu_t> srf_pll::get_alphabeta() const { return mojito::to_alphabeta(_dq_vector, get_estimated_angle()); }

}  // namespace pitaya
