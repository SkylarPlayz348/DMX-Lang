project(${CMAKE_PROJECT_NAME})

if(EMSCRIPTEN)
    # Every object linked into a threaded Emscripten build must be
    # compiled with matching atomics/pthread flags, so this has to be
    # set before SDL3/midi are configured, not just on the final
    # executable target.
    add_compile_options(-pthread)
    add_link_options(-pthread)
endif()

add_subdirectory(midi)

set(SDL_AUDIO OFF)
set(SDL_CAMERA OFF)
set(SDL_HAPTIC OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)

add_subdirectory(SDL)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE SDL3::SDL3 midi)
target_compile_options(vendor INTERFACE -O3)