cmake_minimum_required(VERSION 3.28)
project(backend_news_server VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options(/W0)
endif()

link_directories(
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

include_directories(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/external
)

# Copy sql files
file(
        COPY
        resource/sql_util
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        GLOB_RECURSE
        SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

# Copy news and config files
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


# move dll to exe folder
file(
        GLOB
        DLLS
        ${CMAKE_CURRENT_LIST_DIR}/dll/*.dll
)

file(
        COPY
        ${DLLS}
        DESTINATION ${CMAKE_BINARY_DIR}
)
# image 
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/image
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

add_executable(
        backend_news_server
        main.cpp
        ${SOURCES}
)



target_link_libraries(
        backend_news_server
        libcrypto
        libssl
        mysqlcppconn8
        mysqlcppconn
)
