#!/bin/bash

cd $(pwd)/${BASH_SOURCE[0]%/*}

source variables.sh

BUILD_TYPE=$(echo ${BUILD_TYPE} | tr '[:lower:]' ':upper:')
case "${BUILD_TYPE}" in
    "DEBUG")
        BUILD_TYPE="Debug"
    ;;
    "RELEASE")
        BUILD_TYPE="Release"
    ;;
    *)
        BUILD_TYPE="Debug"
    ;;
esac

echo BUILD_TYPE="${BUILD_TYPE}"

mkdir -p cmake
cat << EOF > cmake/CMakeConf.cmake

## configure.sh ##
add_definitions(-DOTHER_CORE_PROCESSES="${OTHER_CORE_PROCESSES}")

set(CMAKE_BUILD_TYPE "${BUILD_TYPE}")

if( "\${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    add_definitions(-DNDEBUG)
endif()
EOF

echo CMakeConf updated at 'cmake/CMakeConf.cmake'