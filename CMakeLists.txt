cmake_minimum_required(VERSION 3.28.3)
project(DMX-Lang VERSION 0.0.0)

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

add_subdirectory(src)