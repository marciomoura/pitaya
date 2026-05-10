#include "pitaya/simulation/data_exporter.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace pitaya {

void export_to_binary(const simulator& sim, const std::filesystem::path& file_path)
{
    std::ofstream ofs(file_path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Could not open file for writing: " + file_path.string());
    }

    const auto& logger = sim.get_logger();
    const auto& entries = logger.get_entries();

    if (entries.empty()) {
        return;
    }

    // Determine number of samples from the first signal (all signals have same length in data_logger)
    const std::string& first_name = entries[0]->get_name();
    std::size_t num_samples = logger.get_data(first_name).size();

    // 1. Header
    const char magic[4] = {'P', 'T', 'Y', 'A'};
    ofs.write(magic, 4);

    uint32_t version = 1;
    ofs.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint32_t num_signals = static_cast<uint32_t>(entries.size());
    ofs.write(reinterpret_cast<const char*>(&num_signals), sizeof(num_signals));

    uint32_t num_samples_u32 = static_cast<uint32_t>(num_samples);
    ofs.write(reinterpret_cast<const char*>(&num_samples_u32), sizeof(num_samples_u32));

    // 2. Metadata
    for (const auto& entry : entries) {
        const std::string& name = entry->get_name();
        uint32_t name_len = static_cast<uint32_t>(name.size());
        ofs.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        ofs.write(name.data(), name_len);

        uint32_t dimension = static_cast<uint32_t>(entry->get_dimension());
        ofs.write(reinterpret_cast<const char*>(&dimension), sizeof(dimension));
    }

    // 3. Data
    for (const auto& entry : entries) {
        auto view = logger.get_data(entry->get_name());
        const auto& raw = view.raw_data();
        ofs.write(reinterpret_cast<const char*>(raw.data()), raw.size() * sizeof(float));
    }
}

void export_to_csv(const simulator& sim, const std::filesystem::path& file_path)
{
    std::ofstream ofs(file_path);
    if (!ofs) {
        throw std::runtime_error("Could not open file for writing: " + file_path.string());
    }

    const auto& logger = sim.get_logger();
    const auto& entries = logger.get_entries();

    if (entries.empty()) {
        return;
    }

    // Determine number of samples
    const std::string& first_name = entries[0]->get_name();
    std::size_t num_samples = logger.get_data(first_name).size();

    // 1. Header Row
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        std::size_t dim = entry->get_dimension();
        if (dim == 1) {
            ofs << entry->get_name();
        }
        else {
            for (std::size_t d = 0; d < dim; ++d) {
                ofs << entry->get_name() << "_" << d;
                if (d < dim - 1) ofs << ",";
            }
        }
        if (i < entries.size() - 1) ofs << ",";
    }
    ofs << "\n";

    // 2. Data Rows
    for (std::size_t s = 0; s < num_samples; ++s) {
        for (std::size_t i = 0; i < entries.size(); ++i) {
            auto view = logger.get_data(entries[i]->get_name());
            auto sample = view[s];
            for (std::size_t d = 0; d < sample.dimension(); ++d) {
                ofs << sample[d];
                if (d < sample.dimension() - 1) ofs << ",";
            }
            if (i < entries.size() - 1) ofs << ",";
        }
        ofs << "\n";
    }
}

}  // namespace pitaya
