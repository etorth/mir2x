#=======================================================================================
#
#        Filename: BuildZStd.cmake
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
    libzstd

    GIT_REPOSITORY "https://github.com/facebook/zstd.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/libzstd"
    SOURCE_SUBDIR "build/cmake"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/libzstd/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/libzstd/build/install -DZSTD_BUILD_SHARED=OFF -DZSTD_BUILD_STATIC=ON

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(ZSTD_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/libzstd/build/install/include")
SET(ZSTD_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${ZSTD_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/libzstd/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds libzstd)
