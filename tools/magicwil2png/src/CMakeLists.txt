aux_source_directory(. PNGMKER_SRC)

add_executable(magicwil2png ${PNGMKER_SRC})

target_include_directories(magicwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(magicwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(magicwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(magicwil2png common             )
