#=======================================================================================
#
#        Filename: BuildUTF8CPP.cmake
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
    mirror_utf8cpp

    GIT_REPOSITORY "https://github.com/etorth/mirror_utf8cpp.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/mirror_utf8cpp"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

SET(UTF8CPP_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/mirror_utf8cpp/source")
INCLUDE_DIRECTORIES(SYSTEM ${UTF8CPP_INCLUDE_DIRS})
