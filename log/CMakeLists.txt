aux_source_directory(. LOG_DIR)
add_library(log ${LOG_DIR})
target_link_libraries(log buffer)
target_include_directories(log PUBLIC ${PROJECT_SOURCE_DIR})
