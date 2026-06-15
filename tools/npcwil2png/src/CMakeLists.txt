aux_source_directory(. NPCWIL2PNG_SRC)
add_executable(npcwil2png ${NPCWIL2PNG_SRC})

target_include_directories(npcwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(npcwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(npcwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(npcwil2png common          )
