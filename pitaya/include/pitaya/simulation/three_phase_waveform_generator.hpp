#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <optional>
#include <vector>

#include "mojito/mojito.hpp"
#include "pitaya/simulation/white_noise_generator.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

template <typename T>
struct generator_step {
    real_t time{};
    T value{};
};

template <typename T>
struct generator_step_rate_of_change {
    real_t time_start{};
    real_t time_end{};
    T value_delta{};

    T get_incremental_delta(double sampling_time) const
    {
        real_t const delta_time = time_end - time_start;
        assert(delta_time > 0.0f && "Time delta must be greater than 0");
        return value_delta * static_cast<float>(sampling_time) / delta_time;
    }
};

/// A class to generate a three-phase waveform signal with harmonics and noise.
class three_phase_waveform_generator {
private:
    struct signal_harmonics_configuration {
        std::size_t harmonic_order{};
        real_t magnitude{};
    };

    struct signal_harmonics_data {
        mojito::abc<mojito::angle_wrapped> signal_angle{};
        mojito::abc<real_t> signal{};
    };

    struct signal_harmonics {
        signal_harmonics_configuration configuration;
        signal_harmonics_data data;
    };

public:
    explicit three_phase_waveform_generator(double sampling_time) : _sampling_time(static_cast<float>(sampling_time))
    {
        assert(_sampling_time > 0.0f && "Sampling time must be greater than 0");
    }

    void update()
    {
        update_amplitude_frequency_and_angle_steps();
        update_rate_of_change_steps();

        _fundamental_positive_sequence_signal_abc = generate_positive_sequence_signal_abc(_fundamental_signal_angle);
        _fundamental_negative_sequence_signal_abc = generate_negative_sequence_signal_abc(_fundamental_signal_angle);
        _signal_noise_abc = _noise_generator.generate_abc<real_t>();

        for (auto& h : _harmonics) {
            h.data.signal = update_harmonics_signals(h);
        }

        _simulation_time += _sampling_time;
        _fundamental_signal_angle = increment_fundamental_angle(_fundamental_signal_angle);

        for (auto& h : _harmonics) {
            h.data.signal_angle = increment_harmonics_angles(h);
        }
    }

    mojito::abc<real_t> get_fundamental_signal_abc() const
    {
        return _fundamental_positive_sequence_signal_abc + _fundamental_negative_sequence_signal_abc;
    }

    mojito::abc<mojito::angle_wrapped> get_fundamental_signal_angle() const { return _fundamental_signal_angle; }

    mojito::abc<real_t> get_signal_abc() const
    {
        mojito::abc<real_t> full_signal = get_fundamental_signal_abc();
        for (const auto& h : _harmonics) {
            full_signal = full_signal + h.data.signal;
        }
        return full_signal + _signal_noise_abc;
    }

    real_t get_signal_frequency() const { return _signal_frequency; }

    void enable_noise(real_t stddev = 0.1f) { _noise_generator.set_parameters(stddev, 0.0f); }
    void disable_noise() { _noise_generator.set_parameters(0.0f, 0.0f); }

    void set_signal_frequency(real_t frequency) { _signal_frequency = frequency; }

    void set_fundamental_positive_sequence_signal_amplitude(real_t amplitude)
    {
        _fundamental_positive_sequence_signal_amplitude = {amplitude, amplitude, amplitude};
    }

    void set_fundamental_negative_sequence_signal_amplitude(real_t amplitude)
    {
        _fundamental_negative_sequence_signal_amplitude = {amplitude, amplitude, amplitude};
    }

    void set_angle(mojito::angle_wrapped angle) { _fundamental_signal_angle = {angle, angle, angle}; }

    struct step_config {
        real_t time{};
        real_t value{};
    };

    void set_amplitude_step(const step_config& config)
    {
        _step_amplitude = generator_step<mojito::abc<real_t>>{config.time, {config.value, config.value, config.value}};
    }

    void set_frequency_step(const step_config& config)
    {
        _frequency_step = generator_step<real_t>{config.time, config.value};
    }

    void set_angle_step(real_t time, mojito::angle_wrapped angle)
    {
        _angle_step = generator_step<mojito::abc<mojito::angle_wrapped>>{time, {angle, angle, angle}};
    }

    struct frequency_roc_config {
        real_t delta{};
        real_t start_time{};
        real_t end_time{};
    };

    void set_frequency_rate_of_change(const frequency_roc_config& config)
    {
        _rate_of_change_frequency =
            generator_step_rate_of_change<real_t>{config.start_time, config.end_time, config.delta};
    }

private:
    static mojito::abc<real_t> generate_abc_signal(std::size_t harmonic_index,
        mojito::abc<real_t> const& harmonic_amplitude,
        mojito::abc<mojito::angle_wrapped> const& signal_angle,
        mojito::abc<mojito::angle_wrapped> const& phase_shift)
    {
        return mojito::abc<real_t>{
            harmonic_amplitude.a() *
                std::cos(mojito::angle_wrapped::from_radians(
                    mojito::angle_t{signal_angle.a().get_radians().value() +
                                    static_cast<float>(harmonic_index) * phase_shift.a().get_radians().value()})
                        .get_radians()
                        .value()),
            harmonic_amplitude.b() *
                std::cos(mojito::angle_wrapped::from_radians(
                    mojito::angle_t{signal_angle.b().get_radians().value() +
                                    static_cast<float>(harmonic_index) * phase_shift.b().get_radians().value()})
                        .get_radians()
                        .value()),
            harmonic_amplitude.c() *
                std::cos(mojito::angle_wrapped::from_radians(
                    mojito::angle_t{signal_angle.c().get_radians().value() +
                                    static_cast<float>(harmonic_index) * phase_shift.c().get_radians().value()})
                        .get_radians()
                        .value())};
    }

