# CMakeLists.txt for orchestrating all applications examples

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/grid_control/CMakeLists.txt")
    add_subdirectory(grid_control)
endif()
