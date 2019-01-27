#=======================================================================================
#
#        Filename: BuildTinyXML2.cmake
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
    tinyxml2

    GIT_REPOSITORY "https://github.com/mir2x-deps/tinyxml2.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/tinyxml2"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/tinyxml2/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_STATIC_LIBS:BOOL=ON -DBUILD_TESTS:BOOL=OFF -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/tinyxml2/build/install

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(TINYXML2_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/tinyxml2/build/install/include")
SET(TINYXML2_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}tinyxml2${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${TINYXML2_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/tinyxml2/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds tinyxml2)
