project(dmxc)

set(SOURCES
    dmx-interpreter.c
    midi-converter.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(dmxc vendor)