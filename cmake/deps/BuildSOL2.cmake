#=======================================================================================
#
#        Filename: BuildSOL2.cmake
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
    sol2

    GIT_REPOSITORY "https://github.com/ThePhD/sol2.git"
    # GIT_TAG        "develop"
    GIT_TAG        "v2.20.6"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/sol2"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""

    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

ADD_COMPILE_DEFINITIONS(SOL_SAFE_NUMERICS=1)
SET(SOL2_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/sol2/single")
INCLUDE_DIRECTORIES(SYSTEM ${SOL2_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds sol2)
