cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
add_compile_options(
    -g
    -Wall
    -Wextra
    -Wpedantic
    -stdlib=libc++
    -Wno-unused-parameter
)
add_link_options(-stdlib=libc++)

project (curl_coro)

include_directories("poller")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_library(poller
    poller/poller.h
    poller/poller.cpp
    poller/async.h
    poller/handle.cpp
    poller/handle.h
)

add_subdirectory(test/callback)
target_link_libraries(callback poller curl)

add_subdirectory(test/coro)
target_link_libraries(coro poller curl)
