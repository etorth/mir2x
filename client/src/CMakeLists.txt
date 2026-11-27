set(CLIENT_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
mir2x_list_source_recursive_filtered(CLIENT_OBJ_SRCS
    EXCLUDE ${CLIENT_MAIN_SRC})

# client_objs contains the client implementation without main.cpp so unit tests can reuse it.
add_library(client_objs OBJECT ${CLIENT_OBJ_SRCS})
target_include_directories(client_objs PUBLIC ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(client_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(client_objs PUBLIC
    common
    mir2x::sdl3_ext)

if(MIR2X_DESKTOP_BUILD)
    add_dependencies(client_objs zsdbmaker mir2x_resources)
endif()

if(MIR2X_ENABLE_PINYIN_IME)
    target_compile_definitions(client_objs PRIVATE MIR2X_ENABLE_PINYIN_IME)
    target_link_libraries(client_objs PUBLIC mir2x::libpinyin)
endif()

if(ANDROID)
    # SDLActivity loads libmain.so and calls SDL_main from it
    add_library(client SHARED ${CLIENT_MAIN_SRC})
    set_target_properties(client PROPERTIES OUTPUT_NAME main)
else()
    add_executable(client ${CLIENT_MAIN_SRC})
endif()
target_link_libraries(client PRIVATE client_objs)

if(ANDROID)
    # the APK has no install step, pack the client resources at build time
    # the Android project puts ${MIR2X_ANDROID_ASSETS_DIR} into the APK assets
    set(MIR2X_ANDROID_ASSETS_DIR ${CMAKE_BINARY_DIR}/android-assets CACHE PATH "Directory packed into the APK assets")
    add_custom_target(client_res ALL
        COMMAND ${MIR2X_PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
                --install-client
                --ignore-zsdbmaker-error
                --zsdbmaker=$<TARGET_FILE:zsdbmaker>
                --input-dir=${MIR2X_ASSETS_REPO_PATH}
                --output-dir=${MIR2X_ANDROID_ASSETS_DIR}/res
                ${MIR2X_ASSETS_VERSION_ARG}
        USES_TERMINAL
        VERBATIM)
    add_dependencies(client_res zsdbmaker_host mir2x_resources)
    return()
endif()

install(TARGETS client DESTINATION client)
if(MIR2X_ENABLE_PINYIN_IME AND MIR2X_LIBPINYIN_DATA_DIR)
    install(DIRECTORY ${MIR2X_LIBPINYIN_DATA_DIR} DESTINATION client)
endif()
install(CODE "execute_process(
    COMMAND ${MIR2X_PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
            --install-client
            --ignore-zsdbmaker-error
            --zsdbmaker=$<TARGET_FILE:zsdbmaker>
            --input-dir=${MIR2X_ASSETS_REPO_PATH}
            --output-dir=${CMAKE_INSTALL_PREFIX}/client/res
            ${MIR2X_ASSETS_VERSION_ARG}
    COMMAND_ERROR_IS_FATAL ANY
)")
