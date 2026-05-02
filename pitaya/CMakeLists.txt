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
        include/pitaya/state_machine.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC mojito::mojito)

target_include_directories(${PROJECT_NAME} PUBLIC include)
