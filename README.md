# 🐉 Pitaya

---

**Pitaya** is designed to bridge the gap between abstract control theory and real-world embedded implementation. It provides a suite of components for basic signal processing and controls for power electronic applications.

Pitaya uses [🍸 Mojito](https://github.com/marciomoura/mojito) as a dependency, leveraging the usage of strong-types, physical units and coordinate transformations whenever possible.

---

## 🚀 Integration

### 1. Include in your Project (CMake)
The recommended way to use Pitaya is via `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(
    pitaya
    GIT_REPOSITORY https://github.com/marciomoura/pitaya.git
    GIT_TAG main
)
FetchContent_MakeAvailable(pitaya)

target_link_libraries(your_project PRIVATE pitaya::pitaya)
```

### 2. Basic Usage Example
```cpp
#include <pitaya/linear_ramp.hpp>
#include <iostream>

int main() {
    using namespace mojito::units::literals;
    
    pitaya::linear_ramp<float> ramp(0.01_s);
    ramp.configure({ .initial = 0.0f, .final = 100.0f, .duration = 1.0_s });
    
    for(int i = 0; i < 50; ++i) {
        std::cout << "Ramp value: " << ramp.update(true) << "\n";
    }
    
    return 0;
}
```

---

## 📊 Simulation and Reporting

Pitaya includes a lightweight simulation orchestrator and a reporting generation script. You can log signals during a test and automatically generate interactive HTML plots and CSV spreadsheets.

### 1. Logging in Tests
Use the `simulator` and `gtest_exporter` to record and export your data automatically when the test finishes:

```cpp
#include <pitaya/simulation/simulator.hpp>
#include <pitaya/simulation/gtest_data_exporter.hpp>

TEST(MySimulation, TestRun) {
    pitaya::simulator sim;
    
    // 1. Set a base simulation rate via a task
    sim.register_lambda(pitaya::duration_t(100e-6), []() { /* task logic */ });
    
    // 2. Register signals to log
    sim.register_signal("my_signal", [&]() { return 42.0f; });
    
    sim.initialize();
    
    // 3. Instantiate the exporter (saves data to .csv)
    pitaya::gtest_exporter exporter(sim);
    
    // 4. Run the simulation
    sim.simulate_for(pitaya::duration_t(0.1)); // 100ms
}
```

### 2. Generating Reports
After running your tests, you can generate the interactive reports using the built-in CMake targets. The artifacts will be collected in the `reports/` folder of your build directory.

```powershell
# 1. Run your specific test to generate the raw data
ctest --preset host-clang-test -R MySimulation.TestRun

# 2. Generate the HTML report from the existing data
cmake --build build-host --target generate_report
```

You can then find your interactive HTML report, along with the raw `.csv` file, under `build-host/reports/`.

---

## 🛠️ Development

### Prerequisites
- **CMake:** 3.28+
- **Compiler:** C++20 compatible (GCC 13+, Clang 16+, or MSVC 19.34+)
- **Dependency:** [Mojito](https://github.com/marciomoura/mojito) (Fetched automatically by CMake)

### Build and Test
```powershell
# 1. Configure using a preset
cmake --preset host-clang

# 2. Build the library and tests
cmake --build --preset host-clang-debug

# 3. Run the test suite
ctest --preset host-clang-test
```

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
