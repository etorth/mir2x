#=======================================================================================
# mir2x test infrastructure
#
# Convention: for a module "foo" (server, client, tools/pkgviewer, ...):
#
#   foo/src/            100% human-maintained source code, built and installed as usual.
#   foo/test/unit/      AI-generated unit tests, one small standalone executable per test.
#   foo/test/integration AI-generated integration tests (e.g. python scripts that drive a
#                       built executable and check its behavior/output).
#
# Test code is checked in like any other source, but is NOT expected to be hand-maintained;
# it exists purely as regression coverage and can be freely regenerated/rewritten by an AI
# assistant. Test executables are EXCLUDE_FROM_ALL: `make`/`make install` never builds them.
#
# This project uses CTest (native to CMake) rather than a bespoke runner. Use
# mir2x_add_unit_test()/mir2x_add_integration_test() (below) from a module's
# test/unit/CMakeLists.txt or test/integration/CMakeLists.txt to register tests; each
# registers a real CTest test (add_test()) that goes through cmake/run_test.py, which runs
# the test's command and, if a gold file is registered, diffs its stdout against it
# (printing a unified diff on mismatch) - CTest itself only looks at the wrapper's exit
# code, so this is transparent to `ctest`/any CTest-aware tooling (IDE test explorers etc).
#
# After every module has been added (i.e. at the very end of the root CMakeLists.txt), call
# mir2x_finalize_tests() once to define the `check` convenience target: build every
# registered test, then run ctest. Typical usage:
#
#   cmake --build <builddir> --target check          # build + run every test
#   ctest --test-dir <builddir> --output-on-failure   # rerun tests only (after building)
#   ctest --test-dir <builddir> -R xmltypeset         # rerun a single test by name
#
# CMake reserves the target name "test" for CTest once enable_testing() is called (e.g.
# `make test` on the Makefiles generator runs ctest directly) - that's a plain `ctest` run
# with no dependency on building first, hence `check` as the build+run convenience target.
#
# gold files: rerun cmake/run_test.py directly with --update-gold to (re)create a test's
# gold file from its current actual output, e.g.:
#
#   python3 cmake/run_test.py --update-gold --gold <path> -- <built-test-exe> [args...]
#=======================================================================================

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
# builds an EXCLUDE_FROM_ALL executable named test_<name> from SOURCES and registers it as
# a CTest test named <name>. unless NO_GOLD is given, GOLD defaults to
# "<first-source-without-extension>.log.gold" next to the first source file (matches this
# project's convention of naming e.g. xmltypeset_test.cpp's gold file
# xmltypeset_test.log.gold) - this default applies whether or not that file exists yet, so
# a first run reports a normal (fixable) failure instead of silently skipping the check.
# WORKING_DIRECTORY defaults to the directory containing the CMakeLists.txt that calls this
# function.
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
# registers an arbitrary out-of-process command (e.g. `python3 test_client.py`) as a CTest
# test named <name>. this function doesn't compile anything itself; if COMMAND's first word
# names an existing CMake target (e.g. the `client` executable), it's resolved to that
# target's built file and the target is added as a build dependency, so integration tests
# can directly launch the real module executables they're testing. DEPENDS can list further
# targets (e.g. `server`, for a test that drives both client and server) that must also be
# built first.
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

    # integration tests don't build anything of their own (unlike mir2x_add_unit_test's
    # test_<name> executable), so a plain custom target is enough to hold their DEPENDS and
    # give `check` something to depend on
    set(T_BUILD_TARGET "test_${T_NAME}_integration")
    add_custom_target(${T_BUILD_TARGET})
    if(T_DEPENDS)
        add_dependencies(${T_BUILD_TARGET} ${T_DEPENDS})
    endif()

    mir2x__add_test(${T_NAME} ${T_BUILD_TARGET} "${T_COMMAND}" "${T_WORKING_DIRECTORY}" "${T_GOLD}")
endfunction()

# shared by mir2x_add_unit_test()/mir2x_add_integration_test(): registers <build-target> as
# something `check` must build first, then wraps <command> in cmake/run_test.py (for the
# optional gold-file diff) and registers that as a real CTest test named <name>
function(mir2x__add_test T_NAME T_BUILD_TARGET T_COMMAND T_WORKDIR T_GOLD)
    set_property(GLOBAL APPEND PROPERTY MIR2X_TEST_BUILD_TARGETS ${T_BUILD_TARGET})

    set(T_WRAPPER_ARGS --name "${T_NAME}")
    if(T_GOLD)
        list(APPEND T_WRAPPER_ARGS --gold "${T_GOLD}")
    endif()

    add_test(NAME ${T_NAME}
        COMMAND python3 ${CMAKE_SOURCE_DIR}/cmake/run_test.py ${T_WRAPPER_ARGS} -- ${T_COMMAND}
        WORKING_DIRECTORY ${T_WORKDIR})
endfunction()

# call once, from the root CMakeLists.txt, after every add_subdirectory() that might call
# mir2x_add_unit_test()/mir2x_add_integration_test() has already run
function(mir2x_finalize_tests)
    get_property(T_ALL_TEST_TARGETS GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS)

    add_custom_target(mir2x_build_tests)
    if(T_ALL_TEST_TARGETS)
        add_dependencies(mir2x_build_tests ${T_ALL_TEST_TARGETS})
    endif()

    # convenience target: build every registered test, then run ctest. `ctest`/`make test`
    # (CTest's own reserved target) only runs already-built tests, it doesn't build them.
    add_custom_target(check
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target mir2x_build_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM)
endfunction()
