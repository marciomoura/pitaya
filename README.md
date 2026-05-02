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
