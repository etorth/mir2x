set(FLTK_HPP_SRC "")
set(FLTK_CPP_SRC "")
fltk_parse_fl_file(FLTK_HPP_SRC FLTK_CPP_SRC)

aux_source_directory(. PKGVIEWER_SRC)
add_executable(pkgviewer ${PKGVIEWER_SRC} ${FLTK_CPP_SRC})

target_include_directories(pkgviewer PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(pkgviewer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(pkgviewer PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pkgviewer common             )

install(TARGETS pkgviewer DESTINATION tools/pkgviewer)
