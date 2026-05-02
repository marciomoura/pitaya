#include "pitaya/sogi_filter.hpp"

#include <cassert>

namespace pitaya {

sogi_filter::sogi_filter(duration_t sampling_time) : _sampling_time(sampling_time) {
    assert(sampling_time.value() > 0.0 && "Sampling time must be positive");

    _alpha_integrator.configure(sampling_time.value());
    _beta_integrator.configure(sampling_time.value());
    _dc_integrator.configure(sampling_time.value());

    configure(default_k);
    configure_dc_rejection(duration_t{1.0f / 50.0f});
}

void sogi_filter::configure(real_t k) {
    assert(k > 0.0f && "SOGI gain must be positive");
    _k = k;
}

void sogi_filter::configure_dc_rejection(duration_t time_constant) {
    assert(time_constant.value() > 0.0 && "Time constant must be positive");
    _k_dc = 1.0f / static_cast<real_t>(time_constant.value());
}

void sogi_filter::reset() {
    _alpha_integrator.reset();
    _beta_integrator.reset();
    _dc_integrator.reset();
    _output = {};
}

mojito::alphabeta<mojito::voltage_pu_t> sogi_filter::get_output() const { return _output; }

mojito::voltage_pu_t sogi_filter::get_alpha() const { return _output.alpha(); }

mojito::voltage_pu_t sogi_filter::get_beta() const { return _output.beta(); }

mojito::voltage_pu_t sogi_filter::get_dc_offset() const { return mojito::voltage_pu_t{_dc_integrator.get_output()}; }

void sogi_filter::update(mojito::voltage_pu_t input_voltage, mojito::angular_frequency_t omega) {
    real_t const v_alpha_prev = _alpha_integrator.get_output();
    real_t const v_beta_prev = _beta_integrator.get_output();
    real_t const dc_offset_prev = _dc_integrator.get_output();

    real_t const error_sogi = input_voltage.value() - v_alpha_prev - dc_offset_prev;

    real_t const dc_integrator_input = _k_dc * error_sogi;
    _dc_integrator.update(dc_integrator_input);

    real_t const alpha_integrator_input = omega.value() * ((_k * error_sogi) - v_beta_prev);
    _alpha_integrator.update(alpha_integrator_input);
    real_t const v_alpha_new = _alpha_integrator.get_output();

    real_t const beta_integrator_input = omega.value() * v_alpha_new;
    _beta_integrator.update(beta_integrator_input);

    _output = mojito::alphabeta<mojito::voltage_pu_t>{mojito::voltage_pu_t{_alpha_integrator.get_output()},
                                      mojito::voltage_pu_t{_beta_integrator.get_output()}}
                  .rotate_clockwise(mojito::angle_wrapped::from_radians(mojito::angle_t{static_cast<float>(omega.value() * _sampling_time.value())}));
}

}  // namespace pitaya
