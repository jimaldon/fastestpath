cmake_minimum_required(VERSION 3.8)
project(Bachelor)

add_subdirectory(visualizer)

set (CMAKE_CXX_STANDARD 11)

add_executable(Bachelor main.cpp astar.cpp)
target_link_libraries(Bachelor visualizer)

add_custom_command(
    TARGET Bachelor
    POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:Bachelor>/assets)
