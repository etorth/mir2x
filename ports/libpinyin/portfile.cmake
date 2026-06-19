vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/libpinyin
    REF 91f2bf3541f71fc6e27e58c077152f6b81ea5edf
    SHA512 595b1db787fbcf6bca9aaff21e96713db1a582d479ba335f9819ac08ceffde7295cc25ebc5850c47159bd0ca51c1c684a9df205dccfcc80f3e1c5d53151eb2d9
    HEAD_REF main
)

vcpkg_download_distfile(MODEL_ARCHIVE
    URLS "https://downloads.sourceforge.net/project/libpinyin/models/model20.text.tar.gz"
    FILENAME "libpinyin-model20.text.tar.gz"
    SHA512 ed4d0607ad35e0e7ea424670539ddcd81a2b03c1da914b9c00cb748cf065f29471502d40b9a189852001da1fb9178c3bcc4675d7efebea5d081d78bfeee9b5d6
)

vcpkg_extract_source_archive(MODEL_SOURCE_PATH
    ARCHIVE "${MODEL_ARCHIVE}"
    SOURCE_BASE "model20"
    NO_REMOVE_ONE_LEVEL
)
file(COPY "${MODEL_SOURCE_PATH}/" DESTINATION "${SOURCE_PATH}/data")

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}"
    AUTOCONFIG
    OPTIONS
        --with-dbm=BerkeleyDB
        --disable-libzhuyin
        --disable-dependency-tracking
)

vcpkg_install_make()

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/share/doc"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
