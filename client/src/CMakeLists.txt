MIR2X_LIST_SOURCE_RECURSIVE(CLIENT_SRCS)
ADD_EXECUTABLE(client ${CLIENT_SRCS})
ADD_DEPENDENCIES(client zsdbmaker)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(client
    common
    mir2x::sdl2_ext
    mir2x::libpinyin)

INSTALL(TARGETS client DESTINATION client)
IF(MIR2X_LIBPINYIN_DATA_DIR)
    INSTALL(DIRECTORY ${MIR2X_LIBPINYIN_DATA_DIR} DESTINATION client)
ENDIF()
IF(MIR2X_RES_REPO_PATH)
    INSTALL(CODE "EXECUTE_PROCESS(
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
                --install-client
                --ignore-zsdbmaker-error
                --zsdbmaker=$<TARGET_FILE:zsdbmaker>
                --input-dir=${MIR2X_RES_REPO_PATH}
                --output-dir=${CMAKE_INSTALL_PREFIX}/client/res
        COMMAND_ERROR_IS_FATAL ANY
    )")
ENDIF()
