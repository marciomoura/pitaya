project(pitaya CXX)

add_library(${PROJECT_NAME} INTERFACE)
add_library(pitaya::pitaya ALIAS ${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    INTERFACE 
    FILE_SET HEADERS
    BASE_DIRS include
    FILES 
        include/pitaya/pitaya.hpp)

target_link_libraries(${PROJECT_NAME} INTERFACE)
