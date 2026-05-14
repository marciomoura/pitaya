#pragma once

#include <mojito/mojito.hpp>

#include "pitaya/integrator.hpp"
#include "pitaya/pi_controller.hpp"
#include "pitaya/second_order_low_pass_filter.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/// Synchronous-Reference Frame Phase-Locked Loop (SRF-PLL).
class srf_pll {
public:
    struct pi_gains {
        real_t kp;              // Proportional gain
        mojito::duration_t ti;  // Integral time constant
    };

    static constexpr mojito::frequency_t default_estimation_filter_cutoff_frequency = mojito::frequency_t{100.0f};

    explicit srf_pll(mojito::duration_t sampling_time);

    void configure_nominal_frequency(mojito::frequency_t nominal_frequency);

    void configure_pi_controller(real_t kp, mojito::duration_t ti);

    void configure_frequency_estimation_filter(mojito::frequency_t cutoff_frequency);

    void reset();

    void reset(mojito::frequency_pu_t initial_frequency);

    void preset(
        mojito::angle_wrapped preset_angle, mojito::frequency_pu_t preset_frequency = mojito::frequency_pu_t{1.0f});

    void preset_for_bumpless_transfer(mojito::angle_wrapped preset_angle,
        mojito::frequency_pu_t preset_frequency,
        const mojito::alphabeta<mojito::voltage_pu_t>& input_alphabeta);

    void update(const mojito::alphabeta<mojito::voltage_pu_t>& input_alphabeta);

    mojito::angle_wrapped get_estimated_angle() const;

    mojito::angle_wrapped get_estimated_angle_aligned_phase_a() const;

    mojito::frequency_t get_estimated_frequency() const;

    mojito::frequency_pu_t get_estimated_frequency_pu() const;

    mojito::angular_frequency_t get_estimated_angular_frequency() const;

    const mojito::dq<mojito::voltage_pu_t>& get_dq_vector() const;

    mojito::alphabeta<mojito::voltage_pu_t> get_alphabeta() const;

private:
    pi_controller<real_t> _frequency_pi_controller;
    integrator<mojito::angle_wrapped> _phase_integrator;
    second_order_low_pass_filter<real_t> _frequency_estimation_filter;

    mojito::frequency_pu_t _initial_frequency{mojito::frequency_pu_t{1.0f}};
    mojito::frequency_t _nominal_frequency{mojito::frequency_t{50.0f}};

    mojito::frequency_t _estimated_frequency{mojito::frequency_t{50.0f}};
    mojito::dq<mojito::voltage_pu_t> _dq_vector{};
};

}  // namespace pitaya
