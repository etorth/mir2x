aux_source_directory(. MONSTERWIL2PNG_SRC)
add_executable(monsterwil2png ${MONSTERWIL2PNG_SRC})

target_include_directories(monsterwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(monsterwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(monsterwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(monsterwil2png common             )

install(TARGETS monsterwil2png DESTINATION tools/monsterwil2png)
install(PROGRAMS monsterwil2png.sh DESTINATION tools/monsterwil2png)
