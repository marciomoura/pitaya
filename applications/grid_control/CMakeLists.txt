project(grid_control CXX)

add_library(${PROJECT_NAME})
add_library(applications::grid_control ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE src/dummy.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC pitaya::pitaya mojito::mojito)

if(PITAYA_BUILD_TESTING)
    add_executable(grid_control_tests tests/dummy_test.cpp)
    target_link_libraries(grid_control_tests PRIVATE ${PROJECT_NAME} pitaya::simulation GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(grid_control_tests)
endif()
