#=======================================================================================
#
#        Filename: BuildMXTK.cmake
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

ExternalProject_Add(
    mxtk

    GIT_REPOSITORY "https://github.com/etorth/mxtk.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/mxtk"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/mxtk/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/mxtk/build/install -DCMAKE_BUILD_TYPE=Debug

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(MXTK_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/mxtk/src")
SET(MXTK_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}mxtk${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${MXTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/mxtk/build/install)
