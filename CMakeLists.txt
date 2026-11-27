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

# CMake condition notes:
# - WIN32 means the target platform is Windows, including MSVC and MinGW.
# - MSVC  means the compiler is Microsoft-compatible; MinGW UCRT64 is not MSVC.

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file")
    endif()
endif()

project(mir2x LANGUAGES CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "32-bit builds are not supported")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linux and Windows build everything, Android builds the client only
if(ANDROID)
    set(MIR2X_DESKTOP_BUILD OFF)
else()
    set(MIR2X_DESKTOP_BUILD ON)
endif()

option(MIR2X_BUILD_SERVER      "Build the server"                       ${MIR2X_DESKTOP_BUILD})
option(MIR2X_BUILD_TOOLS       "Build the tools"                        ${MIR2X_DESKTOP_BUILD})
option(MIR2X_BUILD_TESTS       "Build the tests"                        ${MIR2X_DESKTOP_BUILD})
option(MIR2X_ENABLE_PINYIN_IME "Build the client's embedded pinyin IME" ${MIR2X_DESKTOP_BUILD})

if(ANDROID AND (MIR2X_BUILD_SERVER OR MIR2X_BUILD_TOOLS OR MIR2X_BUILD_TESTS OR MIR2X_ENABLE_PINYIN_IME))
    message(FATAL_ERROR "Android builds the client only, without the embedded pinyin IME")
endif()

# FLTK is used by the server GUI and some tools, never by the client
if(MIR2X_BUILD_SERVER OR MIR2X_BUILD_TOOLS)
    set(MIR2X_NEED_FLTK ON)
else()
    set(MIR2X_NEED_FLTK OFF)
endif()

if(ANDROID)
    # client is linked as libmain.so, all static code linked into it must be PIC
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(MIR2X_USE_CCACHE "Use ccache to speed up rebuilds if available" OFF)
if(MIR2X_USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)
    endif(CCACHE_FOUND)
endif()

option(MIR2X_ENABLE_ASAN "Enable ASAN" OFF)
option(MIR2X_ENABLE_USAN "Enable USAN" OFF)
option(MIR2X_ENABLE_TSAN "Enable TSAN" OFF)

if(MIR2X_ENABLE_ASAN)
    if(WIN32 AND MSVC)
        message(STATUS "ASAN not enabled on windows platform")
    else()
        message(STATUS "ASAN enabled")
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address")
    endif()
endif()

if(MIR2X_ENABLE_USAN)
    if(WIN32 AND MSVC)
        message(STATUS "USAN not enabled on windows platform")
    else()
        message(STATUS "USAN enabled")
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=undefined")
    endif()
endif()

if(MIR2X_ENABLE_TSAN)
    if(WIN32 AND MSVC)
        message(STATUS "TSAN not enabled on windows platform")
    else()
        message(STATUS "TSAN enabled")
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=thread")
    endif()
endif()

set(MIR2X_COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)

# configure file
# if not version defined, use version enginerring signagure
if(NOT DEFINED MIR2X_BUILD_SIGNATURE)
    string(TIMESTAMP NOW_TIMESTAMP "%Y%m%d%H%M%S")
    set(MIR2X_BUILD_SIGNATURE "VENGINEERING-${NOW_TIMESTAMP}")
endif()
message(STATUS "Build signature: ${MIR2X_BUILD_SIGNATURE}")

# file list to support logProfiler()
# generate the full file list automatically
if(DEFINED MIR2X_LOG_FILELIST)
    message(FATAL_ERROR "Don't define MIR2X_LOG_FILELIST, cmake automatically collect the file list")
