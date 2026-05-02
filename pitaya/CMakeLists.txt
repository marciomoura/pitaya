project(pitaya CXX)

add_library(${PROJECT_NAME})
add_library(pitaya::pitaya ALIAS ${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE
    src/elapsed_timer.cpp
    src/interval_timer.cpp
    src/boolean_debouncer.cpp
    src/edge_detector.cpp
    src/on_delay.cpp
    src/off_delay.cpp
    src/on_off_delay.cpp
    src/hysteresis_limiter.cpp
    src/rate_of_change_limiter.cpp
    src/derivative.cpp
    src/hardware_filter_compensator.cpp
    src/sogi_filter.cpp
    src/sogi_filter_sequence_extractor.cpp
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
    FILES 
        include/pitaya/pitaya.hpp
        include/pitaya/types.hpp
        include/pitaya/elapsed_timer.hpp
        include/pitaya/interval_timer.hpp
        include/pitaya/lookup_table_1d.hpp
        include/pitaya/lookup_table_2d.hpp
        include/pitaya/boolean_debouncer.hpp
        include/pitaya/edge_detector.hpp
        include/pitaya/on_delay.hpp
        include/pitaya/off_delay.hpp
        include/pitaya/on_off_delay.hpp
        include/pitaya/state_machine.hpp
        include/pitaya/limit.hpp
        include/pitaya/hysteresis_limiter.hpp
        include/pitaya/linear_ramp.hpp
        include/pitaya/rate_of_change_limiter.hpp
        include/pitaya/threshold_protection.hpp
        include/pitaya/derivative.hpp
        include/pitaya/integrator.hpp
        include/pitaya/range_kutta_derivative.hpp
        include/pitaya/first_order_low_pass_filter.hpp
        include/pitaya/second_order_filter.hpp
        include/pitaya/hardware_filter_compensator.hpp
        include/pitaya/pi_controller.hpp
        include/pitaya/adaptive_band_reject_filter.hpp
        include/pitaya/sogi_filter.hpp
        include/pitaya/sogi_filter_sequence_extractor.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC mojito::mojito)

target_include_directories(${PROJECT_NAME} PUBLIC include)
