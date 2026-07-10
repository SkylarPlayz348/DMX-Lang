project(${PROJECT_NAME})

if(NOT EMSCRIPTEN)
    add_subdirectory(compiler)
    add_subdirectory(visualizer)
else()
    add_subdirectory(visualizer-web)
endif()