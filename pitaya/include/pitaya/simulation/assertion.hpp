#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "pitaya/simulation/data_logger_utils.hpp"
#include "pitaya/types.hpp"

namespace pitaya {

/// Result of an assertion evaluation.
struct assertion_result {
    bool success{true};
    std::string message{};

    static assertion_result pass() { return {true, ""}; }
    static assertion_result fail(std::string msg) { return {false, std::move(msg)}; }
};

/// Strategy to determine when an assertion should be active.
class assertion_strategy {
public:
    virtual ~assertion_strategy() = default;
    virtual bool is_active(duration_t current_time) const = 0;
    virtual std::string describe() const = 0;
};

class always_active_strategy : public assertion_strategy {
public:
    bool is_active(duration_t) const override { return true; }
    std::string describe() const override { return "always"; }
};

class time_range_strategy : public assertion_strategy {
public:
    time_range_strategy(duration_t start, duration_t end) : _start(start), _end(end) {}
    bool is_active(duration_t current_time) const override { return current_time >= _start && current_time <= _end; }
    std::string describe() const override
    {
        return "from " + std::to_string(_start.value()) + "s to " + std::to_string(_end.value()) + "s";
    }

private:
    duration_t _start;
    duration_t _end;
};

class at_time_strategy : public assertion_strategy {
public:
    at_time_strategy(duration_t time, duration_t tolerance) : _time(time), _tolerance(tolerance) {}
    bool is_active(duration_t current_time) const override
    {
        return std::abs((current_time - _time).value()) <= _tolerance.value();
    }
    std::string describe() const override { return "at " + std::to_string(_time.value()) + "s"; }

private:
    duration_t _time;
    duration_t _tolerance;
};

/// Base class for all simulation test assertions.
class simulation_assertion {
public:
    simulation_assertion(std::string name, std::unique_ptr<assertion_strategy> strategy)
        : _name(std::move(name)), _strategy(std::move(strategy))
    {
    }
    virtual ~simulation_assertion() = default;

    virtual assertion_result evaluate() = 0;

    bool is_active(duration_t current_time) const { return _strategy->is_active(current_time); }
    const std::string& get_name() const { return _name; }
    const assertion_strategy& get_strategy() const { return *_strategy; }

private:
    std::string _name;
    std::unique_ptr<assertion_strategy> _strategy;
};

/// Concrete assertion for range checking.
template <typename T>
class range_assert : public simulation_assertion {
public:
    range_assert(std::string name, std::function<T()> func, T min, T max, std::unique_ptr<assertion_strategy> strategy)
        : simulation_assertion(std::move(name), std::move(strategy)), _func(std::move(func)), _min(min), _max(max)
    {
    }

    assertion_result evaluate() override
    {
        T val = _func();
        if (val < _min || val > _max) {
            return assertion_result::fail("Value " + std::to_string(logger_utils::extract_value(val)) +
                                          " is out of range [" + std::to_string(logger_utils::extract_value(_min)) +
                                          ", " + std::to_string(logger_utils::extract_value(_max)) + "]");
        }
        return assertion_result::pass();
    }

private:
    std::function<T()> _func;
    T _min;
    T _max;
};

/// Concrete assertion for near-value checking.
template <typename T>
class near_assert : public simulation_assertion {
public:
    near_assert(
        std::string name, std::function<T()> func, T expected, T epsilon, std::unique_ptr<assertion_strategy> strategy)
        : simulation_assertion(std::move(name), std::move(strategy)),
          _func(std::move(func)),
          _expected(expected),
          _epsilon(epsilon)
    {
    }

    assertion_result evaluate() override
    {
        T val = _func();
        float diff = std::abs(logger_utils::extract_value(val) - logger_utils::extract_value(_expected));
        if (diff > logger_utils::extract_value(_epsilon)) {
            return assertion_result::fail("Value " + std::to_string(logger_utils::extract_value(val)) +
                                          " is not near " + std::to_string(logger_utils::extract_value(_expected)) +
                                          " (diff=" + std::to_string(diff) +
                                          ", epsilon=" + std::to_string(logger_utils::extract_value(_epsilon)) + ")");
        }
        return assertion_result::pass();
    }

private:
    std::function<T()> _func;
    T _expected;
    T _epsilon;
};

/// Concrete assertion for maximum limit checking.
template <typename T>
class max_assert : public simulation_assertion {
public:
    max_assert(std::string name, std::function<T()> func, T max, std::unique_ptr<assertion_strategy> strategy)
        : simulation_assertion(std::move(name), std::move(strategy)), _func(std::move(func)), _max(max)
    {
    }

