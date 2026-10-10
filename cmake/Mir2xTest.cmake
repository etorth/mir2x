# Tests use the Catch2 v3 package selected by the project's vcpkg baseline.
# Unit executables stay EXCLUDE_FROM_ALL; check builds them before running CTest.
#
#   cmake --build <builddir> --target check --parallel 10
#   ctest --test-dir <builddir> --output-on-failure
#   ctest --test-dir <builddir> -R '^xmltypeset/'

enable_testing()
find_package(Catch2 3...<4 CONFIG REQUIRED)
include(Catch)

define_property(GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS
    BRIEF_DOCS "targets that must be built before running ctest"
    FULL_DOCS "targets that must be built before running ctest")

# mir2x_add_unit_test(NAME <name> SOURCES <src...>
#                    [LIBS <lib...>] [DEPENDS <target...>]
#                    [ARGS <args...>] [WORKING_DIRECTORY <dir>])
function(mir2x_add_unit_test)
    cmake_parse_arguments(T "" "NAME;WORKING_DIRECTORY" "SOURCES;LIBS;DEPENDS;ARGS" ${ARGN})
    if(T_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "mir2x_add_unit_test: unexpected arguments: ${T_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT T_NAME OR NOT T_SOURCES)
        message(FATAL_ERROR "mir2x_add_unit_test: NAME and SOURCES are required")
    endif()

    set(T_TARGET "test_${T_NAME}")
    add_executable(${T_TARGET} EXCLUDE_FROM_ALL ${T_SOURCES})
    target_link_libraries(${T_TARGET} PRIVATE Catch2::Catch2WithMain ${T_LIBS})
    if(T_DEPENDS)
        add_dependencies(${T_TARGET} ${T_DEPENDS})
    endif()
    if(NOT T_WORKING_DIRECTORY)
        set(T_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    catch_discover_tests(${T_TARGET}
        TEST_PREFIX "${T_NAME}/"
        EXTRA_ARGS ${T_ARGS}
        WORKING_DIRECTORY "${T_WORKING_DIRECTORY}"
        PROPERTIES TIMEOUT 600)
    set_property(GLOBAL APPEND PROPERTY MIR2X_TEST_BUILD_TARGETS ${T_TARGET})
endfunction()

# External-command integration tests are registered directly with CTest.
# A target used as the command is also added as a build dependency.
function(mir2x_add_integration_test)
    cmake_parse_arguments(T "" "NAME;WORKING_DIRECTORY" "COMMAND;DEPENDS" ${ARGN})
    if(T_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "mir2x_add_integration_test: unexpected arguments: ${T_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT T_NAME OR NOT T_COMMAND)
        message(FATAL_ERROR "mir2x_add_integration_test: NAME and COMMAND are required")
    endif()

    list(GET T_COMMAND 0 T_CMD0)
    if(TARGET ${T_CMD0})
        list(APPEND T_DEPENDS ${T_CMD0})
        list(REMOVE_AT T_COMMAND 0)
        list(PREPEND T_COMMAND "$<TARGET_FILE:${T_CMD0}>")
    endif()
    if(NOT T_WORKING_DIRECTORY)
        set(T_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(T_BUILD_TARGET "test_${T_NAME}_integration")
    add_custom_target(${T_BUILD_TARGET})
    if(T_DEPENDS)
        add_dependencies(${T_BUILD_TARGET} ${T_DEPENDS})
    endif()
    set_property(GLOBAL APPEND PROPERTY MIR2X_TEST_BUILD_TARGETS ${T_BUILD_TARGET})
    add_test(NAME ${T_NAME} COMMAND ${T_COMMAND})
    set_tests_properties(${T_NAME} PROPERTIES
        WORKING_DIRECTORY "${T_WORKING_DIRECTORY}" TIMEOUT 600)
endfunction()

function(mir2x_finalize_tests)
    get_property(T_ALL_TEST_TARGETS GLOBAL PROPERTY MIR2X_TEST_BUILD_TARGETS)
    add_custom_target(mir2x_build_tests)
    if(T_ALL_TEST_TARGETS)
        add_dependencies(mir2x_build_tests ${T_ALL_TEST_TARGETS})
    endif()

    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS mir2x_build_tests
        USES_TERMINAL
        VERBATIM)
endfunction()
