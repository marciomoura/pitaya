add_executable(pitaya_tests 
    pitaya_test.cpp
    elapsed_timer_test.cpp
    interval_timer_test.cpp
    lookup_table_1d_test.cpp
    lookup_table_2d_test.cpp
    boolean_debouncer_test.cpp
    edge_detector_test.cpp
    on_delay_test.cpp
    off_delay_test.cpp
    on_off_delay_test.cpp
)
target_link_libraries(pitaya_tests PRIVATE pitaya::pitaya GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(pitaya_tests)
