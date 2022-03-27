aux_source_directory(. DIR_SERVER)
add_library(server ${DIR_SERVER})
target_link_libraries(server http)
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR})