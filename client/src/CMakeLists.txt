mir2x_list_source_recursive(CLIENT_SRCS)
add_executable(client ${CLIENT_SRCS})
add_dependencies(client zsdbmaker mir2x_resources)
target_include_directories(client PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(client
    common
    mir2x::sdl3_ext
    mir2x::libpinyin)

install(TARGETS client DESTINATION client)
if(MIR2X_LIBPINYIN_DATA_DIR)
    install(DIRECTORY ${MIR2X_LIBPINYIN_DATA_DIR} DESTINATION client)
endif()
install(CODE "execute_process(
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
            --install-client
            --ignore-zsdbmaker-error
            --zsdbmaker=$<TARGET_FILE:zsdbmaker>
            --input-dir=${MIR2X_RES_REPO_PATH}
            --output-dir=${CMAKE_INSTALL_PREFIX}/client/res
    COMMAND_ERROR_IS_FATAL ANY
)")
