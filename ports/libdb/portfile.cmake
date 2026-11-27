vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etorth/libdb
    REF 8bc35cf5d1023879d36fca9e775545eabb000138
    SHA512 34dee44693cde553af074081af8988872123e3b2deee71ada3a86e19ba2e3af8cf8e48aa4e596841a7c277ec991331b246b998899a2c3896f430f6ea1a362e23
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
