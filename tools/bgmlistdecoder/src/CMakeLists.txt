aux_source_directory(. BGMLISTDECODER_SRC)
add_executable(bgmlistdecoder ${BGMLISTDECODER_SRC})

target_include_directories(bgmlistdecoder PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(bgmlistdecoder PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bgmlistdecoder PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(bgmlistdecoder common)

install(TARGETS bgmlistdecoder DESTINATION tools/bgmlistdecoder)
