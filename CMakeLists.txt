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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(curl_coro
    src/main.cpp
    src/web_client.h
    src/web_client.cpp
    src/async.h
)
target_link_libraries(curl_coro -lcurl )
