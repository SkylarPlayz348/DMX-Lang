project(midi-research)

set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "" FORCE)


set(SOURCES
    write-midi.c
)

add_executable(write-midi ${SOURCES})
target_link_libraries(write-midi PRIVATE vendor)