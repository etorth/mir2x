MIR2X_LIST_SOURCE_RECURSIVE(CLIENT_SRCS)
ADD_EXECUTABLE(client ${CLIENT_SRCS})
ADD_DEPENDENCIES(client mir2x_3rds)

# can not use ExternalProject_Add() to add libpinyin as dependency
# libpinyin generates libpinyin.pc for pkg-config to configure its paths, and ExternalProject_Add() runs in build stage
# but PKG_CHECK_MODULES() is executed in configuration stage, it can not find libpinyin.pc in configuration stage

# by default, libpinyin is installed as shared libpinyin.so, which makes client depends libpinyin.so and path installed
# to build static libpinyin.a, uses
#
#     $ git clone https://github.com/libpinyin/libpinyin.git
#     $ cd libpinyin
#     $ ./autogen.sh --prefix=<your-libpinyin-install-path> --enable-shared=no
#     $ make && make install
#
# then build mir2x as:
#
#     $ cmake ../mir2x ... -DCMAKE_PREFIX_PATH=<your-libpinyin-install-path/lib/pkgconfig>
#
# here CMAKE_PREFIX_PATH helps to find libpinyin.pc, or by setup PKG_CONFIG_PATH
#
#     $ env PKG_CONFIG_PATH=<your-libpinyin-install-path/lib/pkgconfig>:${PKG_CONFIG_PATH} cmake ../mir2x ...

PKG_CHECK_MODULES(LIBPINYIN REQUIRED libpinyin)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# subdirs needs explicitly includes headers from client/src
# otherwise easy to cause issue for headers with same file name

# MIR2X_LIST_SUBDIRS(SRC_SUBDIRS)
# FOREACH(T_SUBDIR ${SRC_SUBDIRS})
#     TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${T_SUBDIR})
# ENDFOREACH()

LINK_DIRECTORIES(${LIBPINYIN_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${LIBPINYIN_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(client
    ${G3LOG_LIBRARIES}
    ${TINYXML2_LIBRARIES}
    ${CMAKE_DL_LIBS}

    common

    SDL2_ttf
    SDL2_gfx
    SDL2_image
    SDL2_mixer

    ${LUA_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${LZ4_LIBRARIES}
    ${ZSTD_LIBRARIES}
    ${LIBPINYIN_LIBRARIES}

    Threads::Threads)

IF(WIN32)
    TARGET_LINK_LIBRARIES(client SDL2::SDL2main SDL2::SDL2-static)
    TARGET_LINK_LIBRARIES(client ${FREETYPE_LIBRARIES})
ELSE()
    TARGET_LINK_LIBRARIES(client ${SDL2_LIBRARIES})
ENDIF()

INSTALL(TARGETS client DESTINATION client)
INSTALL(DIRECTORY ${LIBPINYIN_LIBDIR}/libpinyin DESTINATION client)
INSTALL(DIRECTORY ${MIR2X_DATA_REPO_PATH}/res DESTINATION client)
INSTALL(FILES ${MIR2X_DATA_REPO_PATH}/configuration.xml DESTINATION client)
