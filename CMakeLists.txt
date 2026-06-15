#=======================================================================================
#
#        Filename: CMakeLists.txt
#         Created: 05/03/2016 13:19:07
#     Description:
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

IF(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    IF(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        SET(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file")
    ENDIF()
ENDIF()

PROJECT(mir2x)

SET(CMAKE_CXX_STANDARD 26)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

FIND_PROGRAM(CCACHE_FOUND ccache)
IF(CCACHE_FOUND)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
ENDIF(CCACHE_FOUND)

OPTION(MIR2X_ENABLE_ASAN "Enable ASAN" OFF)
OPTION(MIR2X_ENABLE_USAN "Enable USAN" OFF)
OPTION(MIR2X_ENABLE_TSAN "Enable TSAN" OFF)

IF(MIR2X_ENABLE_ASAN)
    IF(WIN32 AND MSVC)
        MESSAGE(STATUS "ASAN not enabled on windows platform")
    ELSE()
        MESSAGE(STATUS "ASAN enabled")
        STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address")
    ENDIF()
ENDIF()

IF(MIR2X_ENABLE_USAN)
    IF(WIN32 AND MSVC)
        MESSAGE(STATUS "USAN not enabled on windows platform")
    ELSE()
        MESSAGE(STATUS "USAN enabled")
        STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=undefined")
    ENDIF()
ENDIF()

IF(MIR2X_ENABLE_TSAN)
    IF(WIN32 AND MSVC)
        MESSAGE(STATUS "TSAN not enabled on windows platform")
    ELSE()
        MESSAGE(STATUS "TSAN enabled")
        STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=thread")
    ENDIF()
ENDIF()

SET(MIR2X_COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)
SET(MIR2X_RES_REPO_PATH "" CACHE PATH "Path to the mir2x resource repository used by install-time resource packing")

# configure file
# if not version defined, use version enginerring signagure
IF(NOT DEFINED MIR2X_BUILD_SIGNATURE)
    STRING(TIMESTAMP NOW_TIMESTAMP "%Y%m%d%H%M%S")
    SET(MIR2X_BUILD_SIGNATURE "VENGINEERING-${NOW_TIMESTAMP}")
ENDIF()
MESSAGE(STATUS "Build signature: ${MIR2X_BUILD_SIGNATURE}")

# file list to support logProfiler()
# generate the full file list automatically
IF(DEFINED MIR2X_LOG_FILELIST)
    MESSAGE(FATAL_ERROR "Don't define MIR2X_LOG_FILELIST, cmake automatically collect the file list")
ELSE()
    FILE(GLOB MIR2X_LOG_SRC_FILELIST
        ${CMAKE_SOURCE_DIR}/common/src/*.[hc]pp
        ${CMAKE_SOURCE_DIR}/client/src/*.[hc]pp
        ${CMAKE_SOURCE_DIR}/server/src/*.[hc]pp
    )

    FOREACH(LOG_SRC ${MIR2X_LOG_SRC_FILELIST})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" LOG_SRC_BODY ${LOG_SRC})
        STRING(APPEND MIR2X_LOG_FILELIST "R\"DELIM(${LOG_SRC_BODY})DELIM\",\n")
    ENDFOREACH()

    FILE(GLOB MIR2X_LOG_FL_FILELIST ${CMAKE_SOURCE_DIR}/server/src/*.[fF][lL])
    FOREACH(LOG_FL_SRC ${MIR2X_LOG_FL_FILELIST})
        GET_FILENAME_COMPONENT(FL_SRC_BASENAME ${LOG_FL_SRC} NAME_WE)
        STRING(APPEND MIR2X_LOG_FILELIST "R\"DELIM(server/src/${FL_SRC_BASENAME}.hpp)DELIM\",\n")
        STRING(APPEND MIR2X_LOG_FILELIST "R\"DELIM(server/src/${FL_SRC_BASENAME}.cpp)DELIM\",\n")
    ENDFOREACH()
ENDIF()

IF(WIN32 AND MSVC)
    STRING(REPLACE "/" "\\" MIR2X_LOG_FILELIST ${MIR2X_LOG_FILELIST})
ENDIF()

CONFIGURE_FILE(${MIR2X_COMMON_SOURCE_DIR}/buildconfig.hpp.in ${CMAKE_BINARY_DIR}/config_file/buildconfig.hpp)
CONFIGURE_FILE(${MIR2X_COMMON_SOURCE_DIR}/logprof.hpp.in     ${CMAKE_BINARY_DIR}/config_file/logprof.hpp    )

FUNCTION(MIR2X_LIST_SUBDIRS ARG_LIST)
    FILE(GLOB_RECURSE T_F_LIST LIST_DIRECTORIES TRUE "*")
    FOREACH(T_FILE ${T_F_LIST})
        IF(IS_DIRECTORY ${T_FILE})
            LIST(APPEND T_D_LIST ${T_FILE})
        ELSE()
            CONTINUE()
        ENDIF()
    ENDFOREACH()
    SET(${ARG_LIST} ${T_D_LIST} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(MIR2X_LIST_SOURCE_RECURSIVE ARG_LIST)
    AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} T_ALL_SRCS)
    MIR2X_LIST_SUBDIRS(T_SUBDIRS)

    FOREACH(T_DIR ${T_SUBDIRS})
        AUX_SOURCE_DIRECTORY(${T_DIR} T_SRCS)
        LIST(APPEND T_ALL_SRCS ${T_SRCS})
    ENDFOREACH()
    SET(${ARG_LIST} ${T_ALL_SRCS} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(FLTK_PARSE_FL_FILE ARG_HPP ARG_CPP)
    FILE(GLOB T_ALL_FL "*.[fF][lL]")

    SET(RES_HPP "")
    SET(RES_CPP "")

    FOREACH(T_FL ${T_ALL_FL})
        GET_FILENAME_COMPONENT(T_BASE ${T_FL} NAME_WE)

        SET(T_OUT_HPP ${T_BASE}.hpp)
        SET(T_OUT_CPP ${T_BASE}.cpp)

        SET(RES_HPP ${RES_HPP} ${T_OUT_HPP})
        SET(RES_CPP ${RES_CPP} ${T_OUT_CPP})

        ADD_CUSTOM_COMMAND(OUTPUT ${T_OUT_HPP} ${T_OUT_CPP}
            DEPENDS ${T_FL}
            COMMAND ${FLTK_FLUID_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/${T_OUT_CPP} -h ${CMAKE_CURRENT_BINARY_DIR}/${T_OUT_HPP} ${T_FL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    ENDFOREACH()

    SET(${ARG_HPP} "${RES_HPP}" PARENT_SCOPE)
    SET(${ARG_CPP} "${RES_CPP}" PARENT_SCOPE)
ENDFUNCTION()

IF(MIR2X_RES_REPO_PATH)
    IF(IS_DIRECTORY ${MIR2X_RES_REPO_PATH})
        MESSAGE(STATUS "Customized mir2x resource repository path provided: ${MIR2X_RES_REPO_PATH}")
    ELSE()
        MESSAGE(FATAL_ERROR "Invalid MIR2X_RES_REPO_PATH provided: invalid directory")
    ENDIF()
ELSE()
    MESSAGE(STATUS "MIR2X_RES_REPO_PATH not provided: install-time resource packing is disabled")
ENDIF()

IF(WIN32)
    CMAKE_POLICY(SET CMP0003 OLD)
ENDIF()

INCLUDE("${CMAKE_SOURCE_DIR}/cmake/Mir2xDependencies.cmake")

SET(CMAKE_SKIP_RPATH TRUE)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(tools )
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(client)

SET(COMMAND_GIT_TAG "git describe --tag")
EXECUTE_PROCESS(
    COMMAND           ${COMMAND_GIT_TAG}
    OUTPUT_VARIABLE   COMMAND_GIT_TAG_OUTPUT
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
STRING(REGEX REPLACE "\n$" "" PROJ_GIT_TAG_STR "${COMMAND_GIT_TAG_OUTPUT}")

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_VERSION ${PROJ_GIT_TAG_STR})
INCLUDE(CPack)
