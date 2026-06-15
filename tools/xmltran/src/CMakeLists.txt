aux_source_directory(. XMLTRAN_SRC)
add_executable(xmltran ${XMLTRAN_SRC})

target_include_directories(xmltran PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(xmltran PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(xmltran PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(xmltran common)

install(TARGETS xmltran DESTINATION tools/xmltran)
