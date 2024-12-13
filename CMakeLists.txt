cmake_minimum_required(VERSION 3.28)
project(backend_news_server VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options(/W0)
endif()

include_directories(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/external
)

file(
        GLOB_RECURSE
        SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

file(
        GLOB
        NEWS_RESOURCE
        ${CMAKE_CURRENT_LIST_DIR}/resource/output.json
)

file(
        GLOB
        CONFIG_FILES
        ${CMAKE_CURRENT_LIST_DIR}/resource/config_name_change.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/default_config.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/static.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/global_setting.json
)

file(
        COPY
        ${CONFIG_FILES}
        ${NEWS_RESOURCE}
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/version.h
)

add_executable(
        backend_news_server
        main.cpp
        ${SOURCES}
)
