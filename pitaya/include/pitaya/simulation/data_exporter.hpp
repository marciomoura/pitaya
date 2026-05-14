#pragma once

#include <filesystem>

#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/**
 * @brief Exports the recorded simulation data to a CSV format.
 *
 * Includes metadata in commented header lines for layout and grouping.
 *
 * @param sim The simulator containing the logged data.
 * @param file_path The path to the output CSV file.
 */
void export_to_csv(const simulator& sim, const std::filesystem::path& file_path);

}  // namespace pitaya
