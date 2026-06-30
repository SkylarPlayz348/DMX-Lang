project(dmx-lang-visualizer)

set(SOURCES
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} vendor)