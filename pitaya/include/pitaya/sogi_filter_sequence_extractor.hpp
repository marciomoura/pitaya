#pragma once

#include <mojito/mojito.hpp>
#include "pitaya/sogi_filter.hpp"

namespace pitaya {

/**
 * @brief Implements a Dual SOGI-based Positive Sequence Component Extractor.
 */
class sogi_filter_sequence_extractor {
public:
    explicit sogi_filter_sequence_extractor(duration_t sampling_time);

    void configure(real_t k, duration_t time_constant_dc_rejection_seconds, bool scale_output = true);

    void reset();

    void update(const mojito::alphabeta<mojito::voltage_pu_t>& v_in, mojito::angular_frequency_t omega);

    const mojito::alphabeta<mojito::voltage_pu_t>& get_positive_sequence() const;

    mojito::voltage_pu_t get_positive_sequence_alpha() const;

    mojito::voltage_pu_t get_positive_sequence_beta() const;

    mojito::alphabeta<mojito::voltage_pu_t> get_alpha_sogi_output() const;

    mojito::alphabeta<mojito::voltage_pu_t> get_beta_sogi_output() const;

    mojito::alphabeta<mojito::voltage_pu_t> get_dc_offset() const;

private:
    duration_t _sampling_time;
    bool _scale_output{true};

    sogi_filter _sogi_alpha;
    sogi_filter _sogi_beta;

    mojito::alphabeta<mojito::voltage_pu_t> _positive_sequence{};
};

}  // namespace pitaya
