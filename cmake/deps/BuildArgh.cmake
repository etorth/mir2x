INCLUDE(ExternalProject)

ExternalProject_Add(
    argh

    GIT_REPOSITORY "https://github.com/adishavit/argh.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/argh"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/argh/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/argh/build/install

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(ARGH_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/argh/build/install/include")
# SET(ARGH_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${ARGH_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds argh)
