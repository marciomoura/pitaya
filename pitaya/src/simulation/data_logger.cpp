#include "pitaya/simulation/data_logger.hpp"
#include <cassert>

namespace pitaya {

data_logger_entry::data_logger_entry(std::string name, sample_func_t func, std::size_t dimension)
    : _name(std::move(name)), _func(std::move(func)), _dimension(dimension) {
    assert(_dimension > 0 && "Dimension must be positive");
    assert(_func && "Function must be valid");
    assert(_func().size() == _dimension && "Function output dimension must match requested dimension");
}

const std::string& data_logger_entry::get_name() const {
    return _name;
}

std::size_t data_logger_entry::get_dimension() const {
    return _dimension;
}

std::vector<float> data_logger_entry::sample() const {
    return _func();
}

void data_logger::register_signal(std::string name, std::function<float()> func) {
    auto wrapper = [func]() { return std::vector<float>{func()}; };
    _entries.push_back(std::make_shared<data_logger_entry>(std::move(name), std::move(wrapper), 1));
    _data_buffers.emplace_back();
}

void data_logger::register_vector_signal(std::string name, std::function<std::vector<float>()> func, std::size_t dimension) {
    _entries.push_back(std::make_shared<data_logger_entry>(std::move(name), std::move(func), dimension));
    _data_buffers.emplace_back();
}

void data_logger::allocate(std::size_t num_samples) {
    for (auto& buffer : _data_buffers) {
        buffer.reserve(num_samples);
    }
}

void data_logger::capture() {
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        _data_buffers[i].push_back(_entries[i]->sample());
    }
}

void data_logger::clear_data() {
    for (auto& buffer : _data_buffers) {
        buffer.clear();
    }
}

const std::vector<std::vector<float>>& data_logger::get_data(const std::string& name) const {
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        if (_entries[i]->get_name() == name) {
            return _data_buffers[i];
        }
    }
    static const std::vector<std::vector<float>> empty{};
    return empty;
}

const std::vector<std::shared_ptr<data_logger_entry>>& data_logger::get_entries() const {
    return _entries;
}

} // namespace pitaya
