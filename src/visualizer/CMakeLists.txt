project(dmx-lang-visualizer VERSION 0.0.5)

set(SOURCES
    main.c
    ../compiler/dmx-interpreter.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} vendor)