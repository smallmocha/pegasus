cmake_minimum_required(VERSION 3.0.0)
project(pegasus VERSION 0.1.0)

# 设置可自行文件的生成目录，cmake在/build目录执行，可执行文件生成到上一级根目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

add_compile_options(
 -g
 -std=c++14
 -Wall
 -Wextra
 -Werror
)

add_subdirectory(buffer)
add_subdirectory(http)
add_subdirectory(server)
add_subdirectory(timer)
add_subdirectory(log)

aux_source_directory(. DIR_ROOT)
add_executable(pegasus ${DIR_ROOT})
target_link_libraries(pegasus buffer http timer log server pthread)
