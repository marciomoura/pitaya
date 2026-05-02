#include "pitaya/sogi_filter_sequence_extractor.hpp"

#include <cassert>

namespace pitaya {

sogi_filter_sequence_extractor::sogi_filter_sequence_extractor(duration_t sampling_time)
    : _sampling_time(sampling_time), _sogi_alpha(sampling_time), _sogi_beta(sampling_time) {
}

void sogi_filter_sequence_extractor::configure(real_t k, duration_t time_constant_dc_rejection_seconds,
                                               bool scale_output) {
    _scale_output = scale_output;
    _sogi_alpha.configure(k);
    _sogi_alpha.configure_dc_rejection(time_constant_dc_rejection_seconds);
    _sogi_beta.configure(k);
    _sogi_beta.configure_dc_rejection(time_constant_dc_rejection_seconds);
    reset();
}

void sogi_filter_sequence_extractor::reset() {
    _sogi_alpha.reset();
    _sogi_beta.reset();
    _positive_sequence = {};
}

void sogi_filter_sequence_extractor::update(const mojito::alphabeta<mojito::voltage_pu_t>& v_in, mojito::angular_frequency_t omega) {
    _sogi_alpha.update(v_in.alpha(), omega);
    const auto sogi_alpha_out = _sogi_alpha.get_output();
    
    _sogi_beta.update(v_in.beta(), omega);
    const auto sogi_beta_out = _sogi_beta.get_output();

    mojito::voltage_pu_t v_alpha_pos = sogi_alpha_out.alpha() - sogi_beta_out.beta();
    mojito::voltage_pu_t v_beta_pos = sogi_beta_out.alpha() + sogi_alpha_out.beta();

    if (_scale_output) {
        v_alpha_pos = mojito::voltage_pu_t{0.5f * v_alpha_pos.value()};
        v_beta_pos = mojito::voltage_pu_t{0.5f * v_beta_pos.value()};
    }

    _positive_sequence = mojito::alphabeta<mojito::voltage_pu_t>{v_alpha_pos, v_beta_pos};
}

const mojito::alphabeta<mojito::voltage_pu_t>& sogi_filter_sequence_extractor::get_positive_sequence() const {
    return _positive_sequence;
}

mojito::voltage_pu_t sogi_filter_sequence_extractor::get_positive_sequence_alpha() const { return _positive_sequence.alpha(); }

mojito::voltage_pu_t sogi_filter_sequence_extractor::get_positive_sequence_beta() const { return _positive_sequence.beta(); }

mojito::alphabeta<mojito::voltage_pu_t> sogi_filter_sequence_extractor::get_alpha_sogi_output() const {
    return _sogi_alpha.get_output();
}

mojito::alphabeta<mojito::voltage_pu_t> sogi_filter_sequence_extractor::get_beta_sogi_output() const { return _sogi_beta.get_output(); }

mojito::alphabeta<mojito::voltage_pu_t> sogi_filter_sequence_extractor::get_dc_offset() const {
    return mojito::alphabeta<mojito::voltage_pu_t>{_sogi_alpha.get_dc_offset(), _sogi_beta.get_dc_offset()};
}

}  // namespace pitaya
