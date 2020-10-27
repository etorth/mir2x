#=======================================================================================
#
#        Filename: BuildCppCoro.cmake
#         Created: 10/26/2020 21:40:07
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
    cppcoro

    # use a fork which has better gcc-10.2 support
    GIT_REPOSITORY "https://github.com/andreasbuhr/cppcoro"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/cppcoro"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/cppcoro/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/cppcoro/build/install -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(CPPCORO_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/cppcoro/build/install/include")
SET(CPPCORO_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}cppcoro${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${CPPCORO_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/cppcoro/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds cppcoro)
