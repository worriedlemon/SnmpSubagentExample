cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")

project(snmp_test)

set(TOPDIR ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_MODULE_PATH "${TOPDIR}/cmake; ${CMAKE_MODULEPATH}")

message(STATUS "CMake include paths: ${CMAKE_MODULE_PATH}")

include(CMakeConf)

add_compile_options(-std=c++2a)    # standart
add_compile_options(-Wall -Wextra) # warnings

message(STATUS "Used standart: C++${CMAKE_CXX_STANDART}")

include_directories(AFTER
    include
)

add_executable(snmp_test
    src/main.cpp
    src/snmp_agent.cpp
    src/snmp_handlers.cpp
    src/process_info.cpp
)

target_link_libraries(snmp_test
    pthread
    snmp
    netsnmp
    netsnmpagent
)

install(TARGETS snmp_test DESTINATION "${CMAKE_SOURCE_DIR}/out/")
