# followuidmagiceditor, the DBCOM python module of main.py
# it's built only if nanobind is installed for MIR2X_PYTHON_EXECUTABLE, see ../README.md

execute_process(
    COMMAND "${MIR2X_PYTHON_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_VARIABLE MIR2X_NANOBIND_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MIR2X_NANOBIND_RESULT
    ERROR_QUIET)

if(NOT MIR2X_NANOBIND_RESULT EQUAL 0)
    message(STATUS "Skip followuidmagiceditor: no nanobind for ${MIR2X_PYTHON_EXECUTABLE}")
    return()
endif()

set(Python_EXECUTABLE "${MIR2X_PYTHON_EXECUTABLE}")
find_package(Python COMPONENTS Interpreter Development.Module)

if(NOT Python_FOUND)
    message(STATUS "Skip followuidmagiceditor: no python development headers for ${MIR2X_PYTHON_EXECUTABLE}")
    return()
endif()

set(nanobind_DIR "${MIR2X_NANOBIND_CMAKE_DIR}")
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(followuidmagiceditor NB_STATIC followuidmagiceditormodule.cpp)

target_include_directories(followuidmagiceditor PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(followuidmagiceditor PRIVATE ${CMAKE_BINARY_DIR}/config_file)
target_link_libraries(followuidmagiceditor PRIVATE mir2x::project_options mir2x::header_deps)

# installed as a self-contained dir
# main.py finds the module and gfxres2apng.py next to itself

install(TARGETS followuidmagiceditor LIBRARY DESTINATION tools/followuidmagiceditor)
install(PROGRAMS main.py DESTINATION tools/followuidmagiceditor)
install(FILES magiccanvas.py magicdir.py ${CMAKE_CURRENT_SOURCE_DIR}/../../gfxres2apng/src/gfxres2apng.py DESTINATION tools/followuidmagiceditor)
