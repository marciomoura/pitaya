#pragma once

#include <mojito/mojito.hpp>

#include "pitaya/integrator.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/// Discretization-Compensated Second-Order Generalized Integrator (SOGI).
class sogi_filter {
public:
    static constexpr real_t default_k = 1.414f;

    explicit sogi_filter(duration_t sampling_time);

    void configure(real_t k);

    void configure_dc_rejection(duration_t time_constant);

    void reset();

    void update(mojito::voltage_pu_t v_in, mojito::angular_frequency_t omega);

    mojito::alphabeta<mojito::voltage_pu_t> get_output() const;

    mojito::voltage_pu_t get_alpha() const;

    mojito::voltage_pu_t get_beta() const;

    mojito::voltage_pu_t get_dc_offset() const;

private:
    duration_t _sampling_time;

    real_t _k{1.414f};
    real_t _k_dc{0.0f};

    integrator<real_t> _alpha_integrator{};
    integrator<real_t> _beta_integrator{};
    integrator<real_t> _dc_integrator{};

    mojito::alphabeta<mojito::voltage_pu_t> _output{};
};

}  // namespace pitaya
