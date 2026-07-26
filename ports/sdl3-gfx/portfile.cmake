vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/SDL3_gfx
    REF b611962ae4112cb5cd7e9b9d84b3f824e1688206
    SHA512 abac6703041f4a43ae89e00184dc58670425a60367505d09b798bbd34e79ca0339506ff70cf21bd337d18c0c2f0cfa4333773c6a5022e68c62ca93aeeadcc921
    HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" SDL3_GFX_BUILD_SHARED)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_SHARED_LIBS=${SDL3_GFX_BUILD_SHARED}
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME SDL3_gfx CONFIG_PATH lib/cmake/SDL3_gfx)
vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/COPYING"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
