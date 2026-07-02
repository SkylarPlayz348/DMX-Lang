project(${PROJECT_NAME})


add_subdirectory(SDL)
add_subdirectory(midi)

add_library(vendor INTERFACE)
target_include_directories(vendor INTERFACE midi/src)
target_link_libraries(vendor INTERFACE SDL3::SDL3 midi)