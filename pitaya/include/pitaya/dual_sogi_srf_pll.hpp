#pragma once

#include <mojito/mojito.hpp>
#include "pitaya/sogi_filter_sequence_extractor.hpp"
#include "pitaya/three_phase_pll.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief Robust three-phase PLL using a Dual-SOGI sequence extractor as a pre-filter.
 */
class dual_sogi_srf_pll {
public:
    explicit dual_sogi_srf_pll(mojito::duration_t sampling_time);

    void configure_sequence_extractor(real_t k, mojito::duration_t time_constant_dc_offset_rejection, bool scale_output = true);

    void configure_pi_controller(real_t kp, mojito::duration_t ti);

    void configure_frequency_limits(mojito::frequency_t min_freq, mojito::frequency_t max_freq);

    void configure_nominal_frequency(mojito::frequency_t nominal_frequency);

    void reset();

    void update(const mojito::alphabeta<mojito::voltage_pu_t>& v_in);

    mojito::frequency_t get_estimated_frequency() const;

    mojito::frequency_pu_t get_estimated_frequency_pu() const;

    mojito::angle_wrapped get_estimated_angle() const;

    bool is_locked() const;

    mojito::voltage_pu_t get_estimation_error() const;

    mojito::voltage_pu_t get_estimated_voltage_magnitude() const;

    const mojito::alphabeta<mojito::voltage_pu_t>& get_positive_sequence() const;

private:
    mojito::duration_t _sampling_time;

    sogi_filter_sequence_extractor _sequence_extractor;
    three_phase_pll _internal_pll;

    mojito::alphabeta<mojito::voltage_pu_t> _filtered_voltage_alphabeta{};
};

}  // namespace pitaya
