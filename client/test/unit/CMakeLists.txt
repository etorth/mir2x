# AI-generated unit tests for individual client/src classes/submodules. each test is its
# own standalone executable (see mir2x_add_unit_test() in cmake/Mir2xTest.cmake), reusing
# client_objs (the client build minus main.cpp) so it exercises the exact same compiled
# code as the real client without recompiling it.
#
# these targets are EXCLUDE_FROM_ALL: they do not slow down normal "build everything" runs.
# build+run them all via `make test` from the top of the build tree, or build a single one
# explicitly, e.g.:
#
#   cmake --build <builddir> --target test_xmltypeset
#   <builddir>/client/test/unit/test_xmltypeset
#
# unit tests that need the packed client resources (fontex.zsdb / emoji.zsdb) require
# `cmake --install <builddir>` to have been run at least once; they default to reading
# from ${CMAKE_INSTALL_PREFIX}/client/res, override with an explicit ARGS resource dir.

mir2x_add_unit_test(
    NAME xmltypeset
    SOURCES xmltypeset_test.cpp
    LIBS client_objs
    DEPENDS zsdbmaker mir2x_resources
)
target_compile_definitions(test_xmltypeset PRIVATE
    MIR2X_TEST_DEFAULT_RES_DIR="${CMAKE_INSTALL_PREFIX}/client/res")
