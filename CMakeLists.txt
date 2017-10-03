#=======================================================================================
#
#        Filename: CMakeLists.txt
#         Created: 05/03/2016 13:19:07
#   Last Modified: 10/03/2017 15:57:28
#
#     Description: -fno-strict-aliasing
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

PROJECT(MIR2X)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)


# 1. add c++11 support, if no then exit
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

IF(COMPILER_SUPPORTS_CXX14)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
ELSEIF(COMPILER_SUPPORTS_CXX11)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSEIF(COMPILER_SUPPORTS_CXX0X)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE()
    MESSAGE(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
ENDIF()

# 2. enable condcheck()
#    cmake disble assert() if in release mode
#    and for item-7 libtheron requires compiler flags to enable NDEBUG
ADD_DEFINITIONS(-DCONDCHECK)

# 3. always compile mir2x in debug mode
#    MIR2X_DEBUG will be used as std::getenv("MIR2X_DEBUG")
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
# SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
# SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb3 -pedantic")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wunused -Werror")

# 4. to enable gprof for performance profiling
#    not use it since I move to google-gproftools for multi-thread support
# SET(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -pg")

# 5. set common / external source used in client, server, tools
SET(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/src)
SET(EXTERNAL_INCLUDE_DIR "")
FILE(GLOB EXTERNAL_ALL_SUBDIR ${CMAKE_CURRENT_SOURCE_DIR}/external/*)
FOREACH(EXTERNAL_SUBDIR ${EXTERNAL_ALL_SUBDIR})
    IF(IS_DIRECTORY ${EXTERNAL_SUBDIR})
        SET(EXTERNAL_INCLUDE_DIR ${EXTERNAL_INCLUDE_DIR} ${EXTERNAL_SUBDIR})
    ENDIF()
ENDFOREACH()

# 6. to include external cmake packages
FIND_PACKAGE(Lua    REQUIRED)
FIND_PACKAGE(SDL2   REQUIRED)
FIND_PACKAGE(FLTK   REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)

# 7. for libthread, compile arguments matching is required
#    reason why I disable NDEBUG
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -DTHERON_CPP11=1 -DTHERON_POSIX=0 -DTHERON_XS=1")

# 6. add tasks to build, each in a sub-dir
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(client)
