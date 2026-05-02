#pragma once

#include <mojito/mojito.hpp>
#include "pitaya/adaptive_band_reject_filter.hpp"
#include "pitaya/integrator.hpp"
#include "pitaya/on_delay.hpp"
#include "pitaya/pi_controller.hpp"
#include "pitaya/second_order_filter.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief Three-phase phase locked loop (PLL) for frequency and phase estimation
 */
class three_phase_pll {
public:
    static constexpr real_t default_pi_kp = 0.2f;
    static constexpr real_t default_pi_ti = 0.05f;

    explicit three_phase_pll(mojito::duration_t sampling_time);

    void configure_pi_controller(real_t kp, mojito::duration_t ti);

    void configure_frequency_limits(mojito::frequency_t min_freq, mojito::frequency_t max_freq);

    void configure_nominal_frequency(mojito::frequency_t nominal_frequency);

    void configure_lock_detection(mojito::duration_t lock_duration, mojito::voltage_pu_t q_error_threshold);

    void reset();

    void update(const mojito::alphabeta<mojito::voltage_pu_t>& voltage_ab);

    mojito::frequency_t get_estimated_frequency() const;

    mojito::frequency_pu_t get_estimated_frequency_pu() const;

    mojito::angle_wrapped get_estimated_angle() const;

    const mojito::dq<mojito::voltage_pu_t>& get_voltage_dq() const;

    bool is_locked() const;

    mojito::alphabeta<mojito::voltage_pu_t> get_estimated_alphabeta() const;

private:
    mojito::duration_t _sampling_time;

    mojito::frequency_t _nominal_frequency{50.0f};
    mojito::voltage_pu_t _lock_q_error_threshold{0.05f};

    pi_controller<real_t> _frequency_pi_controller;
    integrator<mojito::angle_wrapped> _frequency_integrator;
    on_delay _lock_timer;

    second_order_low_pass_filter<real_t> _filter_frequency_estimation;
    adaptive_band_reject_filter<real_t> _omega_reject_filter;

    mojito::frequency_t _estimated_frequency{50.0f};
    mojito::dq<mojito::voltage_pu_t> _voltage_dq{};
};

}  // namespace pitaya
