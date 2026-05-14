#pragma once

#include <filesystem>

#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/**
 * @brief Exports the recorded simulation data to a custom, performant binary format.
 *
 * Format:
 * - Header:
 *   - Magic: "PTYA" (4 bytes)
 *   - Version: uint32_t (1)
 *   - NumSignals: uint32_t
 *   - NumSamples: uint32_t
 * - For each signal:
 *   - NameLength: uint32_t
 *   - Name: char[NameLength]
 *   - Dimension: uint32_t
 * - For each signal:
 *   - Data: float[NumSamples * Dimension]
 *
 * @param sim The simulator containing the logged data.
 * @param file_path The path to the output binary file.
 */
void export_to_binary(const simulator& sim, const std::filesystem::path& file_path);

/**
 * @brief Exports the recorded simulation data to a CSV format.
 *
 * @param sim The simulator containing the logged data.
 * @param file_path The path to the output CSV file.
 */
void export_to_csv(const simulator& sim, const std::filesystem::path& file_path);

}  // namespace pitaya
