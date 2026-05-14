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

TEST_F(DataExporterTest, ExportToBinaryV2AndReadBack)
{
    simulator sim;
    sim.register_lambda(duration_t(0.001), []() {});

    // Register with explicit metadata
    sim.register_signal(
        plot_metadata{.name = "signal1", .group = "CustomGroup", .row = 1, .col = 2}, []() { return 1.0f; });

    sim.initialize();
    sim.simulate_steps(1);

    std::filesystem::path bin_path = temp_dir / "test_v2.bin";
    export_to_binary(sim, bin_path);

    ASSERT_TRUE(std::filesystem::exists(bin_path));

    std::ifstream ifs(bin_path, std::ios::binary);
    char magic[4];
    ifs.read(magic, 4);

    uint32_t version;
    ifs.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 2);

    uint32_t num_signals;
    ifs.read(reinterpret_cast<char*>(&num_signals), sizeof(num_signals));
    EXPECT_EQ(num_signals, 2);  // "time" + "signal1"

    uint32_t num_samples;
    ifs.read(reinterpret_cast<char*>(&num_samples), sizeof(num_samples));
    EXPECT_EQ(num_samples, 1);

    // Metadata for "time"
    uint32_t len;
    ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string name(len, ' ');
    ifs.read(&name[0], len);
    EXPECT_EQ(name, "time");

    ifs.read(reinterpret_cast<char*>(&len), sizeof(len));  // group len
    std::string group(len, ' ');
    ifs.read(&group[0], len);
    EXPECT_EQ(group, "General");

    uint32_t row, col, dim;
    ifs.read(reinterpret_cast<char*>(&row), sizeof(row));
    ifs.read(reinterpret_cast<char*>(&col), sizeof(col));
    ifs.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    EXPECT_EQ(row, 0);
    EXPECT_EQ(col, 0);
    EXPECT_EQ(dim, 1);

    // Metadata for "signal1"
    ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
    name.resize(len);
    ifs.read(&name[0], len);
    EXPECT_EQ(name, "signal1");

    ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
    group.resize(len);
    ifs.read(&group[0], len);
    EXPECT_EQ(group, "CustomGroup");

    ifs.read(reinterpret_cast<char*>(&row), sizeof(row));
    ifs.read(reinterpret_cast<char*>(&col), sizeof(col));
    ifs.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    EXPECT_EQ(row, 1);
    EXPECT_EQ(col, 2);
    EXPECT_EQ(dim, 1);
}

}  // namespace pitaya
