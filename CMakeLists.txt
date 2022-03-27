cmake_minimum_required(VERSION 3.0.0)
project(pegasus VERSION 0.1.0)

add_compile_options(
 -g
 -std=c++11
 -Wall
 -Wextra
 -Werror
)

add_subdirectory(buffer)
add_subdirectory(http)
add_subdirectory(server)

aux_source_directory(. DIR_ROOT)
add_executable(pegasus ${DIR_ROOT})
target_link_libraries(pegasus buffer http server pthread)
