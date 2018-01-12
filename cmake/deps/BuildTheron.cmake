#=======================================================================================
#
#        Filename: BuildTheron.cmake
#         Created: 05/03/2016 13:19:07
#   Last Modified: 01/11/2018 21:24:59
#
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
    theron

    GIT_REPOSITORY "https://github.com/etorth/theron.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/theron"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND make library mode=release windows=off boost=off c++11=on posix=off numa=off xs=off shared=off
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    BUILD_IN_SOURCE 1

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(THERON_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/theron/Include")
SET(THERON_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}theron${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${THERON_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/theron/Lib)
