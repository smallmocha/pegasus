aux_source_directory(. DIR_HTTP)
add_library(http ${DIR_HTTP})
target_link_libraries(http buffer log)
target_include_directories(http PUBLIC ${PROJECT_SOURCE_DIR})