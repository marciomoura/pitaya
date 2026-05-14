#include "pitaya/simulation/data_exporter.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace pitaya {

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

    // 1. Metadata Header (Commented out)
    ofs << "# PTYA_CSV_VERSION: 1\n";
    for (const auto& entry : entries) {
        ofs << "# SIGNAL: name=" << entry->get_name() << ", group=" << entry->get_group()
            << ", row=" << entry->get_row() << ", col=" << entry->get_col() << ", dim=" << entry->get_dimension()
            << "\n";
    }

    // 2. Column Names Row
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

    // 3. Data Rows
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
