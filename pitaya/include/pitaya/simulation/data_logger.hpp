#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pitaya {

/// Lightweight view into a single sample of a signal.
class sample_view {
public:
    sample_view(const float* data, std::size_t dimension) : _data(data), _dimension(dimension)
    {
        assert(dimension > 0 && "Dimension must be positive");
        assert(data != nullptr && "Data pointer must be valid");
    }

    float operator[](std::size_t dim_index) const
    {
        assert(dim_index < _dimension);
        return _data[dim_index];
    }

    std::size_t dimension() const { return _dimension; }
    const float* data() const { return _data; }

    const float* begin() const { return _data; }
    const float* end() const { return _data + _dimension; }

private:
    const float* _data;
    std::size_t _dimension;
};

/// View into the full recorded history of a signal.
/// Hides the underlying flattened memory layout.
class signal_data_view {
public:
    signal_data_view(const std::vector<float>& data, std::size_t dimension) : _data(data), _dimension(dimension)
    {
        assert(dimension > 0 && "Dimension must be positive");
        assert(data.size() % dimension == 0 && "Data size must be a multiple of dimension");
    }

    /// Access a specific sample by index.
    sample_view operator[](std::size_t sample_index) const
    {
        assert(sample_index < size());
        return sample_view(&_data[sample_index * _dimension], _dimension);
    }

    sample_view at(std::size_t sample_index) const { return (*this)[sample_index]; }

    sample_view get_view(std::size_t sample_index) const { return (*this)[sample_index]; }

    /// Get the number of recorded samples.
    std::size_t size() const
    {
        if (_dimension == 0) return 0;
        return _data.size() / _dimension;
    }

    /// Get the number of samples the view can hold without reallocation.
    std::size_t capacity() const
    {
        if (_dimension == 0) return 0;
        return _data.capacity() / _dimension;
    }

    /// Get the dimension of the signal.
    std::size_t dimension() const { return _dimension; }

    /// Access the raw flattened data.
    const std::vector<float>& raw_data() const { return _data; }

private:
    const std::vector<float>& _data;
    std::size_t _dimension;
};

/// Represents a single data point or vector to be logged.
class data_logger_entry {
public:
    using sample_func_t = std::function<void(float*)>;

    data_logger_entry(std::string name, sample_func_t func, std::size_t dimension = 1);

    const std::string& get_name() const;
    std::size_t get_dimension() const;
    void sample(float* out_ptr) const;

private:
    std::string _name;
    sample_func_t _func;
    std::size_t _dimension;
};

/// Records time-series data during simulation.
class data_logger {
public:
    /// Register a signal to be logged.
    void register_signal(std::string name, std::function<void(float*)> func, std::size_t dimension);

    /// Pre-allocate memory for the expected number of samples.
    void allocate(std::size_t num_samples);

    /// Capture a sample of all registered signals.
    void capture();

    /// Clear all recorded data but keep registered signals.
    void clear_data();

    /// Get a view into the recorded data for a specific signal by name.
    signal_data_view get_data(const std::string& name) const;

    const std::vector<std::shared_ptr<data_logger_entry>>& get_entries() const;

private:
    std::vector<std::shared_ptr<data_logger_entry>> _entries{};

    /// Internal storage architecture:
    ///
    /// _signal_buffers[signal_index] -> A single contiguous vector containing ALL samples for that signal.
    ///
    /// For multi-dimensional signals (dimension > 1), samples are INTERLEAVED:
    /// [S0_D0, S0_D1, S0_D2, S1_D0, S1_D1, S1_D2, ..., SN_D0, SN_D1, SN_D2]
    ///
    /// Legend:
    /// S = Sample index (Time)
    /// D = Dimension index (Component, e.g., a, b, c)
    ///
    /// This layout provides optimal cache locality when capturing all components of a signal at once.
    std::vector<std::vector<float>> _signal_buffers{};
};

}  // namespace pitaya
