cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

project(snmp_test)

set(TOPDIR ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${TOPDIR}/cmake; ${CMAKE_MODULEPATH}")

message(STATUS "CMake include paths: ${CMAKE_MODULE_PATH}")

# config file from 'configure' Makefile target
include(CMakeConf)

# enable all warnings as errors
add_compile_options(-Wall -Wextra)

# explicit standart
add_compile_options(-std=c++${CMAKE_CXX_STANDART})

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

install(TARGETS snmp_test DESTINATION "${TOPDIR}/out/")
