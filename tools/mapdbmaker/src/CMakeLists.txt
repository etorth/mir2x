aux_source_directory(. MAPDBMAKER)
add_executable(mapdbmaker ${MAPDBMAKER})

target_include_directories(mapdbmaker PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(mapdbmaker PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(mapdbmaker PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(mapdbmaker common)
