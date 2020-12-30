#=======================================================================================
#
#        Filename: BuildCereal.cmake
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
    cereal

    GIT_REPOSITORY "https://github.com/USCiLab/cereal.git"
    # GIT_TAG        "master"
    GIT_TAG        "v1.3.0"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/cereal"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

SET(CEREAL_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/cereal/include")
INCLUDE_DIRECTORIES(SYSTEM ${CEREAL_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds cereal)
