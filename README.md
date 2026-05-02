# 🐉 Pitaya

A minimal C++ template project for modern engineering applications.

## ✨ Features

- **Header-Only Library:** Easy to integrate into any C++ project.
- **Modern CMake:** Uses CMake Presets and modern practices.
- **Unit Testing:** Integrated with GoogleTest.
- **CI/CD:** GitHub Actions workflow included.

## 🛠️ Getting Started

### Prerequisites

- CMake 3.28+
- A C++20 compatible compiler (GCC 13+, Clang 16+, or MSVC 19.34+)

### Build and Test

```powershell
# Configure using a preset
cmake --preset host-gcc

# Build
cmake --build --preset host-gcc-debug

# Run Tests
ctest --preset host-gcc-test
```

## 📜 License

This project is licensed under the MIT License.
