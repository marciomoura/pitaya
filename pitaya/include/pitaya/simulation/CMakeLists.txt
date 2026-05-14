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
)

target_include_directories(
    pitaya_simulation
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${PROJECT_SOURCE_DIR}/src/simulation"
)

target_link_libraries(pitaya_simulation PUBLIC pitaya::pitaya)
