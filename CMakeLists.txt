cmake_minimum_required(VERSION 3.25)

# quick utils for getting dirs
set(THIRDPARTY ${CMAKE_SOURCE_DIR}/thirdparty)
set(SOURCE ${CMAKE_SOURCE_DIR}/source)
set(ASSETS ${CMAKE_SOURCE_DIR}/assets)
set(OUTPUT ${CMAKE_SOURCE_DIR}/output)

set(CMAKE_TOOLCHAIN_FILE ${THIRDPARTY}/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithAsserts RelWithDebInfo Release)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

# multi-configuration based builds (Visual Studio, Ninja Multi-Config) need to be handled properly
foreach(CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIGURATION} CONFIGURATION_NAME)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIGURATION_NAME} ${CMAKE_SOURCE_DIR}/output)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIGURATION_NAME} ${CMAKE_SOURCE_DIR}/output)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIGURATION_NAME} ${CMAKE_SOURCE_DIR}/output)

endforeach(CONFIGURATION)

set(PROJECT Epico)
project(${PROJECT})

add_subdirectory(source)
add_subdirectory(assets)