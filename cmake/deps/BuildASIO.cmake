#=======================================================================================
#
#        Filename: BuildASIO.cmake
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
    asio

    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    # GIT_TAG        "master"
    GIT_TAG        "asio-1-10-8"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/asio"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

ADD_COMPILE_DEFINITIONS(ASIO_STANDALONE)
SET(ASIO_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/asio/asio/include")
INCLUDE_DIRECTORIES(SYSTEM ${ASIO_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds asio)
