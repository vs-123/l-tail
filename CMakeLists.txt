cmake_minimum_required(VERSION 3.10)

project(l-tail)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRCS "src/main.c")

add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
