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
# Use mir2x_add_unit_test()/mir2x_add_integration_test() (below) from a module's
# test/unit/CMakeLists.txt or test/integration/CMakeLists.txt to register tests. After every
# module has been added (i.e. at the very end of the root CMakeLists.txt), call
# mir2x_finalize_tests() once to wire up the aggregate `test` target:
#
#   make test
#
# builds every registered test (via a `mir2x_build_tests` helper target) and then runs
# cmake/run_tests.py, which executes each test, diffs its stdout against a "gold" file when
# one is registered, and prints a pass/fail summary (nonzero exit if anything failed or
# differed from its gold file).
#
# NOTE: don't call enable_testing()/CTest anywhere in this project - CMake reserves the
# target name "test" for CTest once enable_testing() is called, which would conflict with
# the `test` target defined here. This project intentionally uses its own lightweight
# runner instead of CTest, since gold-file diffing isn't something add_test() does for us
# anyway.
#=======================================================================================

define_property(GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS
    BRIEF_DOCS "targets that must be built for `make test`"
    FULL_DOCS  "targets that must be built for `make test`")

set(MIR2X_TEST_MANIFEST_DIR "${CMAKE_BINARY_DIR}/mir2x_test_manifest" CACHE INTERNAL "")

# turns a CMake list into a JSON array string literal, kept as its own helper since both
# mir2x_add_unit_test() and mir2x_add_integration_test() need it, escaping is applied
# before any generator expressions in the list are evaluated, this is safe because
# generator expression syntax itself never contains a backslash or a double quote
function(mir2x__json_array OUT_VAR)
    set(T_JSON "[")
    set(T_FIRST TRUE)
    foreach(T_ITEM ${ARGN})
        if(NOT T_FIRST)
            string(APPEND T_JSON ",")
        endif()
        set(T_FIRST FALSE)
        string(REPLACE "\\" "\\\\" T_ESCAPED "${T_ITEM}")
        string(REPLACE "\"" "\\\"" T_ESCAPED "${T_ESCAPED}")
        string(APPEND T_JSON "\"${T_ESCAPED}\"")
    endforeach()
    string(APPEND T_JSON "]")
    set(${OUT_VAR} "${T_JSON}" PARENT_SCOPE)
endfunction()

# writes ${CMAKE_BINARY_DIR}/mir2x_test_manifest/<build-target>.json, one small descriptor
# per test, and appends <build-target> to the global list of targets `make test` builds
# first. run_tests.py globs this directory at run time, so nothing here needs to track the
# full manifest itself, each test's descriptor is entirely self-contained.
function(mir2x__register_test T_BUILD_TARGET T_NAME T_COMMAND T_WORKDIR T_GOLD)
    set_property(GLOBAL APPEND PROPERTY MIR2X_TEST_BUILD_TARGETS ${T_BUILD_TARGET})

    mir2x__json_array(T_COMMAND_JSON ${T_COMMAND})
    if(T_GOLD)
        set(T_GOLD_JSON "\"${T_GOLD}\"")
    else()
        set(T_GOLD_JSON "null")
    endif()

    file(GENERATE OUTPUT "${MIR2X_TEST_MANIFEST_DIR}/${T_BUILD_TARGET}.json" CONTENT
"{
    \"name\": \"${T_NAME}\",
    \"command\": ${T_COMMAND_JSON},
    \"workdir\": \"${T_WORKDIR}\",
    \"gold\": ${T_GOLD_JSON}
}
")
endfunction()

# mir2x_add_unit_test(NAME <name>
#                      SOURCES <src...>
#                      [LIBS <lib...>]
#                      [DEPENDS <target...>]
#                      [ARGS <args...>]
#                      [GOLD <path>]
#                      [NO_GOLD]
#                      [WORKING_DIRECTORY <dir>])
#
# builds an EXCLUDE_FROM_ALL executable named test_<name> from SOURCES. unless NO_GOLD is
# given, GOLD defaults to "<first-source-without-extension>.log.gold" next to the first
# source file (matches this project's convention of naming e.g. xmltypeset_test.cpp's gold
# file xmltypeset_test.log.gold) - this default is used whether or not that file exists yet,
# so a first `make test` run reports a normal (fixable) failure instead of silently
# skipping the check, and `run_tests.py --update-gold` can create it. when `make test` runs
# this test, its stdout is diffed against GOLD; a mismatch, or a missing GOLD file, is
# reported as a failure. WORKING_DIRECTORY defaults to the directory containing the
# CMakeLists.txt that calls this function.
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

    mir2x__register_test(${T_TARGET} ${T_NAME} "$<TARGET_FILE:${T_TARGET}>;${T_ARGS}" "${T_WORKING_DIRECTORY}" "${T_GOLD}")
endfunction()

# mir2x_add_integration_test(NAME <name>
#                             COMMAND <cmd...>
#                             [DEPENDS <target...>]
#                             [GOLD <path>]
#                             [WORKING_DIRECTORY <dir>])
#
# registers an arbitrary out-of-process command (e.g. `python3 test_client.py`) as a test.
# this function doesn't compile anything itself; if COMMAND's first word names an existing
# CMake target (e.g. the `client` executable), it's resolved to that target's built file
# and the target is added as a build dependency of `make test`, so integration tests can
# directly launch the real module executables they're testing. DEPENDS can list further
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
    # test_<name> executable), so a plain custom target is enough to hold their DEPENDS
    set(T_TARGET "test_${T_NAME}_integration")
    add_custom_target(${T_TARGET})
    if(T_DEPENDS)
        add_dependencies(${T_TARGET} ${T_DEPENDS})
    endif()

    mir2x__register_test(${T_TARGET} ${T_NAME} "${T_COMMAND}" "${T_WORKING_DIRECTORY}" "${T_GOLD}")
endfunction()

# call once, from the root CMakeLists.txt, after every add_subdirectory() that might call
# mir2x_add_unit_test()/mir2x_add_integration_test() has already run
function(mir2x_finalize_tests)
    get_property(T_ALL_TEST_TARGETS GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS)

    add_custom_target(mir2x_build_tests)
    if(T_ALL_TEST_TARGETS)
        add_dependencies(mir2x_build_tests ${T_ALL_TEST_TARGETS})
    endif()

    # `test` builds every registered test first, then runs the runner; this is a plain
    # custom target, not CTest (see the note at the top of this file for why)
    add_custom_target(test
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target mir2x_build_tests
        COMMAND python3 ${CMAKE_SOURCE_DIR}/cmake/run_tests.py --manifest-dir ${MIR2X_TEST_MANIFEST_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        VERBATIM)
endfunction()
