# zsdbmaker packs resources at build/install time, so it has to run on the build host
# it is built in two ways from this file:
#
#   1. desktop: add_subdirectory(zsdbmaker) from the top-level CMakeLists.txt
#   2. Android: a standalone host build, ExternalProject_Add() in the top-level CMakeLists.txt
#
# both ways it only needs zstd and argh
# the common sources below are compiled with sol2 disabled, don't link the common library

cmake_minimum_required(VERSION 3.21)
project(zsdbmaker LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 26)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(zstd CONFIG REQUIRED)
find_package(argh CONFIG REQUIRED)

set(ZSDBMAKER_COMMON_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../common/src)

add_executable(zsdbmaker
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${ZSDBMAKER_COMMON_SOURCE_DIR}/zsdb.cpp
    ${ZSDBMAKER_COMMON_SOURCE_DIR}/strf.cpp
    ${ZSDBMAKER_COMMON_SOURCE_DIR}/raiitimer.cpp)

target_include_directories(zsdbmaker PRIVATE ${ZSDBMAKER_COMMON_SOURCE_DIR})
target_compile_definitions(zsdbmaker PRIVATE
    MIR2X_STRF_DISABLE_SOL2
    MIR2X_TOTYPE_DISABLE_SOL2)

target_link_libraries(zsdbmaker PRIVATE
    zstd::libzstd
    argh)

if(TARGET mir2x::project_options)
    target_link_libraries(zsdbmaker PRIVATE mir2x::project_options)
endif()

install(TARGETS zsdbmaker DESTINATION tools/zsdbmaker)
