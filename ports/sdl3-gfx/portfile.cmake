vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/SDL3_gfx
    REF 7db6b5f4dad6859394ac99554306e18542ba076c
    SHA512 d1dd2b1e97d9b2ab01788f69dc1838c5f09abe77d4197b83ba7ec9f42fce7ac748a368a20362572f996629cf8f0ebda479604cb23bdc3ffa8dddcf64022442c7
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