    assertion_result evaluate() override
    {
        T val = _func();
        if (val > _max) {
            return assertion_result::fail("Value " + std::to_string(logger_utils::extract_value(val)) +
                                          " exceeds maximum " + std::to_string(logger_utils::extract_value(_max)));
        }
        return assertion_result::pass();
    }

private:
    std::function<T()> _func;
    T _max;
};

/// Concrete assertion for minimum limit checking.
template <typename T>
class min_assert : public simulation_assertion {
public:
    min_assert(std::string name, std::function<T()> func, T min, std::unique_ptr<assertion_strategy> strategy)
        : simulation_assertion(std::move(name), std::move(strategy)), _func(std::move(func)), _min(min)
    {
    }

    assertion_result evaluate() override
    {
        T val = _func();
        if (val < _min) {
            return assertion_result::fail("Value " + std::to_string(logger_utils::extract_value(val)) +
                                          " is below minimum " + std::to_string(logger_utils::extract_value(_min)));
        }
        return assertion_result::pass();
    }

private:
    std::function<T()> _func;
    T _min;
};

/// Generic assertion using a lambda.
class lambda_assert : public simulation_assertion {
public:
    lambda_assert(
        std::string name, std::function<assertion_result()> func, std::unique_ptr<assertion_strategy> strategy)
        : simulation_assertion(std::move(name), std::move(strategy)), _func(std::move(func))
    {
    }

    assertion_result evaluate() override { return _func(); }

private:
    std::function<assertion_result()> _func;
};

// Factory functions for easier creation
inline std::unique_ptr<assertion_strategy> always_active() { return std::make_unique<always_active_strategy>(); }

struct time_range_config {
    duration_t start{};
    duration_t end{};
};

inline std::unique_ptr<assertion_strategy> time_range(const time_range_config& config)
{
    return std::make_unique<time_range_strategy>(config.start, config.end);
}

struct at_time_config {
    duration_t time{};
    duration_t tolerance{0.0};
};

inline std::unique_ptr<assertion_strategy> at_time(const at_time_config& config)
{
    return std::make_unique<at_time_strategy>(config.time, config.tolerance);
}

template <typename T>
struct range_assert_config {
    std::string name;
    std::function<T()> func;
    T min;
    T max;
    std::unique_ptr<assertion_strategy> strategy;
};

template <typename T>
inline std::unique_ptr<simulation_assertion> make_range_assert(range_assert_config<T> config)
{
    if (!config.strategy) config.strategy = always_active();
    return std::make_unique<range_assert<T>>(
        std::move(config.name), std::move(config.func), config.min, config.max, std::move(config.strategy));
}

template <typename T>
struct near_assert_config {
    std::string name;
    std::function<T()> func;
    T expected;
    T epsilon;
    std::unique_ptr<assertion_strategy> strategy;
};

template <typename T>
inline std::unique_ptr<simulation_assertion> make_near_assert(near_assert_config<T> config)
{
    if (!config.strategy) config.strategy = always_active();
    return std::make_unique<near_assert<T>>(
        std::move(config.name), std::move(config.func), config.expected, config.epsilon, std::move(config.strategy));
}

template <typename T>
struct max_assert_config {
    std::string name;
    std::function<T()> func;
    T max;
    std::unique_ptr<assertion_strategy> strategy;
};

template <typename T>
inline std::unique_ptr<simulation_assertion> make_max_assert(max_assert_config<T> config)
{
    if (!config.strategy) config.strategy = always_active();
    return std::make_unique<max_assert<T>>(
        std::move(config.name), std::move(config.func), config.max, std::move(config.strategy));
}

template <typename T>
struct min_assert_config {
    std::string name;
    std::function<T()> func;
    T min;
    std::unique_ptr<assertion_strategy> strategy;
};

template <typename T>
inline std::unique_ptr<simulation_assertion> make_min_assert(min_assert_config<T> config)
{
    if (!config.strategy) config.strategy = always_active();
    return std::make_unique<min_assert<T>>(
        std::move(config.name), std::move(config.func), config.min, std::move(config.strategy));
}

struct lambda_assert_config {
    std::string name;
    std::function<assertion_result()> func;
    std::unique_ptr<assertion_strategy> strategy;
};

inline std::unique_ptr<simulation_assertion> make_lambda_assert(lambda_assert_config config)
{
    if (!config.strategy) config.strategy = always_active();
    return std::make_unique<lambda_assert>(std::move(config.name), std::move(config.func), std::move(config.strategy));
}

}  // namespace pitaya
