set(VCPKG_BUILD_TYPE release) # header-only

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/sol2
    REF a6872ef46b08704b9069ebf83161f4637459ce63 # current latest commit on develop
    SHA512 78d1d921eb6452475c180609785597afaa90b549517b0b53bea2c714982f9bedd19ec3fcdea0d701e88e043b802dadcfc76270fdd5c11fcaa973b6dec4dde7e8
    HEAD_REF develop
)

vcpkg_replace_string("${SOURCE_PATH}/CMakeLists.txt" "if (sol2-is-top-level-project)" "if (0)")
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSOL2_DOCS=OFF
        -DSOL2_EXAMPLES=OFF
        -DSOL2_SINGLE=OFF
        -DSOL2_TESTS=OFF
        -DSOL2_ENABLE_INSTALL=ON)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cmake/sol2)
vcpkg_fixup_pkgconfig()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
