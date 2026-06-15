aux_source_directory(. SOUNDLISTDECODER_SRC)
add_executable(soundlistdecoder ${SOUNDLISTDECODER_SRC})

target_include_directories(soundlistdecoder PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(soundlistdecoder PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(soundlistdecoder PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(soundlistdecoder common)

install(TARGETS soundlistdecoder DESTINATION tools/soundlistdecoder)
