#pragma once

#ifdef PITAYA_HAS_GTEST

#include <filesystem>

#include "pitaya/simulation/data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/// Helper function to automatically export simulator data using the active Google Test name.
///
/// Extracts the test suite and test case name from Google Test and saves the
/// simulation data to a CSV file named `<TestSuite>_<TestCase>.csv`.
///
/// @param sim The simulator containing the logged data.
/// @param output_directory Directory where the file will be saved.
void export_to_csv_gtest(const simulator& sim, const std::filesystem::path& output_directory = ".");

/// RAII utility that automatically exports data when the test scope ends.
///
/// This can be instantiated in a GTest fixture's TearDown() or directly in a TEST_F.
/// It ensures the simulation data is exported even if the test fails or exits early.
class gtest_exporter {
public:
    explicit gtest_exporter(const simulator& sim, std::filesystem::path output_directory = ".");

    ~gtest_exporter();

    // Disable copying
    gtest_exporter(const gtest_exporter&) = delete;
    gtest_exporter& operator=(const gtest_exporter&) = delete;

private:
    const simulator& _sim;
    std::filesystem::path _output_directory;
};

}  // namespace pitaya

#endif  // PITAYA_HAS_GTEST
