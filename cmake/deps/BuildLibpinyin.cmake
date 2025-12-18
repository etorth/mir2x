IF(NOT MIR2X_BUILD_LIBPINYIN)
    RETURN()
ENDIF()

INCLUDE(FetchContent)

FetchContent_Declare(
    libdb

    GIT_REPOSITORY https://github.com/berkeleydb/libdb.git
    GIT_TAG        master

    GIT_PROGRESS 1
)

FetchContent_MakeAvailable(libdb)

IF(NOT DEFINED libdb_CONFIGURED)
    MESSAGE(STATUS "Configuring libdb")
    EXECUTE_PROCESS(
        COMMAND ${libdb_SOURCE_DIR}/dist/configure --prefix=${MIR2X_3RD_PARTY_DIR}/libdb/build --enable-shared=no
        WORKING_DIRECTORY ${libdb_BINARY_DIR}

        RESULT_VARIABLE libdb_CONFIGURED

        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )
ENDIF()

IF(NOT DEFINED libdb_INSTALLED)
    MESSAGE(STATUS "Building libdb")
    EXECUTE_PROCESS(
        COMMAND make install
        WORKING_DIRECTORY ${libdb_BINARY_DIR}

        RESULT_VARIABLE libdb_INSTALLED

        OUTPUT_QUIET
        ERROR_QUIET
    )
ENDIF()

FetchContent_Declare(
    libpinyin

    GIT_REPOSITORY https://github.com/libpinyin/libpinyin.git
    GIT_TAG        main

    GIT_PROGRESS 1
)

FetchContent_MakeAvailable(libpinyin)

IF(NOT DEFINED libpinyin_CONFIGURED)
    MESSAGE(STATUS "Configuring libpinyin")
    EXECUTE_PROCESS(
        COMMAND env CFLAGS=-I${MIR2X_3RD_PARTY_DIR}/libdb/build/include CXXFLAGS=-I${MIR2X_3RD_PARTY_DIR}/libdb/build/include LIBS=-L${MIR2X_3RD_PARTY_DIR}/libdb/build/lib ${libpinyin_SOURCE_DIR}/autogen.sh --prefix=${MIR2X_3RD_PARTY_DIR}/libpinyin/build --enable-shared=no
        WORKING_DIRECTORY ${libpinyin_SOURCE_DIR}

        RESULT_VARIABLE libpinyin_CONFIGURED

        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )
ENDIF()

IF(NOT DEFINED libpinyin_INSTALLED)
    MESSAGE(STATUS "Building libpinyin")
    EXECUTE_PROCESS(
        COMMAND make install
        WORKING_DIRECTORY ${libpinyin_SOURCE_DIR}

        RESULT_VARIABLE libpinyin_INSTALLED

        OUTPUT_QUIET
        ERROR_QUIET
    )
ENDIF()

SET(ENV{PKG_CONFIG_PATH} ${MIR2X_3RD_PARTY_DIR}/libpinyin/build/lib/pkgconfig:$ENV{PKG_CONFIG_PATH})
MESSAGE(STATUS "Update PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")
