aux_source_directory(. ZSDBMAKER_SRC)
add_executable(zsdbmaker ${ZSDBMAKER_SRC})

target_include_directories(zsdbmaker PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(zsdbmaker PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(zsdbmaker PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(zsdbmaker common)

install(TARGETS zsdbmaker DESTINATION tools/zsdbmaker)
