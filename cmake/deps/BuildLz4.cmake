#=======================================================================================
#
#        Filename: BuildLz4.cmake
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

IF(WIN32 AND MSVC)
    MESSAGE(STATUS "liblz4 build skipped on windows platform, use vcpkg")
    RETURN()
ENDIF()

ExternalProject_Add(
    liblz4

    GIT_REPOSITORY "https://github.com/lz4/lz4.git"
    GIT_TAG        "dev"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/lz4"
    BUILD_IN_SOURCE TRUE

    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND make install DESTDIR=${MIR2X_3RD_PARTY_DIR}/lz4/install_lz4lib
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(LZ4_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/lz4/install_lz4lib/usr/local/include")
IF(WIN32)
    SET(LZ4_LIBRARIES lz4_static)
ELSE()
    SET(LZ4_LIBRARIES "${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX}")
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM ${LZ4_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/lz4/install_lz4lib/usr/local/lib)
ADD_DEPENDENCIES(mir2x_3rds liblz4)
