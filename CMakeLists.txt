cmake_minimum_required(VERSION 3.0)
project(live_danmaku_hime)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -mwindows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /GL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

aux_source_directory(src SRC_SOURCES)
add_executable(live_danmaku_hime ${SRC_SOURCES})

set_target_properties(live_danmaku_hime PROPERTIES
    CXX_STANDARD 11
    POSITION_INDEPENDENT_CODE ON
)
