# standalone regression tests for client/src classes that are complicated enough to
# deserve their own coverage (currently just XMLTypeset). each test is its own
# executable with its own main(), reusing client_objs (the client build minus main.cpp)
# so it exercises the exact same compiled code as the real client without recompiling it.
#
# these targets are EXCLUDE_FROM_ALL: they do not slow down normal "build everything" runs,
# build them explicitly instead, e.g.:
#
#   cmake --build <builddir> --target xmltypeset_test
#   <builddir>/client/src/test/xmltypeset_test
#
# xmltypeset_test needs the packed client resources (fontex.zsdb / emoji.zsdb) to already
# exist, i.e. `cmake --install <builddir>` must have been run at least once; it defaults to
# reading them from ${CMAKE_INSTALL_PREFIX}/client/res, override with:
#
#   xmltypeset_test [res_dir]

add_executable(xmltypeset_test EXCLUDE_FROM_ALL xmltypesettest.cpp)
target_link_libraries(xmltypeset_test PRIVATE client_objs)
target_compile_definitions(xmltypeset_test PRIVATE
    MIR2X_TEST_DEFAULT_RES_DIR="${CMAKE_INSTALL_PREFIX}/client/res")
