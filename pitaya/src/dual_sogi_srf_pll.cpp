#include "pitaya/dual_sogi_srf_pll.hpp"

#include <cassert>

namespace pitaya {

dual_sogi_srf_pll::dual_sogi_srf_pll(mojito::duration_t sampling_time)
    : _sampling_time(sampling_time), _sequence_extractor(sampling_time), _internal_pll(sampling_time)
{
    assert(sampling_time.value() > 0.0 && "Sampling time must be positive");
    configure_sequence_extractor(1.414f, mojito::duration_t{0.1f});
    reset();
}

void dual_sogi_srf_pll::configure_sequence_extractor(
    real_t k, mojito::duration_t time_constant_dc_offset_rejection, bool scale_output)
{
    _sequence_extractor.configure(k, time_constant_dc_offset_rejection, scale_output);
}

void dual_sogi_srf_pll::configure_pi_controller(real_t kp, mojito::duration_t ti)
{
    _internal_pll.configure_pi_controller(kp, ti);
}

void dual_sogi_srf_pll::configure_frequency_limits(mojito::frequency_t min_freq, mojito::frequency_t max_freq)
{
    _internal_pll.configure_frequency_limits(min_freq, max_freq);
}

void dual_sogi_srf_pll::configure_nominal_frequency(mojito::frequency_t nominal_frequency)
{
    _internal_pll.configure_nominal_frequency(nominal_frequency);
    reset();
}

void dual_sogi_srf_pll::reset()
{
    _sequence_extractor.reset();
    _internal_pll.reset();
    _filtered_voltage_alphabeta = {};
}

void dual_sogi_srf_pll::update(const mojito::alphabeta<mojito::voltage_pu_t>& voltage_alphabeta)
{
    mojito::angular_frequency_t const omega_estimated = mojito::angular_frequency_t{static_cast<float>(2.0 * mojito::pi * _internal_pll.get_estimated_frequency().value())};
    _sequence_extractor.update(voltage_alphabeta, omega_estimated);
    _filtered_voltage_alphabeta = _sequence_extractor.get_positive_sequence();
    _internal_pll.update(_filtered_voltage_alphabeta);
}

mojito::frequency_t dual_sogi_srf_pll::get_estimated_frequency() const { return _internal_pll.get_estimated_frequency(); }

mojito::frequency_pu_t dual_sogi_srf_pll::get_estimated_frequency_pu() const
{
    return _internal_pll.get_estimated_frequency_pu();
}

mojito::angle_wrapped dual_sogi_srf_pll::get_estimated_angle() const { return _internal_pll.get_estimated_angle(); }

bool dual_sogi_srf_pll::is_locked() const { return _internal_pll.is_locked(); }

mojito::voltage_pu_t dual_sogi_srf_pll::get_estimation_error() const
{
    return _internal_pll.get_voltage_dq().q(); // In three_phase_pll, q-axis was error
}

mojito::voltage_pu_t dual_sogi_srf_pll::get_estimated_voltage_magnitude() const
{
    return _internal_pll.get_voltage_dq().magnitude();
}

const mojito::alphabeta<mojito::voltage_pu_t>& dual_sogi_srf_pll::get_positive_sequence() const { return _filtered_voltage_alphabeta; }

}  // namespace pitaya
