INCLUDE(ExternalProject)

ExternalProject_Add(
    libpinyin

    GIT_REPOSITORY "https://github.com/etorth/libpinyin.git"
    GIT_TAG        "main"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/libpinyin"
    INSTALL_DIR "${MIR2X_3RD_PARTY_DIR}/libpinyin/build"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${MIR2X_3RD_PARTY_DIR}/libpinyin/build/install -DBUILD_SHARED_LIBS=OFF

    LOG_BUILD 1
    LOG_CONFIGURE 1
    LOG_INSTALL 1
)

# can not export LIBPINYIN_VERSION from ExternalProject_Add command
# hack by hard-coding here

SET(LIBPINYIN_VERSION "2.1.0")
SET(LIBPINYIN_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/libpinyin/build/install/include/libpinyin-${LIBPINYIN_VERSION}")

IF(WIN32)
    SET(LIBPINYIN_LIBRARIES pinyin_static)
ELSE()
    SET(LIBPINYIN_LIBRARIES "${CMAKE_STATIC_LIBRARY_PREFIX}pinyin${CMAKE_STATIC_LIBRARY_SUFFIX}")
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM ${LIBPINYIN_INCLUDE_DIRS})
LINK_DIRECTORIES(${MIR2X_3RD_PARTY_DIR}/libpinyin/build/install/lib)
ADD_DEPENDENCIES(mir2x_3rds libpinyin)
