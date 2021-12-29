cmake_minimum_required(VERSION 3.21)
project(parking C)

set(CMAKE_C_STANDARD 99)

find_library(LPTHREAD lpthread /usr/lib)
find_package(Threads REQUIRED)

add_executable(parking main.c)
target_link_libraries(parking ${CMAKE_THREAD_LIBS_INIT})