aux_source_directory(. NPCGEN)
add_executable(npcgen ${NPCGEN})

target_include_directories(npcgen PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(npcgen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(npcgen PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(npcgen common)
