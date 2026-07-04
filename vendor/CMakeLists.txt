project(midi)

set(SOURCES
    midi/src/midi_helper.c
    midi/src/midi.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC midi/src)

project(${CMAKE_PROJECT_NAME})

set(BUILD_SHARED_LIBS OFF)

set(SDL_AUDIO OFF)
set(SDL_CAMERA OFF)
set(SDL_HAPTIC OFF)
set(SDL_JOYSTICK OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)

add_subdirectory(SDL)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE SDL3::SDL3 midi)