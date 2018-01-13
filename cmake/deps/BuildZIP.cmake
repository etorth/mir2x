#=======================================================================================
#
#        Filename: BuildZIP.cmake
#         Created: 05/03/2016 13:19:07
#   Last Modified: 01/12/2018 18:47:47
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
    libzip

    GIT_REPOSITORY "https://github.com/mir2x-deps/libzip.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/libzip"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/libzip/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/libzip/build/install -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(ZIP_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/libzip/build/install/include")
SET(ZIP_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${ZIP_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/libzip/build/install/lib)
