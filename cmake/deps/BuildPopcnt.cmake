INCLUDE(ExternalProject)

ExternalProject_Add(
    libpopcnt

    GIT_REPOSITORY "https://github.com/kimwalisch/libpopcnt.git"
    GIT_TAG        "master"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/libpopcnt"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/libpopcnt/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/libpopcnt/build/install

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

SET(LIBPOPCNT_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/libpopcnt/build/install/include")
INCLUDE_DIRECTORIES(SYSTEM ${LIBPOPCNT_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds libpopcnt)
