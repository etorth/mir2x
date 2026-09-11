# This file defines the mir2x test infrastructure.
#
# A module keeps human-maintained code in src/ and generated tests in test/.
#
# Unit tests are standalone executables, and integration tests run external commands.
# Test executables use EXCLUDE_FROM_ALL so normal builds and installs do not build them.
#
# Register tests with one of these functions.
#
#     mir2x_add_unit_test()
#     mir2x_add_integration_test()
#
# Call these functions from a module's test/unit or test/integration CMakeLists.txt.
# Each call registers a CTest test that runs through cmake/run_test.py.
#
# Call mir2x_finalize_tests() once at the end of the root CMakeLists.txt.
# It defines the check target, which builds every registered test and runs CTest.
#
#   cmake --build <builddir> --target check           # build + run every test
#   ctest --test-dir <builddir> --output-on-failure   # rerun tests after building
#   ctest --test-dir <builddir> -R xmltypeset         # rerun a single test by name
#
# Makefile targets:
#
#   make test   # run CTest directly
#   make check  # build and run CTest
#
# Regenerate a gold file with this command:
#
#   ${MIR2X_PYTHON_EXECUTABLE} cmake/run_test.py --update-gold --gold <path> -- <built-test-exe> [args...]
#
enable_testing()

define_property(GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS
    BRIEF_DOCS "targets that must be built before running ctest"
    FULL_DOCS  "targets that must be built before running ctest")

# mir2x_add_unit_test(NAME <name>
#                      SOURCES <src...>
#                      [LIBS <lib...>]
#                      [DEPENDS <target...>]
#                      [ARGS <args...>]
#                      [GOLD <path>]
#                      [NO_GOLD]
#                      [WORKING_DIRECTORY <dir>])
#
# This function builds an EXCLUDE_FROM_ALL executable named test_<name> from SOURCES and registers it as a CTest test named <name>.
#
# GOLD
#    Defaults to "<first-source-without-extension>.log.gold" next to the first source file.
#    Specify NO_GOLD to skip gold-file comparison.
#
# WORKING_DIRECTORY
#    Defaults to the directory containing the calling CMakeLists.txt.
#
function(mir2x_add_unit_test)
    set(T_OPTIONS NO_GOLD)
    set(T_ONE_VALUE_ARGS NAME GOLD WORKING_DIRECTORY)
    set(T_MULTI_VALUE_ARGS SOURCES LIBS DEPENDS ARGS)
    cmake_parse_arguments(T "${T_OPTIONS}" "${T_ONE_VALUE_ARGS}" "${T_MULTI_VALUE_ARGS}" ${ARGN})

    if(NOT T_NAME)
        message(FATAL_ERROR "mir2x_add_unit_test: NAME is required")
    endif()
    if(NOT T_SOURCES)
        message(FATAL_ERROR "mir2x_add_unit_test: SOURCES is required")
    endif()

    set(T_TARGET "test_${T_NAME}")
    add_executable(${T_TARGET} EXCLUDE_FROM_ALL ${T_SOURCES})

    if(T_LIBS)
        target_link_libraries(${T_TARGET} PRIVATE ${T_LIBS})
    endif()
    if(T_DEPENDS)
        add_dependencies(${T_TARGET} ${T_DEPENDS})
    endif()

    if(NOT T_GOLD AND NOT T_NO_GOLD)
        list(GET T_SOURCES 0 T_FIRST_SRC)
        if(NOT IS_ABSOLUTE ${T_FIRST_SRC})
            set(T_FIRST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${T_FIRST_SRC})
        endif()
        get_filename_component(T_FIRST_SRC_WE  ${T_FIRST_SRC} NAME_WE)
        get_filename_component(T_FIRST_SRC_DIR ${T_FIRST_SRC} DIRECTORY)
        set(T_GOLD "${T_FIRST_SRC_DIR}/${T_FIRST_SRC_WE}.log.gold")
    endif()

    if(NOT T_WORKING_DIRECTORY)
        set(T_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    mir2x__add_test(${T_NAME} ${T_TARGET} "$<TARGET_FILE:${T_TARGET}>;${T_ARGS}" "${T_WORKING_DIRECTORY}" "${T_GOLD}")
endfunction()

# mir2x_add_integration_test(NAME <name>
#                             COMMAND <cmd...>
#                             [DEPENDS <target...>]
#                             [GOLD <path>]
#                             [WORKING_DIRECTORY <dir>])
#
# This function registers an arbitrary external command, such as `${MIR2X_PYTHON_EXECUTABLE} test_client.py`, as a CTest test named <name>.
# It does not compile anything itself.
#
# COMMAND
#    If the first word of COMMAND names an existing CMake target, it is resolved to that target's built file and added as a build dependency.
#
# DEPENDS
#    Lists additional targets that must be built first.
#
function(mir2x_add_integration_test)
    set(T_ONE_VALUE_ARGS NAME GOLD WORKING_DIRECTORY)
    set(T_MULTI_VALUE_ARGS COMMAND DEPENDS)
    cmake_parse_arguments(T "" "${T_ONE_VALUE_ARGS}" "${T_MULTI_VALUE_ARGS}" ${ARGN})

    if(NOT T_NAME)
        message(FATAL_ERROR "mir2x_add_integration_test: NAME is required")
    endif()
    if(NOT T_COMMAND)
        message(FATAL_ERROR "mir2x_add_integration_test: COMMAND is required")
    endif()

    list(GET T_COMMAND 0 T_CMD0)
    if(TARGET ${T_CMD0})
        list(APPEND T_DEPENDS ${T_CMD0})
        list(REMOVE_AT T_COMMAND 0)
        list(PREPEND T_COMMAND "$<TARGET_FILE:${T_CMD0}>")
    endif()

    if(NOT T_WORKING_DIRECTORY)
        set(T_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Integration tests do not build an executable, so a custom target gives check a dependency to build.
    set(T_BUILD_TARGET "test_${T_NAME}_integration")
    add_custom_target(${T_BUILD_TARGET})
    if(T_DEPENDS)
        add_dependencies(${T_BUILD_TARGET} ${T_DEPENDS})
    endif()

    mir2x__add_test(${T_NAME} ${T_BUILD_TARGET} "${T_COMMAND}" "${T_WORKING_DIRECTORY}" "${T_GOLD}")
endfunction()

# This shared helper adds the build dependency, wraps the command in cmake/run_test.py, and registers the CTest test.
function(mir2x__add_test T_NAME T_BUILD_TARGET T_COMMAND T_WORKDIR T_GOLD)
    set_property(GLOBAL APPEND PROPERTY MIR2X_TEST_BUILD_TARGETS ${T_BUILD_TARGET})

    set(T_WRAPPER_ARGS --name "${T_NAME}")
    if(T_GOLD)
        list(APPEND T_WRAPPER_ARGS --gold "${T_GOLD}")
    endif()

    add_test(NAME ${T_NAME}
        COMMAND ${MIR2X_PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/run_test.py ${T_WRAPPER_ARGS} -- ${T_COMMAND}
        WORKING_DIRECTORY ${T_WORKDIR})
endfunction()

# Call this once from the root CMakeLists.txt after all test subdirectories have been added.
function(mir2x_finalize_tests)
    get_property(T_ALL_TEST_TARGETS GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS)

    add_custom_target(mir2x_build_tests)
    if(T_ALL_TEST_TARGETS)
        add_dependencies(mir2x_build_tests ${T_ALL_TEST_TARGETS})
    endif()

    # This convenience target builds every registered test and then runs CTest.
    add_custom_target(check
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target mir2x_build_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM)
endfunction()
