vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libpinyin/libpinyin
    REF 88e39fccf0eb457cd5bed35103692502e12618aa
    SHA512 13c3d555236032216b0c5e4cb4e3fa190257eeb970274087a40ded31b232221b18780b8a010c1e59146968a4e5d3a0499c0f8ed194d122add488421d351c307f
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

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_TESTING=OFF
        -DBUILD_UTILS=ON
        -DSHARE_INSTALL_PREFIX=lib
        "-DDB_INCLUDE_DIR=${CURRENT_INSTALLED_DIR}/include"
        "-DDB_LIBRARY=${CURRENT_INSTALLED_DIR}/lib/libdb.a"
)

vcpkg_cmake_install()

foreach(build_suffix IN ITEMS rel dbg)
    set(generated_data_dir "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${build_suffix}/data")
    set(build_library_dir "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${build_suffix}/src")
    if(build_suffix STREQUAL "dbg")
        set(package_suffix debug)
        set(package_data_dir "${CURRENT_PACKAGES_DIR}/${package_suffix}/lib/libpinyin/data")
        set(package_library_dir "${CURRENT_PACKAGES_DIR}/${package_suffix}/lib")
    else()
        set(package_data_dir "${CURRENT_PACKAGES_DIR}/lib/libpinyin/data")
        set(package_library_dir "${CURRENT_PACKAGES_DIR}/lib")
    endif()

    if(EXISTS "${generated_data_dir}/table.conf")
        file(COPY "${generated_data_dir}/" DESTINATION "${package_data_dir}")
    endif()

    foreach(internal_library IN ITEMS lookup storage)
        set(internal_archive "${build_library_dir}/${internal_library}/lib${internal_library}.a")
        if(NOT EXISTS "${internal_archive}")
            message(FATAL_ERROR "Could not find libpinyin internal static library '${internal_archive}'.")
        endif()
        file(INSTALL "${internal_archive}" DESTINATION "${package_library_dir}")
    endforeach()
endforeach()

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/share/doc"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
