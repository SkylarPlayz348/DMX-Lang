project(dmx-lang-visualizer-web VERSION 0.0.1)

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

set(SOURCES
    main.c
    ../compiler/dmx-interpreter.c
)


add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} vendor)
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -pthread)
target_link_options(${PROJECT_NAME} PRIVATE
    --emrun
    -pthread
    -sPTHREAD_POOL_SIZE=2
    -sFORCE_FILESYSTEM=1
    -sEXPORTED_RUNTIME_METHODS=ccall
)