else()
    file(GLOB MIR2X_LOG_SRC_FILELIST
        ${CMAKE_SOURCE_DIR}/common/src/*.[hc]pp
        ${CMAKE_SOURCE_DIR}/client/src/*.[hc]pp
        ${CMAKE_SOURCE_DIR}/server/src/*.[hc]pp
    )

    foreach(LOG_SRC ${MIR2X_LOG_SRC_FILELIST})
        string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" LOG_SRC_BODY ${LOG_SRC})
        string(APPEND MIR2X_LOG_FILELIST "R\"DELIM(${LOG_SRC_BODY})DELIM\",\n")
    endforeach()

    file(GLOB MIR2X_LOG_FL_FILELIST ${CMAKE_SOURCE_DIR}/server/src/*.[fF][lL])
    foreach(LOG_FL_SRC ${MIR2X_LOG_FL_FILELIST})
        get_filename_component(FL_SRC_BASENAME ${LOG_FL_SRC} NAME_WE)
        string(APPEND MIR2X_LOG_FILELIST "R\"DELIM(server/src/${FL_SRC_BASENAME}.hpp)DELIM\",\n")
        string(APPEND MIR2X_LOG_FILELIST "R\"DELIM(server/src/${FL_SRC_BASENAME}.cpp)DELIM\",\n")
    endforeach()
endif()

if(WIN32 AND MSVC)
    string(REPLACE "/" "\\" MIR2X_LOG_FILELIST ${MIR2X_LOG_FILELIST})
endif()

configure_file(${MIR2X_COMMON_SOURCE_DIR}/buildconfig.hpp.in ${CMAKE_BINARY_DIR}/config_file/buildconfig.hpp)
configure_file(${MIR2X_COMMON_SOURCE_DIR}/logprof.hpp.in     ${CMAKE_BINARY_DIR}/config_file/logprof.hpp    )

function(mir2x_list_subdirs ARG_LIST)
    file(GLOB_RECURSE T_F_LIST LIST_DIRECTORIES TRUE "*")
    foreach(T_FILE ${T_F_LIST})
        if(IS_DIRECTORY ${T_FILE})
            list(APPEND T_D_LIST ${T_FILE})
        else()
            continue()
        endif()
    endforeach()
    set(${ARG_LIST} ${T_D_LIST} PARENT_SCOPE)
endfunction()

function(mir2x_list_source_recursive ARG_LIST)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} T_ALL_SRCS)
    mir2x_list_subdirs(T_SUBDIRS)

    foreach(T_DIR ${T_SUBDIRS})
        aux_source_directory(${T_DIR} T_SRCS)
        list(APPEND T_ALL_SRCS ${T_SRCS})
    endforeach()
    set(${ARG_LIST} ${T_ALL_SRCS} PARENT_SCOPE)
endfunction()

function(mir2x_list_source_recursive_filtered ARG_LIST)
    cmake_parse_arguments(PARSE_ARGV 1 T "" "" "EXCLUDE")
    if(T_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments: ${T_UNPARSED_ARGUMENTS}")
    endif()

    mir2x_list_source_recursive(T_ALL_SRCS)
    foreach(T_EXCLUDE ${T_EXCLUDE})
        get_filename_component(T_EXCLUDE ${T_EXCLUDE} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
        list(REMOVE_ITEM T_ALL_SRCS ${T_EXCLUDE})
    endforeach()
    set(${ARG_LIST} ${T_ALL_SRCS} PARENT_SCOPE)
endfunction()

function(fltk_parse_fl_file ARG_HPP ARG_CPP)
    file(GLOB T_ALL_FL "*.[fF][lL]")

    set(RES_HPP "")
    set(RES_CPP "")

    foreach(T_FL ${T_ALL_FL})
        get_filename_component(T_BASE ${T_FL} NAME_WE)

        set(T_OUT_HPP ${T_BASE}.hpp)
        set(T_OUT_CPP ${T_BASE}.cpp)

        set(RES_HPP ${RES_HPP} ${T_OUT_HPP})
        set(RES_CPP ${RES_CPP} ${T_OUT_CPP})

        add_custom_command(OUTPUT ${T_OUT_HPP} ${T_OUT_CPP}
            DEPENDS ${T_FL}
            COMMAND ${FLTK_FLUID_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/${T_OUT_CPP} -h ${CMAKE_CURRENT_BINARY_DIR}/${T_OUT_HPP} ${T_FL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach()

    set(${ARG_HPP} "${RES_HPP}" PARENT_SCOPE)
    set(${ARG_CPP} "${RES_CPP}" PARENT_SCOPE)
endfunction()

include("${CMAKE_SOURCE_DIR}/cmake/Mir2xAssetsPath.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/Mir2xDependencies.cmake")

# Python 3 is used at build/install time:
#
#     tools/installres/src/install_res.py
#
# Find it once here so every caller uses the same interpreter.
# find_package() also resolves Python on platforms without a `python3` executable, such as Windows.
# Override it with -DMIR2X_PYTHON_EXECUTABLE=/path/to/python3 when a specific interpreter is required.
find_package(Python3 COMPONENTS Interpreter)
set(MIR2X_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH
    "Python 3 interpreter used to run install_res.py")
if(NOT MIR2X_PYTHON_EXECUTABLE)
    message(FATAL_ERROR "No python3 interpreter found; set -DMIR2X_PYTHON_EXECUTABLE=/path/to/python3")
endif()
message(STATUS "Using python3: ${MIR2X_PYTHON_EXECUTABLE}")

if(MIR2X_BUILD_TESTS)
    include("${CMAKE_SOURCE_DIR}/cmake/Mir2xTest.cmake")
endif()

set(MIR2X_3RD_PARTY_DIR "${CMAKE_BINARY_DIR}/3rdparty" CACHE PATH "Directory for downloaded third-party files")
include("${CMAKE_SOURCE_DIR}/cmake/download/debugger.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/download/inspect.cmake")

set(CMAKE_SKIP_RPATH TRUE)

add_subdirectory(common)

# zsdbmaker packs the client and server resources, so it must run on the build host
# Android builds it for the host as a standalone project, imported here as the same target name
if(ANDROID)
    set(MIR2X_HOST_CXX_COMPILER "" CACHE FILEPATH "Host C++26 compiler used to build zsdbmaker for the build host")
    if(NOT MIR2X_HOST_CXX_COMPILER)
        message(FATAL_ERROR "Android build needs -DMIR2X_HOST_CXX_COMPILER=<host C++26 compiler> to build zsdbmaker for the build host")
    endif()

    if(CMAKE_HOST_WIN32)
        set(MIR2X_HOST_ZSDBMAKER ${CMAKE_BINARY_DIR}/zsdbmaker_host/zsdbmaker.exe)
    else()
        set(MIR2X_HOST_ZSDBMAKER ${CMAKE_BINARY_DIR}/zsdbmaker_host/zsdbmaker)
    endif()

    include(ExternalProject)
    ExternalProject_Add(zsdbmaker_host
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/zsdbmaker
        BINARY_DIR ${CMAKE_BINARY_DIR}/zsdbmaker_host
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_CXX_COMPILER=${MIR2X_HOST_CXX_COMPILER}
            -DCMAKE_PREFIX_PATH=${VCPKG_INSTALLED_DIR}/${VCPKG_HOST_TRIPLET}
        BUILD_ALWAYS ON
        BUILD_BYPRODUCTS ${MIR2X_HOST_ZSDBMAKER}
        INSTALL_COMMAND "")

    add_executable(zsdbmaker IMPORTED GLOBAL)
    set_target_properties(zsdbmaker PROPERTIES IMPORTED_LOCATION ${MIR2X_HOST_ZSDBMAKER})
    add_dependencies(zsdbmaker zsdbmaker_host)
else()
    add_subdirectory(zsdbmaker)
endif()

if(MIR2X_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(MIR2X_BUILD_SERVER)
    add_subdirectory(server)
endif()

add_subdirectory(client)

# The top-level test/ directory holds cross-module integration tests, such as server and client scenarios.
# It must be added after all modules so mir2x_finalize_tests() sees every registered test.
if(MIR2X_BUILD_TESTS)
    add_subdirectory(test)
    mir2x_finalize_tests()
endif()

set(COMMAND_GIT_TAG "git describe --tag")
execute_process(
    COMMAND           ${COMMAND_GIT_TAG}
    OUTPUT_VARIABLE   COMMAND_GIT_TAG_OUTPUT
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
string(REGEX REPLACE "\n$" "" PROJ_GIT_TAG_STR "${COMMAND_GIT_TAG_OUTPUT}")

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VERSION ${PROJ_GIT_TAG_STR})
include(CPack)
