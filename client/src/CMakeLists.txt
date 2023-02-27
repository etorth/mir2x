AUX_SOURCE_DIRECTORY(. CLIENT_SRC)
ADD_EXECUTABLE(client ${CLIENT_SRC})
ADD_DEPENDENCIES(client mir2x_3rds)

# can not use ExternalProject_Add() to add libpinyin as dependency
# libpinyin generates libpinyin.pc for pkg-config to configure its paths, and ExternalProject_Add() runs in build stage
# but PKG_CHECK_MODULES() is executed in configuration stage, it can not find libpinyin.pc in configuration stage

# by default, libpinyin gets installed as shared libpinyin.so, to build static libpinyin.a, uses
#
#     $ git clone https://github.com/libpinyin/libpinyin.git
#     $ cd libpinyin
#     $ ./autogen.sh --prefix=/usr --enable-shared=no
#     $ sudo make install

PKG_CHECK_MODULES(LIBPINYIN REQUIRED libpinyin)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${MIR2X_COMMON_SOURCE_DIR})

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
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/client/bin/res DESTINATION client)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/client/bin/configuration.xml DESTINATION client)
