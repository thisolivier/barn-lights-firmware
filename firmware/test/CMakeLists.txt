cmake_minimum_required(VERSION 3.16)
project(stair-lights-tests C)

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(unity)

add_compile_definitions(UNIT_TEST)

# Test: udp_comms init/validation
add_executable(test_udp_comms
    test_udp_comms.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/udp_comms/udp_comms.c
)
target_include_directories(test_udp_comms PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/udp_comms/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../main
)
target_link_libraries(test_udp_comms PRIVATE unity)

# Test: protocol pack/parse
add_executable(test_protocol
    test_protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../main/lighting.c
)
target_include_directories(test_protocol PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../main
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/udp_comms/include
)
target_link_libraries(test_protocol PRIVATE unity)
