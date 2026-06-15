aux_source_directory(. WTLDECODER_SRC)
add_executable(wtldecoder ${WTLDECODER_SRC})

target_include_directories(wtldecoder PRIVATE ${MIR2X_COMMON_SOURCE_DIR} )
target_include_directories(wtldecoder PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(wtldecoder PRIVATE ${CMAKE_CURRENT_LIST_DIR}  )

target_link_libraries(wtldecoder common          )
