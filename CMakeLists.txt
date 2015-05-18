cmake_minimum_required(VERSION 3.0)
project(live_danmaku_hime)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -mwindows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /GL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

aux_source_directory(src SRC_MAIN)
aux_source_directory(src/fetcher SRC_FETCHER)
aux_source_directory(src/renderer SRC_RENDERER)
aux_source_directory(src/presenter SRC_PRESENTER)
add_executable(live_danmaku_hime ${SRC_MAIN} ${SRC_FETCHER} ${SRC_RENDERER} ${SRC_PRESENTER})

if(WIN32)
    target_include_directories(live_danmaku_hime PRIVATE "3rd-party/include")
    find_library(LIB_CAIRO cairo "3rd-party/lib")
    find_library(LIB_THIN_BUNDLE thin_bundle "3rd-party/lib")
    target_link_libraries(live_danmaku_hime PUBLIC "${LIB_CAIRO}" "${LIB_THIN_BUNDLE}" "winpthread")
endif()

set_target_properties(live_danmaku_hime PROPERTIES
    CXX_STANDARD 11
    POSITION_INDEPENDENT_CODE ON
)
