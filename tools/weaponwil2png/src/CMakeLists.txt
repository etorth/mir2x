set(FLTK_HPP_SRC "")
set(FLTK_CPP_SRC "")
fltk_parse_fl_file(FLTK_HPP_SRC FLTK_CPP_SRC)

aux_source_directory(. SHADOWMAKER_SRC)
add_executable(weaponwil2png ${SHADOWMAKER_SRC} ${FLTK_CPP_SRC})

target_include_directories(weaponwil2png PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(weaponwil2png PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(weaponwil2png PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(weaponwil2png common             )
