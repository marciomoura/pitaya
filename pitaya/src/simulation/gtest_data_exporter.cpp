#include "pitaya/simulation/gtest_data_exporter.hpp"

#ifdef PITAYA_HAS_GTEST
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "pitaya/simulation/data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

void export_to_csv_gtest(const simulator& sim, const std::filesystem::path& output_directory)
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
    std::string filename = suite_name + "_" + test_name + ".csv";
    std::filesystem::path export_path = output_directory / filename;
    export_to_csv(sim, export_path);
}

gtest_exporter::gtest_exporter(const simulator& sim, std::filesystem::path output_directory)
    : _sim(sim), _output_directory(std::move(output_directory))
{
}

gtest_exporter::~gtest_exporter() { export_to_csv_gtest(_sim, _output_directory); }

}  // namespace pitaya

#endif  // PITAYA_HAS_GTEST
