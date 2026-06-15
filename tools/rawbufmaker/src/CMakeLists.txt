aux_source_directory(. RAWBUFMAKER_SRC)
add_executable(rawbufmaker ${RAWBUFMAKER_SRC})

target_include_directories(rawbufmaker PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(rawbufmaker PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(rawbufmaker PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(rawbufmaker common)

install(TARGETS rawbufmaker DESTINATION tools/rawbufmaker)
