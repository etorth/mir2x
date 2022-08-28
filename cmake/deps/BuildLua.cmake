INCLUDE(ExternalProject)

IF(WIN32 AND MSVC)
    MESSAGE(STATUS "lua build skipped on windows platform, use vcpkg")
    RETURN()
ENDIF()

ExternalProject_Add(
    lua

    URL "http://www.lua.org/ftp/lua-5.4.4.tar.gz"
    DOWNLOAD_NAME "lua"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/lua"
    BUILD_IN_SOURCE TRUE

    CONFIGURE_COMMAND ""
    BUILD_COMMAND make all test
    INSTALL_COMMAND make install INSTALL_TOP=${MIR2X_3RD_PARTY_DIR}/lua/install
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(LUA_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/lua/install/include")
IF(WIN32)
    SET(LUA_LIBRARIES lua_static)
ELSE()
    SET(LUA_LIBRARIES "${CMAKE_STATIC_LIBRARY_PREFIX}lua${CMAKE_STATIC_LIBRARY_SUFFIX}")
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM ${LUA_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/lua/install/lib)
ADD_DEPENDENCIES(mir2x_3rds lua)
