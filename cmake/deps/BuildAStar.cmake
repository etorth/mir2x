#=======================================================================================
#
#        Filename: BuildAStar.cmake
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
    astar

    GIT_REPOSITORY "https://github.com/justinhj/astar-algorithm-cpp"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/astar"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

SET(ASTAR_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/astar/cpp")
INCLUDE_DIRECTORIES(SYSTEM ${ASTAR_INCLUDE_DIRS})
