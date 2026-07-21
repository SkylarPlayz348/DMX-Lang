project(dmxc) # No Version as this is all logic for the main language

set(SOURCES
    dmx-interpreter.c
    midi-converter.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(dmxc vendor)