    mojito::abc<mojito::angle_wrapped> increment_fundamental_angle(mojito::abc<mojito::angle_wrapped> angle) const
    {
        float increment = 2.0f * mojito::pi * _signal_frequency * _sampling_time;
        auto inc_angle = mojito::angle_wrapped::from_radians(mojito::angle_t{increment});
        return {angle.a() + inc_angle, angle.b() + inc_angle, angle.c() + inc_angle};
    }

    mojito::abc<mojito::angle_wrapped> increment_harmonics_angles(signal_harmonics const& h) const
    {
        float increment =
            static_cast<float>(h.configuration.harmonic_order) * 2.0f * mojito::pi * _signal_frequency * _sampling_time;
        auto inc_angle = mojito::angle_wrapped::from_radians(mojito::angle_t{increment});
        return {h.data.signal_angle.a() + inc_angle, h.data.signal_angle.b() + inc_angle,
            h.data.signal_angle.c() + inc_angle};
    }

    mojito::abc<real_t> generate_positive_sequence_signal_abc(mojito::abc<mojito::angle_wrapped> const& angle) const
    {
        return generate_abc_signal(1, _fundamental_positive_sequence_signal_amplitude, angle, _phase_shift);
    }

    mojito::abc<real_t> generate_negative_sequence_signal_abc(mojito::abc<mojito::angle_wrapped> angle) const
    {
        return generate_abc_signal(1, _fundamental_negative_sequence_signal_amplitude, angle, -_phase_shift);
    }

    mojito::abc<real_t> update_harmonics_signals(signal_harmonics const& h)
    {
        return generate_abc_signal(h.configuration.harmonic_order,
            {h.configuration.magnitude, h.configuration.magnitude, h.configuration.magnitude}, h.data.signal_angle,
            _phase_shift);
    }

    bool is_at_step_time(float step_time) const
    {
        return (_simulation_time <= step_time + _sampling_time * 0.5f) &&
               (_simulation_time >= step_time - _sampling_time * 0.5f);
    }

    void update_amplitude_frequency_and_angle_steps()
    {
        if (_step_amplitude && is_at_step_time(_step_amplitude->time)) {
            _fundamental_positive_sequence_signal_amplitude =
                _fundamental_positive_sequence_signal_amplitude + _step_amplitude->value;
        }
        if (_frequency_step && is_at_step_time(_frequency_step->time)) {
            _signal_frequency += _frequency_step->value;
        }
        if (_angle_step && is_at_step_time(_angle_step->time)) {
            _fundamental_signal_angle = _fundamental_signal_angle + _angle_step->value;
        }
    }

    void update_rate_of_change_steps()
    {
        if (_rate_of_change_frequency && _simulation_time >= _rate_of_change_frequency->time_start &&
            _simulation_time <= _rate_of_change_frequency->time_end) {
            _signal_frequency += _rate_of_change_frequency->get_incremental_delta(_sampling_time);
        }
    }

    mojito::abc<mojito::angle_wrapped> _phase_shift{mojito::angle_wrapped::from_radians(mojito::angle_t{0.0f}),
        mojito::angle_wrapped::from_radians(mojito::angle_t{-2.0f * mojito::pi / 3.0f}),
        mojito::angle_wrapped::from_radians(mojito::angle_t{2.0f * mojito::pi / 3.0f})};

    float _sampling_time;
    float _simulation_time{0.0f};
    float _signal_frequency{50.0f};

    mojito::abc<mojito::angle_wrapped> _fundamental_signal_angle{};
    mojito::abc<real_t> _signal_noise_abc{};
    mojito::abc<real_t> _fundamental_positive_sequence_signal_abc{};
    mojito::abc<real_t> _fundamental_negative_sequence_signal_abc{};
    mojito::abc<real_t> _fundamental_positive_sequence_signal_amplitude{1.0f, 1.0f, 1.0f};
    mojito::abc<real_t> _fundamental_negative_sequence_signal_amplitude{0.0f, 0.0f, 0.0f};

    std::vector<signal_harmonics> _harmonics;
    std::optional<generator_step<mojito::abc<real_t>>> _step_amplitude;
    std::optional<generator_step<real_t>> _frequency_step;
    std::optional<generator_step<mojito::abc<mojito::angle_wrapped>>> _angle_step;
    std::optional<generator_step_rate_of_change<real_t>> _rate_of_change_frequency;

    white_noise_generator<mojito::abc<real_t>> _noise_generator;
};

}  // namespace pitaya
