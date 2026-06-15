aux_source_directory(. MONSTERGEN)
add_executable(monstergen ${MONSTERGEN})

target_include_directories(monstergen PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(monstergen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(monstergen PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(monstergen common)
