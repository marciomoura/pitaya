#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <utility>

#include "pitaya/simulation/data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/**
 * @brief Helper function to automatically export simulator data using the active Google Test name.
 *
 * Extracts the test suite and test case name from Google Test and saves the
 * simulation data to a binary file named `<TestSuite>_<TestCase>.bin`.
 *
 * @param sim The simulator containing the logged data.
 * @param output_directory Directory where the file will be saved.
 */
inline void export_to_binary_gtest(const simulator& sim, const std::filesystem::path& output_directory = ".")
{
    const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    if (!test_info) {
        return;
    }

    if (sim.get_logger().get_entries().empty()) {
        return;
    }

    std::string suite_name = test_info->test_suite_name();
    std::string test_name = test_info->name();
    std::string filename = suite_name + "_" + test_name + ".bin";
    std::filesystem::path export_path = output_directory / filename;
    export_to_binary(sim, export_path);
}

/**
 * @brief RAII utility that automatically exports data when the test scope ends.
 *
 * This can be instantiated in a GTest fixture's TearDown() or directly in a TEST_F.
 * It ensures the simulation data is exported even if the test fails or exits early.
 */
class gtest_exporter {
public:
    explicit gtest_exporter(const simulator& sim, std::filesystem::path output_directory = ".")
        : _sim(sim), _output_directory(std::move(output_directory))
    {
    }

    ~gtest_exporter() { export_to_binary_gtest(_sim, _output_directory); }

    // Disable copying
    gtest_exporter(const gtest_exporter&) = delete;
    gtest_exporter& operator=(const gtest_exporter&) = delete;

private:
    const simulator& _sim;
    std::filesystem::path _output_directory;
};

}  // namespace pitaya
