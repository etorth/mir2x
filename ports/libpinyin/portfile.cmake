vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libpinyin/libpinyin
    REF "${VERSION}"
    SHA512 3f4c217c9962cb80057edea50314b152f050290c2dbbb25bcefd833d54f934a587cf0ef00c0e857065e08f4d5c91bd8dd18b9f18aeb93d8cdfb072e779cc5928
    HEAD_REF main
)

vcpkg_download_distfile(MODEL_ARCHIVE
    URLS "https://downloads.sourceforge.net/libpinyin/models/model20.text.tar.gz"
    FILENAME "libpinyin-model20.text.tar.gz"
    SHA512 ed4d0607ad35e0e7ea424670539ddcd81a2b03c1da914b9c00cb748cf065f29471502d40b9a189852001da1fb9178c3bcc4675d7efebea5d081d78bfeee9b5d6
)

file(MAKE_DIRECTORY "${SOURCE_PATH}/data")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xzf "${MODEL_ARCHIVE}"
    WORKING_DIRECTORY "${SOURCE_PATH}/data"
    COMMAND_ERROR_IS_FATAL ANY
)

set(ENV{CPPFLAGS} "-I${CURRENT_INSTALLED_DIR}/include $ENV{CPPFLAGS}")
set(ENV{LDFLAGS} "-L${CURRENT_INSTALLED_DIR}/lib $ENV{LDFLAGS}")
set(ENV{PKG_CONFIG_PATH} "${CURRENT_INSTALLED_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}"
    AUTOCONFIG
    USE_WRAPPERS
    OPTIONS
        --disable-shared
        --enable-static
        --with-dbm=BerkeleyDB
)

vcpkg_install_make()
vcpkg_fixup_pkgconfig()

foreach(LIBPINYIN_PC IN ITEMS
    "${CURRENT_PACKAGES_DIR}/lib/pkgconfig/libpinyin.pc"
    "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/libpinyin.pc")
    if(EXISTS "${LIBPINYIN_PC}")
        vcpkg_replace_string("${LIBPINYIN_PC}"
            [[Libs: "-L${libdir}" -lpinyin]]
            [[Libs: "-L${libdir}" -lpinyin
Libs.private: -ldb]])
    endif()
endforeach()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/libpinyin")
file(INSTALL "${SOURCE_PATH}/COPYING"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
