INCLUDE(ExternalProject)

# for the CMAKE_ARGS, the author recommends -DCPACK_PACKAGING_INSTALL_PREFIX=xxx
# but seems on windows this doesn't work

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
