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

PROJECT(mir2x)
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable condcheck()
# cmake disble assert() if in release mode
ADD_COMPILE_DEFINITIONS(CONDCHECK)

IF(WIN32 AND MSVC)
    # won't use the /WX
    # have a lot secure warnings
    ADD_COMPILE_OPTIONS(/W4)
    ADD_COMPILE_DEFINITIONS(WIN32_LEAN_AND_MEAN)
    ADD_COMPILE_DEFINITIONS(_HAS_STD_BYTE=0)
ELSE()
    ADD_COMPILE_OPTIONS(-fcoroutines)
    ADD_COMPILE_OPTIONS(-fno-strict-aliasing)
    ADD_COMPILE_OPTIONS(-Wall -Wfatal-errors -Wextra -Wunused -Werror)

    STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " $ENV{CXXFLAGS}")
    STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0")
    STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -g3 -ggdb3 -pedantic")

    # add flags to release mode
    # by default won't use release mode
    STRING(APPEND CMAKE_CXX_FLAGS_RELEASE " $ENV{CXXFLAGS}")
    STRING(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")
ENDIF()

OPTION(MIR2X_ENABLE_ASAN "Enable ASAN" OFF)
OPTION(MIR2X_ENABLE_USAN "Enable USAN" OFF)
OPTION(MIR2X_ENABLE_TSAN "Enable TSAN" OFF)

OPTION(MIR2X_ENABLE_SQLITE3 "Enable SQLite3" ON)
OPTION(MIR2X_ENABLE_MYSQL   "Enable MySQL"   OFF)

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

IF(MIR2X_ENABLE_SQLITE3)
    MESSAGE(STATUS "SQLite3 supported")
    ADD_COMPILE_DEFINITIONS(MIR2X_ENABLE_SQLITE3)
ENDIF()

IF(MIR2X_ENABLE_MYSQL)
    MESSAGE(STATUS "MySQL(mysql/mariadb) supported")
    ADD_COMPILE_DEFINITIONS(MIR2X_ENABLE_MYSQL)
ENDIF()

SET(MIR2X_3RD_PARTY_DIR "${CMAKE_BINARY_DIR}/3rdparty")
SET(MIR2X_COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common/src)

# configure file
# if not version defined, use version enginerring signagure
IF(NOT DEFINED MIR2X_BUILD_SIGNATURE)
    STRING(TIMESTAMP NOW_TIMESTAMP "%Y%m%d%H%M%S")
    SET(MIR2X_BUILD_SIGNATURE "VENGINEERING-${NOW_TIMESTAMP}")
ENDIF()
MESSAGE(STATUS "Build signature: ${MIR2X_BUILD_SIGNATURE}")

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_BINARY_DIR}/config_file)
CONFIGURE_FILE(${MIR2X_COMMON_SOURCE_DIR}/buildconfig.hpp.in ${CMAKE_BINARY_DIR}/config_file/buildconfig.hpp)

# set additional module path
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

ADD_CUSTOM_TARGET(mir2x_3rds)
FILE(GLOB MIR2X_DEPS_DIR "${CMAKE_SOURCE_DIR}/cmake/deps/*.cmake")
FOREACH(MIR2X_DEP ${MIR2X_DEPS_DIR})
    INCLUDE(${MIR2X_DEP})
ENDFOREACH()

IF(WIN32)
    CMAKE_POLICY(SET CMP0003 OLD)
ENDIF()

# include external cmake packages
FIND_PACKAGE(Lua     REQUIRED)
FIND_PACKAGE(FLTK    REQUIRED)
FIND_PACKAGE(OpenGL  REQUIRED)
FIND_PACKAGE(PNG     REQUIRED)
# FIND_PACKAGE(SQLite3 REQUIRED) # seems travis has no sqlite3 ?

# TODO
# appveyor says "can find make command"
# use vcpkg to get lz4, but for linux still use github to get lz4
IF(WIN32 AND MSVC)
    FIND_PACKAGE(LZ4 REQUIRED)
ENDIF()

# report sdl2 status
# seems it won't report in find_package
IF(WIN32)
    FIND_PACKAGE(SDL2 CONFIG REQUIRED)
    FIND_PACKAGE(FreeType REQUIRED)
ELSE()
    FIND_PACKAGE(SDL2 REQUIRED)
ENDIF()

MESSAGE(STATUS "SDL2 found: SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}, SDL2_LIBRARIES: ${SDL2_LIBRARIES}")

IF(WIN32)
    FIND_PACKAGE(JPEG REQUIRED)
ENDIF()

IF(NOT WIN32)
    SET(CMAKE_THREAD_PREFER_PTHREAD ON)
ENDIF()
FIND_PACKAGE(Threads REQUIRED)

SET(CMAKE_SKIP_RPATH TRUE)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(tools )

SET(COMMAND_GIT_TAG git describe --tag)
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
