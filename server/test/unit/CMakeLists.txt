mir2x_add_unit_test(
    NAME itemgrant
    SOURCES test_itemgrant.cpp
    LIBS server_objs
)

# runs in the installed server directory, the lua runner loads script/api from there, needs `cmake --install` once
mir2x_add_unit_test(
    NAME luarunner
    SOURCES test_luarunner.cpp
    LIBS server_objs
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/server
)
