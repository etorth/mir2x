aux_source_directory(. HAIRWIL2PNG_SRC)
add_executable(hairwil2png ${HAIRWIL2PNG_SRC})

target_include_directories(hairwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(hairwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(hairwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(hairwil2png common          )
