# 🐉 Pitaya

A high-performance C++20 control systems library for modern engineering and power electronics applications.

---

**Pitaya** is designed to bridge the gap between abstract control theory and real-world embedded implementation. It provides a suite of numerically stable, type-safe, and ready-to-use components ranging from basic signal conditioning to advanced grid synchronization algorithms.

Built on top of [🍸 Mojito](https://github.com/marciomoura/mojito), Pitaya leverages advanced metaprogramming to ensure physical units and coordinate frame consistency at compile-time.

## ✨ Features

- **🛡️ Type-Safe Control:** Native support for physical quantities (Voltage, Current, Frequency) and coordinate frames (ABC, Alpha-Beta, DQ).
- **📈 Advanced Filters:** Includes first/second-order filters, adaptive notch filters, and SOGI-based sequence extractors.
- **⚡ Grid Synchronization:** Production-ready SRF-PLL and Dual-SOGI PLL implementations.
- **🧱 Building Blocks:** Robust logic handlers (debouncers, edge detectors) and math utilities (ramps, limiters, integrators).
- **🎯 Performance Oriented:** Zero-allocation algorithms, optimized for real-time control loops.
- **🧪 Fully Verified:** Comprehensive unit test suite with 180+ tests passing on every build.

---

## 🛡️ Technical Showcase

### 1. Robust PI Control
Configuring and updating a PI controller is concise and type-safe.

```cpp
#include <pitaya/pi_controller.hpp>

// Define a controller with 10ms sampling time
pitaya::pi_controller<float> speed_reg(0.01);

// Configure gains: Kp = 2.0, Ti = 100ms
speed_reg.configure_with_ti(2.0f, 0.1f);
speed_reg.set_output_limits(-1.0f, 1.0f);

// Update in the control loop
float output = speed_reg.update(error);
```

### 2. Grid Synchronization (PLL)
Extract grid frequency and phase using advanced Synchronous Reference Frame (SRF) PLLs.

```cpp
#include <pitaya/srf_pll.hpp>
#include <mojito/mojito.hpp>

using namespace mojito;

pitaya::srf_pll pll(duration_t{50e-6f}); // 50us sampling
pll.configure_nominal_frequency(frequency_t{60.0f});

// In the ADC interrupt:
void on_adc_ready(const abc<voltage_pu_t>& v_grid) {
    pll.update(to_alphabeta(v_grid));
    
    auto freq = pll.get_estimated_frequency(); // 60.01 Hz
    auto theta = pll.get_estimated_angle();    // Grid phase angle
}
```

### 3. SOGI Sequence Extraction
Filter stationary frames and extract positive sequence components even under distorted grid conditions.

```cpp
#include <pitaya/sogi_filter_sequence_extractor.hpp>

pitaya::sogi_filter_sequence_extractor extractor(ts);
extractor.configure(1.414f, duration_t{0.02f});

// Update with raw alpha-beta voltages
extractor.update(v_in_ab, grid_omega);

// Get clean positive sequence
auto v_pos = extractor.get_positive_sequence();
```

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
