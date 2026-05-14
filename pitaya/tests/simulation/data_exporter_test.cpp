#include "pitaya/simulation/data_exporter.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

class DataExporterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        temp_dir = std::filesystem::temp_directory_path() / "pitaya_tests";
        std::filesystem::create_directories(temp_dir);
    }

    void TearDown() override { std::filesystem::remove_all(temp_dir); }

    std::filesystem::path temp_dir;
};

TEST_F(DataExporterTest, ExportToCSVAndReadBack)
{
    simulator sim;
    sim.register_lambda(duration_t(0.001), []() {});

    // Register with explicit metadata
    sim.register_signal(
        plot_metadata{.name = "signal1", .group = "CustomGroup", .row = 1, .col = 2}, []() { return 1.0f; });

    sim.initialize();
    sim.simulate_steps(1);

    std::filesystem::path csv_path = temp_dir / "test.csv";
    export_to_csv(sim, csv_path);

    ASSERT_TRUE(std::filesystem::exists(csv_path));

    std::ifstream ifs(csv_path);
    std::string line;

    // Check version
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("PTYA_CSV_VERSION: 1") != std::string::npos);

    // Check metadata for "time"
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("SIGNAL: name=time, group=General, row=0, col=0, dim=1") != std::string::npos);

    // Check metadata for "signal1"
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("SIGNAL: name=signal1, group=CustomGroup, row=1, col=2, dim=1") != std::string::npos);

    // Check column names
    std::getline(ifs, line);
    EXPECT_EQ(line, "time,signal1");

    // Check data
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("0,1") != std::string::npos);
}

}  // namespace pitaya
