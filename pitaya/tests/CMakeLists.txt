add_executable(pitaya_tests pitaya_test.cpp)
target_link_libraries(pitaya_tests PRIVATE pitaya::pitaya GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(pitaya_tests)
