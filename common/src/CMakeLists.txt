aux_source_directory(. COMMON_SRC)
add_library(common ${COMMON_SRC})

target_include_directories(common PUBLIC ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(common PUBLIC ${CMAKE_BINARY_DIR}/config_file)
target_link_libraries(common PUBLIC mir2x::common_deps)
