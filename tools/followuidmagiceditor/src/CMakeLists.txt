set(FLTK_HPP_SRC "")
set(FLTK_CPP_SRC "")
fltk_parse_fl_file(FLTK_HPP_SRC FLTK_CPP_SRC)

aux_source_directory(. FOLLOWUIDMAGICEDITOR_SRC)
add_executable(followuidmagiceditor ${FOLLOWUIDMAGICEDITOR_SRC} ${FLTK_CPP_SRC})

target_include_directories(followuidmagiceditor PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(followuidmagiceditor PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(followuidmagiceditor PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(followuidmagiceditor common)

install(TARGETS followuidmagiceditor DESTINATION tools/followuidmagiceditor)
