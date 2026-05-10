#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "pitaya/types.hpp"

namespace pitaya {

/**
 * @brief Represents a single data point or vector to be logged.
 */
class data_logger_entry {
public:
    using sample_func_t = std::function<std::vector<float>()>;

    data_logger_entry(std::string name, sample_func_t func, std::size_t dimension = 1);

    const std::string& get_name() const;
    std::size_t get_dimension() const;
    std::vector<float> sample() const;

private:
    std::string _name;
    sample_func_t _func;
    std::size_t _dimension;
};

/**
 * @brief Records time-series data during simulation.
 */
class data_logger {
public:
    /**
     * @brief Register a signal to be logged.
     */
    void register_signal(std::string name, std::function<float()> func);

    /**
     * @brief Register a multi-dimensional signal to be logged.
     */
    void register_vector_signal(std::string name, std::function<std::vector<float>()> func, std::size_t dimension);

    /**
     * @brief Pre-allocate memory for the expected number of samples.
     */
    void allocate(std::size_t num_samples);

    /**
     * @brief Capture a sample of all registered signals.
     */
    void capture();

    /**
     * @brief Clear all recorded data but keep registered signals.
     */
    void clear_data();

    /**
     * @brief Get the recorded data for a specific signal by name.
     */
    const std::vector<std::vector<float>>& get_data(const std::string& name) const;

    const std::vector<std::shared_ptr<data_logger_entry>>& get_entries() const;

private:
    std::vector<std::shared_ptr<data_logger_entry>> _entries{};
    std::vector<std::vector<std::vector<float>>> _data_buffers{};
};

}  // namespace pitaya
