INCLUDE(ExternalProject)

ExternalProject_Add(
    tiny-aes

    GIT_REPOSITORY "https://github.com/kokke/tiny-AES-c"
    GIT_TAG        "master"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/tiny-aes"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/tiny-aes/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    INSTALL_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/tiny-aes/build/install -DCMAKE_BUILD_TYPE=Release

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(TINYAES_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/tiny-aes/build/install/include")
SET(TINYAES_LIBRARIES    "${CMAKE_STATIC_LIBRARY_PREFIX}tiny-aes${CMAKE_STATIC_LIBRARY_SUFFIX}")

INCLUDE_DIRECTORIES(SYSTEM ${TINYAES_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/tiny-aes/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds tiny-aes)
