set(CLIENT_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
mir2x_list_source_recursive_filtered(CLIENT_OBJ_SRCS
    EXCLUDE ${CLIENT_MAIN_SRC})

# client_objs: everything the client is made of except main.cpp, as an OBJECT library so
# test executables under ../test/unit (e.g. test_xmltypeset) can reuse the exact same
# compiled objects instead of recompiling the whole client a second time
add_library(client_objs OBJECT ${CLIENT_OBJ_SRCS})
add_dependencies(client_objs zsdbmaker mir2x_resources)
target_include_directories(client_objs PUBLIC ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(client_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(client_objs PUBLIC
    common
    mir2x::sdl3_ext
    mir2x::libpinyin)

add_executable(client ${CLIENT_MAIN_SRC})
target_link_libraries(client PRIVATE client_objs)

install(TARGETS client DESTINATION client)
if(MIR2X_LIBPINYIN_DATA_DIR)
    install(DIRECTORY ${MIR2X_LIBPINYIN_DATA_DIR} DESTINATION client)
endif()
install(CODE "execute_process(
    COMMAND ${MIR2X_PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
            --install-client
            --ignore-zsdbmaker-error
            --zsdbmaker=$<TARGET_FILE:zsdbmaker>
            --input-dir=${MIR2X_RES_REPO_PATH}
            --output-dir=${CMAKE_INSTALL_PREFIX}/client/res
    COMMAND_ERROR_IS_FATAL ANY
)")
