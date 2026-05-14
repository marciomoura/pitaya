add_executable(
    pitaya_tests
    elapsed_timer_test.cpp
    interval_timer_test.cpp
    lookup_table_1d_test.cpp
    lookup_table_2d_test.cpp
    boolean_debouncer_test.cpp
    edge_detector_test.cpp
    on_delay_test.cpp
    off_delay_test.cpp
    on_off_delay_test.cpp
    limit_test.cpp
    hysteresis_limiter_test.cpp
    linear_ramp_test.cpp
    rate_of_change_limiter_test.cpp
    threshold_protection_test.cpp
    derivative_test.cpp
    integrator_test.cpp
    first_order_low_pass_filter_test.cpp
    second_order_filter_test.cpp
    hardware_filter_compensator_test.cpp
    pi_controller_test.cpp
    sogi_filter_test.cpp
    sogi_filter_sequence_extractor_test.cpp
    srf_pll_test.cpp
    three_phase_pll_test.cpp
    dual_sogi_srf_pll_test.cpp
)

if(PITAYA_ENABLE_SIMULATION)
    target_sources(
        pitaya_tests
        PRIVATE simulation/simulation_test.cpp
                simulation/simulation_task_test.cpp
                simulation/data_logger_test.cpp
                simulation/task_scheduler_test.cpp
                simulation/data_exporter_test.cpp
                simulation/simulation_report_demo.cpp
                simulation/first_order_low_pass_filter_sim_test.cpp
                simulation/second_order_filter_sim_test.cpp
                simulation/srf_pll_sim_test.cpp
                simulation/pi_controller_sim_test.cpp
                simulation/dual_sogi_srf_pll_sim_test.cpp
                simulation/threshold_protection_sim_test.cpp
                simulation/timing_logic_sim_test.cpp
    )

    target_link_libraries(pitaya_tests PRIVATE pitaya::simulation)

    # Target to run tests and generate reports in one go
    find_package(Python3 REQUIRED)
    add_custom_target(
        run_and_report
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pitaya/tools/generate_report.py --executable
                $<TARGET_FILE:pitaya_tests> --output-dir ${CMAKE_BINARY_DIR}/reports --headless
        DEPENDS pitaya_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests and generating interactive Plotly reports..."
    )

    # Target to generate reports from ALREADY EXECUTED tests
    add_custom_target(
        generate_report
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pitaya/tools/generate_report.py --input-dir
                ${CMAKE_BINARY_DIR} --output-dir ${CMAKE_BINARY_DIR}/reports --headless
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating reports from existing .csv files..."
    )
endif()

target_link_libraries(pitaya_tests PRIVATE pitaya::pitaya GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(pitaya_tests)
