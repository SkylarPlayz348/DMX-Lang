project(dmx-lang-visualizer VERSION 0.0.1)

set(SOURCES
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} vendor)