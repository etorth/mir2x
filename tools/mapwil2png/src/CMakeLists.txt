aux_source_directory(. MAPWIL2PNG_SRC)
add_executable(mapwil2png ${MAPWIL2PNG_SRC})

target_include_directories(mapwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(mapwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(mapwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(mapwil2png common          )
