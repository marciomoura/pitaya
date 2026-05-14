#pragma once

#include <random>
#include <type_traits>

#include "mojito/mojito.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/// White noise generator for simulation signals.
template <typename T>
class white_noise_generator {
public:
    explicit white_noise_generator(real_t stddev = 0.0, real_t mean = 0.0)
        : _rng(std::random_device{}()), _mean(mean), _stddev(stddev)
    {
        update_distribution();
    }

    void set_parameters(real_t stddev, real_t mean = 0.0)
    {
        _mean = mean;
        _stddev = stddev;
        update_distribution();
    }

    /// Generate noise for types that can be constructed from a float.
    T generate()
    {
        if (_stddev <= 0.0f) {
            return T{_mean};
        }
        return T{static_cast<float>(_distribution(_rng))};
    }

    /// Generate noise for mojito::abc<U> types.
    template <typename U>
    mojito::abc<U> generate_abc()
    {
        if (_stddev <= 0.0f) {
            return mojito::abc<U>{U{_mean}, U{_mean}, U{_mean}};
        }
        return mojito::abc<U>{U{static_cast<float>(_distribution(_rng))}, U{static_cast<float>(_distribution(_rng))},
            U{static_cast<float>(_distribution(_rng))}};
    }

    /// Generate noise for mojito::alphabeta<U> types.
    template <typename U>
    mojito::alphabeta<U> generate_alphabeta()
    {
        if (_stddev <= 0.0f) {
            return mojito::alphabeta<U>{U{_mean}, U{_mean}};
        }
        return mojito::alphabeta<U>{
            U{static_cast<float>(_distribution(_rng))}, U{static_cast<float>(_distribution(_rng))}};
    }

private:
    void update_distribution()
    {
        if (_stddev > 0.0f) {
            _distribution = std::normal_distribution<float>(_mean, _stddev);
        }
    }

    std::mt19937 _rng;
    std::normal_distribution<float> _distribution;
    float _mean{0.0f};
    float _stddev{0.0f};
};

}  // namespace pitaya
