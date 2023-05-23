INCLUDE(ExternalProject)

# for the CMAKE_ARGS, the author recommends -DCPACK_PACKAGING_INSTALL_PREFIX=xxx
# but seems on windows this doesn't work

ExternalProject_Add(
    zlib

    URL "https://www.zlib.net/zlib-1.2.13.tar.gz"
    DOWNLOAD_NAME "zlib"
    DOWNLOAD_EXTRACT_TIMESTAMP 0

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/zlib"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/zlib/build/install"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/zlib/build/install

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(ZLIB_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/argh/build/install/include")
SET(ZLIB_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${ZLIB_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/zlib/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds zlib)

ExternalProject_Add(
    libpng

    GIT_REPOSITORY "https://github.com/etorth/libpng-apng-support"
    GIT_TAG        "main"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/libpng"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/libpng/build/install"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CONFIGURE_COMMAND ${MIR2X_3RD_PARTY_DIR}/libpng/libpng-1.6.39/configure --prefix=${MIR2X_3RD_PARTY_DIR}/libpng/build/install

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(PNG_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/libpng/build/install/include")
SET(PNG_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}png${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${PNG_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/libpng/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds libpng)
ADD_DEPENDENCIES(libpng zlib)
