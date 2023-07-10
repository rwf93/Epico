cmake_minimum_required(VERSION 3.25)

# quick utils for getting dirs
set(THIRDPARTY ${CMAKE_SOURCE_DIR}/thirdparty)
set(SOURCE ${CMAKE_SOURCE_DIR}/source)
set(ASSETS ${CMAKE_SOURCE_DIR}/assets)
set(OUTPUT ${CMAKE_SOURCE_DIR}/output)

set(CMAKE_TOOLCHAIN_FILE thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

set(PROJECT epico)
project(${PROJECT})

add_subdirectory(source)
add_subdirectory(assets)