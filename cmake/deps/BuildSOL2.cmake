#=======================================================================================
#
#        Filename: BuildSOL2.cmake
#         Created: 05/03/2016 13:19:07
#   Last Modified: 01/10/2018 21:21:34
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
    sol2

    GIT_REPOSITORY "https://github.com/ThePhD/sol2.git"
    GIT_TAG        "develop"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/sol2"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""

    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

SET(SOL2_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/sol2/single")
INCLUDE_DIRECTORIES(SYSTEM ${SOL2_INCLUDE_DIRS})
