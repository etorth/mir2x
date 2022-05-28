#=======================================================================================
#
#        Filename: BuildG3Log.cmake
#         Created: 05/03/2016 13:19:07
#     Description: required: MIR2X_3RD_PARTY_DIR
#
#         Version: 1.0
#        Revision: none
#        Compiler: cmake
#
#          Author: ANHONG
#           Email: anhonghe@gmail.com
#    Organization: USTC
#
#=======================================================================================

INCLUDE(ExternalProject)

# for the CMAKE_ARGS, the author recommends -DCPACK_PACKAGING_INSTALL_PREFIX=xxx
# but seems on windows this doesn't work

ExternalProject_Add(
    SQLiteCpp

    GIT_REPOSITORY "https://github.com/SRombauts/SQLiteCpp"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/SQLiteCpp"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/SQLiteCpp/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/SQLiteCpp/build/install -DCMAKE_BUILD_TYPE=Release

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(SQLITECPP_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/SQLiteCpp/build/install/include")
SET(SQLITECPP_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}SQLiteCpp${CMAKE_STATIC_LIBRARY_SUFFIX}" "${CMAKE_STATIC_LIBRARY_PREFIX}sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${SQLITECPP_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/SQLiteCpp/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds SQLiteCpp)
