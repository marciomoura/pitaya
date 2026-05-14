#include "pitaya/simulation/data_logger.hpp"

#include <cassert>

namespace pitaya {

data_logger_entry::data_logger_entry(plot_metadata metadata, sample_func_t func, std::size_t dimension)
    : _metadata(std::move(metadata)), _func(std::move(func)), _dimension(dimension)
{
    assert(_dimension > 0 && "Dimension must be positive");
    assert(_func && "Function must be valid");
}

const std::string& data_logger_entry::get_name() const { return _metadata.name; }

const std::string& data_logger_entry::get_group() const { return _metadata.group; }

uint32_t data_logger_entry::get_row() const { return _metadata.row; }

uint32_t data_logger_entry::get_col() const { return _metadata.col; }

std::size_t data_logger_entry::get_dimension() const { return _dimension; }

void data_logger_entry::sample(float* out_ptr) const { _func(out_ptr); }

void data_logger::register_signal(plot_metadata metadata, std::function<void(float*)> func, std::size_t dimension)
{
    _entries.push_back(std::make_shared<data_logger_entry>(std::move(metadata), std::move(func), dimension));
    _signal_buffers.emplace_back();
}

void data_logger::allocate(std::size_t num_samples)
{
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        _signal_buffers[i].reserve(num_samples * _entries[i]->get_dimension());
    }
}

void data_logger::capture()
{
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        auto& buffer = _signal_buffers[i];
        std::size_t dim = _entries[i]->get_dimension();

        std::size_t current_size = buffer.size();
        buffer.resize(current_size + dim);
        _entries[i]->sample(&buffer[current_size]);
    }
}

void data_logger::clear_data()
{
    for (auto& buffer : _signal_buffers) {
        buffer.clear();
    }
}

signal_data_view data_logger::get_data(const std::string& name) const
{
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        if (_entries[i]->get_name() == name) {
            return signal_data_view(_signal_buffers[i], _entries[i]->get_dimension());
        }
    }
    static const std::vector<float> empty_vec{};
    return signal_data_view(empty_vec, 1);  // Dimension 1 for empty view consistency
}

const std::vector<std::shared_ptr<data_logger_entry>>& data_logger::get_entries() const { return _entries; }

}  // namespace pitaya
