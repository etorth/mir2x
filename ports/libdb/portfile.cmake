vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/libdb
    REF 5e3264915ef0b4f77e82620034e785e606ae3965
    SHA512 19241b5bea3ff300e6ae6c1f2c1ca362061d4b1026ce51cf74e238a414045d0a432848ffd15ac374986f8ba03c9879e2e9748f25ee6cf07771ed7d8b02e2aee9
    HEAD_REF main
)

file(CHMOD
    "${SOURCE_PATH}/dist/configure"
    "${SOURCE_PATH}/dist/config.guess"
    "${SOURCE_PATH}/dist/config.sub"
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

set(LIBDB_CFLAGS "\${CFLAGS} -Wno-incompatible-pointer-types")
set(LIBDB_CXXFLAGS "\${CXXFLAGS}")
set(LIBDB_LDFLAGS "\${LDFLAGS}")
set(LIBDB_BUILD_OPTIONS)

set(configure_options
    --disable-shared
    --enable-static
    --disable-compat185
    --disable-compression
    --disable-cxx
    --disable-dbm
    --disable-heap
    --disable-java
    --disable-partition
    --disable-queue
    --disable-replication
    --disable-rpath
    --disable-sql
    --disable-statistics
    --disable-stl
    --disable-tcl
    --disable-verify
)

if(VCPKG_TARGET_IS_MINGW)
    string(APPEND LIBDB_CFLAGS " -DUNICODE -D_UNICODE")
    string(APPEND LIBDB_CXXFLAGS " -DUNICODE -D_UNICODE")
    string(APPEND LIBDB_LDFLAGS " -lpthread")
    list(APPEND LIBDB_BUILD_OPTIONS LIBSO_LIBS=-lpthread)
    list(APPEND configure_options
        --enable-mingw
        ac_cv_func_time=yes
        ac_cv_func_localtime=yes
    )
endif()

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}"
    PROJECT_SUBPATH dist
    OPTIONS
        ${configure_options}
        "CFLAGS=${LIBDB_CFLAGS}"
        "CXXFLAGS=${LIBDB_CXXFLAGS}"
        "LDFLAGS=${LIBDB_LDFLAGS}"
)

vcpkg_install_make(OPTIONS ${LIBDB_BUILD_OPTIONS})

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/bin"
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/docs"
    "${CURRENT_PACKAGES_DIR}/share/doc"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
