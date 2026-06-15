aux_source_directory(. MAPCONVERTER)
add_executable(mapconverter ${MAPCONVERTER})

target_include_directories(mapconverter PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(mapconverter PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(mapconverter PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(mapconverter common)
