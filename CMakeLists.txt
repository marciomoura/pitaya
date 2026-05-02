cmake_minimum_required(VERSION 3.28)

project(
    pitaya
    VERSION 0.1.0
    DESCRIPTION "A modular C++ library for units and math"
    HOMEPAGE_URL "https://github.com/marciomoura/pitaya/"
    LANGUAGES CXX
)

# Output directory configuration
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies")

# Standard C++ Configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(PITAYA_BUILD_TESTING "Build the testing tree" ON)
option(PITAYA_BUILD_EXAMPLES "Build examples" ON)
option(PITAYA_ENABLE_CCACHE "Enable ccache" ON)
option(PITAYA_ENABLE_CPPCHECK "Enable static analysis with cppcheck" ON)

# Check for recommended generator
message(STATUS "Generator: ${CMAKE_GENERATOR}")
if(NOT CMAKE_GENERATOR MATCHES "Ninja" AND NOT MSVC)
    message(WARNING "Using ${CMAKE_GENERATOR} - Ninja is recommended for best performance.")
endif()

# Enable CCache if available
if(PITAYA_ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        message(STATUS "CCache enabled")
    else()
        message(WARNING "CCache not found, proceeding without it.")
    endif()
endif()

# Static analysis with cppcheck
if(PITAYA_ENABLE_CPPCHECK)
    include(cppcheck)
endif()

# Testing Setup
if(PITAYA_BUILD_TESTING)
    message(STATUS "Testing enabled")
    include(gtest)
    enable_testing()
endif()

# Core Library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pitaya/CMakeLists.txt")
    add_subdirectory(pitaya)
endif()

# Examples
if(PITAYA_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pitaya/examples")
    message(STATUS "Examples enabled")
    add_subdirectory(pitaya/examples)
endif()

# Tests
if(PITAYA_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pitaya/tests")
    message(STATUS "Tests enabled")
    add_subdirectory(pitaya/tests)
endif()

message(STATUS "Pitaya configuration complete")
