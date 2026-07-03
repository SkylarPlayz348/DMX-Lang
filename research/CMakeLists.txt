project(midi-research)

set(SOURCES
    write-midi.c
)

add_executable(write-midi ${SOURCES})
target_link_libraries(write-midi PRIVATE vendor)