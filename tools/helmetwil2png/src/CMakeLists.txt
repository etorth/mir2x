aux_source_directory(. HELMETWIL2PNG_SRC)
add_executable(helmetwil2png ${HELMETWIL2PNG_SRC})

target_include_directories(helmetwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR} )
target_include_directories(helmetwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(helmetwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR}  )

target_link_libraries(helmetwil2png common          )
