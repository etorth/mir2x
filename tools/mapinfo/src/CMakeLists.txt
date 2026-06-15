aux_source_directory(. MAPINFO_SRC)
add_executable(mapinfo ${MAPINFO_SRC})

target_include_directories(mapinfo PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(mapinfo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(mapinfo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(mapinfo common)
