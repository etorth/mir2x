INCLUDE(ExternalProject)

ExternalProject_Add(
    phmap

    GIT_REPOSITORY "https://github.com/greg7mdp/parallel-hashmap.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/phmap"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/phmap/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/phmap/build/install -DCMAKE_BUILD_TYPE=Release -DPHMAP_BUILD_TESTS=OFF -DPHMAP_BUILD_EXAMPLES=OFF

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(PHMAP_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/phmap/build/install/include")
INCLUDE_DIRECTORIES(SYSTEM ${PHMAP_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds phmap)
