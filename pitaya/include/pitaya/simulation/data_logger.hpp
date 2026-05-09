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

    data_logger_entry(std::string name, sample_func_t func, std::size_t dimension = 1)
        : _name(std::move(name)), _func(std::move(func)), _dimension(dimension)
    {
        assert(_dimension > 0 && "Dimension must be positive");
        assert(_func && "Function must be valid");
        assert(_func().size() == _dimension && "Function output dimension must match requested dimension");
    }

    const std::string& get_name() const { return _name; }
    std::size_t get_dimension() const { return _dimension; }
    std::vector<float> sample() const { return _func(); }

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
    void register_signal(std::string name, std::function<float()> func)
    {
        auto wrapper = [func]() { return std::vector<float>{func()}; };
        _entries.push_back(std::make_shared<data_logger_entry>(std::move(name), std::move(wrapper), 1));
        _data_buffers.emplace_back();
    }

    /**
     * @brief Register a multi-dimensional signal to be logged.
     */
    void register_vector_signal(std::string name, std::function<std::vector<float>()> func, std::size_t dimension)
    {
        _entries.push_back(std::make_shared<data_logger_entry>(std::move(name), std::move(func), dimension));
        _data_buffers.emplace_back();
    }

    /**
     * @brief Pre-allocate memory for the expected number of samples.
     */
    void allocate(std::size_t num_samples)
    {
        for (auto& buffer : _data_buffers) {
            buffer.reserve(num_samples);
        }
    }

    /**
     * @brief Capture a sample of all registered signals.
     */
    void capture()
    {
        for (std::size_t i = 0; i < _entries.size(); ++i) {
            _data_buffers[i].push_back(_entries[i]->sample());
        }
    }

    /**
     * @brief Clear all recorded data but keep registered signals.
     */
    void clear_data()
    {
        for (auto& buffer : _data_buffers) {
            buffer.clear();
        }
    }

    /**
     * @brief Get the recorded data for a specific signal by name.
     */
    const std::vector<std::vector<float>>& get_data(const std::string& name) const
    {
        for (std::size_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i]->get_name() == name) {
                return _data_buffers[i];
            }
        }
        static const std::vector<std::vector<float>> empty{};
        return empty;
    }

    const std::vector<std::shared_ptr<data_logger_entry>>& get_entries() const { return _entries; }

private:
    std::vector<std::shared_ptr<data_logger_entry>> _entries{};
    std::vector<std::vector<std::vector<float>>> _data_buffers{};
};

}  // namespace pitaya
