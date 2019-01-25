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
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable condcheck()
# cmake disble assert() if in release mode
ADD_DEFINITIONS(-DCONDCHECK)

# add flags to release mode
# by default won't use release mode
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wfatal-errors -Wextra -Wunused -Werror")

# always compile mir2x in debug mode
# for trace message use MIR2X_DEBUG_ARG_LIST to enable
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# enable check for asan/usan/tsan
# keep in mind that these three can't be set at the same time

OPTION(MIR2X_ENABLE_ASAN "Enable ASAN" OFF)
OPTION(MIR2X_ENABLE_USAN "Enable USAN" OFF)
OPTION(MIR2X_ENABLE_TSAN "Enable TSAN" OFF)

OPTION(MIR2X_ENABLE_SQLITE3 "Enable SQLite3" ON)
OPTION(MIR2X_ENABLE_MYSQL   "Enable MySQL"   OFF)

IF(MIR2X_ENABLE_SQLITE3)
    MESSAGE(STATUS "SQLite3 supported")
    ADD_DEFINITIONS(-DMIR2X_ENABLE_SQLITE3)
ENDIF()

IF(MIR2X_ENABLE_MYSQL)
    MESSAGE(STATUS "MySQL(mysql/mariadb) supported")
    ADD_DEFINITIONS(-DMIR2X_ENABLE_MYSQL)
ENDIF()

IF(MIR2X_ENABLE_ASAN)
    MESSAGE(STATUS "ASAN enabled")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
ENDIF()

IF(MIR2X_ENABLE_USAN)
    MESSAGE(STATUS "USAN enabled")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
ENDIF()

IF(MIR2X_ENABLE_TSAN)
    MESSAGE(STATUS "TSAN enabled")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb3 -pedantic")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wunused -Werror")

# to enable gprof for performance profiling
# not use it since I move to google-gproftools for multi-thread support
# SET(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -pg")

# set common source used in client, server, tools
SET(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/src)
SET(MIR2X_3RD_PARTY_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdparty")

# set additional module path
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

FILE(GLOB MIR2X_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps/*.cmake")
FOREACH(MIR2X_DEP ${MIR2X_DEPS_DIR})
    INCLUDE(${MIR2X_DEP})
ENDFOREACH()

IF(WIN32)
    CMAKE_POLICY(SET CMP0003 OLD)
ENDIF()

# include external cmake packages
FIND_PACKAGE(Lua    REQUIRED)
FIND_PACKAGE(SDL2   REQUIRED)
FIND_PACKAGE(FLTK   REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(ZLIB   REQUIRED)
FIND_PACKAGE(PNG    REQUIRED)

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
STRING(REGEX REPLACE "\n$" "" PROJ_GIT_TAG_STR ${COMMAND_GIT_TAG_OUTPUT})

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_VERSION ${PROJ_GIT_TAG_STR})
INCLUDE(CPack)
