add_library(pitaya_simulation STATIC)
add_library(pitaya::simulation ALIAS pitaya_simulation)

target_sources(
    pitaya_simulation
    PRIVATE ${PROJECT_SOURCE_DIR}/src/simulation/data_logger.cpp
            ${PROJECT_SOURCE_DIR}/src/simulation/simulation_task.cpp
            ${PROJECT_SOURCE_DIR}/src/simulation/simulator.cpp
            ${PROJECT_SOURCE_DIR}/src/simulation/task_scheduler.cpp
            ${PROJECT_SOURCE_DIR}/src/simulation/data_exporter.cpp
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
    FILES data_logger.hpp
          data_logger_utils.hpp
          simulation_task.hpp
          simulator.hpp
          task_scheduler.hpp
          data_exporter.hpp
          gtest_data_exporter.hpp
          assertion.hpp
          gtest_simulator.hpp
)

# Conditional GTest support
if(TARGET gtest::gtest
   OR TARGET GTest::gtest
   OR TARGET gtest
)
    target_sources(pitaya_simulation PRIVATE ${PROJECT_SOURCE_DIR}/src/simulation/gtest_data_exporter.cpp)
    target_compile_definitions(pitaya_simulation PUBLIC PITAYA_HAS_GTEST)

    if(TARGET GTest::gtest)
        target_link_libraries(pitaya_simulation PRIVATE GTest::gtest)
    elseif(TARGET gtest::gtest)
        target_link_libraries(pitaya_simulation PRIVATE gtest::gtest)
    else()
        target_link_libraries(pitaya_simulation PRIVATE gtest)
    endif()
endif()

target_include_directories(
    pitaya_simulation
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/simulation"
)

target_link_libraries(pitaya_simulation PUBLIC pitaya::pitaya)
