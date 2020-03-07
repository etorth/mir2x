#=======================================================================================
#
#        Filename: BuildACO.cmake
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
    MESSAGE(STATUS "libaco disabled on windows platform")
    RETURN()
ENDIF()

ExternalProject_Add(
    aco

    GIT_REPOSITORY "https://github.com/hnes/libaco.git"
    GIT_TAG        "3eda876efe913eda9b4a9d3478ef4e002ccfb9ae"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/aco"
    BUILD_IN_SOURCE TRUE

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_C_COMPILER} -O3 -Wall -Werror -c acosw.S aco.c
    INSTALL_COMMAND ${CMAKE_AR} rcs ${MIR2X_3RD_PARTY_DIR}/aco/libaco.a acosw.o aco.o
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

# ADD_COMPILE_DEFINITIONS(ACO_STANDALONE)
SET(ACO_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/aco")
INCLUDE_DIRECTORIES(SYSTEM ${ACO_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds aco)
