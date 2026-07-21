cmake_minimum_required(VERSION 3.28.3)
project(DMX-Lang VERSION 0.0.10)

if(WIN32)
    set(TARGET_OS "Windows")
elseif(APPLE)
    set(TARGET_OS "macOS")
elseif(LINUX)
    set(TARGET_OS "Linux")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendor)

add_subdirectory(research)
add_subdirectory(src)