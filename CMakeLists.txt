cmake_minimum_required(VERSION 3.8)
project(Bachelor)

add_subdirectory(visualizer)

set (CMAKE_CXX_STANDARD 14)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/boost/include")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/opt/boost/lib")

FIND_PACKAGE( Boost 1.65 )
IF (Boost_FOUND)
        INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
        ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

add_executable(Bachelor main.cpp utility.hpp)
target_link_libraries(Bachelor visualizer ${LINK_LIBRARIES})

add_custom_command(
    TARGET Bachelor
    POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:Bachelor>/assets)
