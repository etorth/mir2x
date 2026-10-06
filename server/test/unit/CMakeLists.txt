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

# same working directory as luarunner
mir2x_add_unit_test(
    NAME queststate
    SOURCES test_queststate.cpp
    LIBS server_objs
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/server
)

# loads server/src/player.lua, same working directory as luarunner
mir2x_add_unit_test(
    NAME playerquest
    SOURCES test_playerquest.cpp
    LIBS server_objs
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/server
)

# loads server/src/servermap.lua, same working directory as luarunner
mir2x_add_unit_test(
    NAME gridtrigger
    SOURCES test_gridtrigger.cpp
    LIBS server_objs
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/server
)
