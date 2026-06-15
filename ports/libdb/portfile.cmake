set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO berkeleydb/libdb
    REF eae0c147ed430872da5db993f5e920c8f41ecb31
    SHA512 27de98cacdff80a7055682a2b9d27d9ba7d531824379bf83659bc4eaab7214e2e208236fa6e8ec848571a469f15b10c9546dc0cc0d82e0d02cc19b6ced58602d
    HEAD_REF master
)

set(ENV{CFLAGS} "$ENV{CFLAGS} -Wno-incompatible-pointer-types")

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}/dist"
    OPTIONS
        --disable-shared
        --enable-static
)

vcpkg_install_make()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/docs")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/docs")
