#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cmath>

#include "pitaya/simulation/simulator.hpp"
#include "pitaya/simulation/data_exporter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"

namespace pitaya {

class DataExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        temp_dir = std::filesystem::temp_directory_path() / "pitaya_tests";
        std::filesystem::create_directories(temp_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir);
    }

    std::filesystem::path temp_dir;
};

TEST_F(DataExporterTest, ExportToBinaryAndReadBack) {
    simulator sim;
    // Register a dummy task to set the base period
    sim.register_lambda(duration_t(0.001), []() {});

    float signal1_val = 1.0f;
    float signal2_a = 10.0f, signal2_b = 20.0f;

    sim.register_signal("signal1", [&]() { return signal1_val; });
    sim.register_raw_signal("signal2", [&](float* out) {
        out[0] = signal2_a;
        out[1] = signal2_b;
    }, 2);

    sim.initialize();
    
    // Step 0: capture happens at t=0
    // Step 1: capture happens at t=0.001
    signal1_val = 1.1f; signal2_a = 11.0f; signal2_b = 21.0f;
    sim.simulate_steps(1);

    // Step 2: capture happens at t=0.002
    signal1_val = 1.2f; signal2_a = 12.0f; signal2_b = 22.0f;
    sim.simulate_steps(1);

    std::filesystem::path bin_path = temp_dir / "test_data.bin";
    export_to_binary(sim, bin_path);

    ASSERT_TRUE(std::filesystem::exists(bin_path));

    // Read back and verify
    std::ifstream ifs(bin_path, std::ios::binary);
    char magic[4];
    ifs.read(magic, 4);
    EXPECT_EQ(magic[0], 'P'); EXPECT_EQ(magic[1], 'T'); EXPECT_EQ(magic[2], 'Y'); EXPECT_EQ(magic[3], 'A');

    uint32_t version;
    ifs.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 1);

    uint32_t num_signals;
    ifs.read(reinterpret_cast<char*>(&num_signals), sizeof(num_signals));
    EXPECT_EQ(num_signals, 3); // "time", "signal1", "signal2"

    uint32_t num_samples;
    ifs.read(reinterpret_cast<char*>(&num_samples), sizeof(num_samples));
    EXPECT_EQ(num_samples, 2);

    // 1. Metadata for "time"
    uint32_t name_len;
    ifs.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    std::string name(name_len, ' ');
    ifs.read(&name[0], name_len);
    EXPECT_EQ(name, "time");
    uint32_t dim;
    ifs.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    EXPECT_EQ(dim, 1);

    // 2. Metadata for "signal1"
    ifs.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    name.resize(name_len);
    ifs.read(&name[0], name_len);
    EXPECT_EQ(name, "signal1");
    ifs.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    EXPECT_EQ(dim, 1);

    // 3. Metadata for "signal2"
    ifs.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    name.resize(name_len);
    ifs.read(&name[0], name_len);
    EXPECT_EQ(name, "signal2");
    ifs.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    EXPECT_EQ(dim, 2);

    // Data for "time"
    std::vector<float> time_data(2);
    ifs.read(reinterpret_cast<char*>(time_data.data()), 2 * sizeof(float));
    EXPECT_FLOAT_EQ(time_data[0], 0.0f);
    EXPECT_FLOAT_EQ(time_data[1], 0.001f);

    // Data for signal1
    std::vector<float> data1(2);
    ifs.read(reinterpret_cast<char*>(data1.data()), 2 * sizeof(float));
    EXPECT_FLOAT_EQ(data1[0], 1.1f);
    EXPECT_FLOAT_EQ(data1[1], 1.2f);

    // Data for signal2
    std::vector<float> data2(4);
    ifs.read(reinterpret_cast<char*>(data2.data()), 4 * sizeof(float));
    EXPECT_FLOAT_EQ(data2[0], 11.0f);
    EXPECT_FLOAT_EQ(data2[1], 21.0f);
    EXPECT_FLOAT_EQ(data2[2], 12.0f);
    EXPECT_FLOAT_EQ(data2[3], 22.0f);
}

TEST_F(DataExporterTest, ExportToCSV) {
    simulator sim;
    sim.register_lambda(duration_t(0.001), []() {});

    float s1 = 1.0f;
    sim.register_signal("s1", [&]() { return s1; });
    sim.initialize();
    
    s1 = 1.1f; sim.simulate_steps(1);
    s1 = 1.2f; sim.simulate_steps(1);

    std::filesystem::path csv_path = temp_dir / "test_data.csv";
    export_to_csv(sim, csv_path);

    ASSERT_TRUE(std::filesystem::exists(csv_path));

    std::ifstream ifs(csv_path);
    std::string line;
    
    // Header
    std::getline(ifs, line);
    EXPECT_EQ(line, "time,s1");
    
    // Row 1: time=0, s1=1.1
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("0") != std::string::npos);
    EXPECT_TRUE(line.find("1.1") != std::string::npos);

    // Row 2: time=0.001, s1=1.2
    std::getline(ifs, line);
    EXPECT_TRUE(line.find("0.001") != std::string::npos);
    EXPECT_TRUE(line.find("1.2") != std::string::npos);
}

TEST_F(DataExporterTest, ExportToBinaryGTest) {
    simulator sim;
    sim.register_lambda(duration_t(0.001), []() {});
    float s1 = 1.0f;
    sim.register_signal("s1", [&]() { return s1; });
    sim.initialize();
    
    s1 = 1.1f; sim.simulate_steps(1);
    
    // Use the gtest helper
    export_to_binary_gtest(sim, temp_dir);
    
    std::string expected_filename = "DataExporterTest_ExportToBinaryGTest.bin";
    std::filesystem::path expected_path = temp_dir / expected_filename;
    
    EXPECT_TRUE(std::filesystem::exists(expected_path));
    
    // Verify it's a valid pitaya binary
    std::ifstream ifs(expected_path, std::ios::binary);
    char magic[4];
    ifs.read(magic, 4);
    EXPECT_EQ(magic[0], 'P'); EXPECT_EQ(magic[1], 'T'); EXPECT_EQ(magic[2], 'Y'); EXPECT_EQ(magic[3], 'A');
}

TEST_F(DataExporterTest, GTestExporterRAII) {
    std::string expected_filename = "DataExporterTest_GTestExporterRAII.bin";
    std::filesystem::path expected_path = temp_dir / expected_filename;

    {
        simulator sim;
        sim.register_lambda(duration_t(0.001), []() {});
        sim.register_signal("s1", []() { return 1.0f; });
        sim.initialize();
        
        // Instantiate the RAII exporter
        gtest_exporter exporter(sim, temp_dir);
        
        sim.simulate_steps(1);
        
        // At the end of this block, the file should be exported
    }
    
    EXPECT_TRUE(std::filesystem::exists(expected_path));
}

} // namespace pitaya
