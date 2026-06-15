set(MIR2X_INSPECT_LUA_URL "https://raw.githubusercontent.com/kikito/inspect.lua/master/inspect.lua")
set(MIR2X_INSPECT_LUA_DIR "${MIR2X_3RD_PARTY_DIR}/download/inspect")
set(MIR2X_INSPECT_LUA "${MIR2X_INSPECT_LUA_DIR}/inspect.lua")

if(NOT EXISTS "${MIR2X_INSPECT_LUA}")
    file(MAKE_DIRECTORY "${MIR2X_INSPECT_LUA_DIR}")
    message(STATUS "Downloading inspect.lua from ${MIR2X_INSPECT_LUA_URL}")
    file(DOWNLOAD
        "${MIR2X_INSPECT_LUA_URL}"
        "${MIR2X_INSPECT_LUA}"
        STATUS MIR2X_INSPECT_LUA_DOWNLOAD_STATUS
        TLS_VERIFY ON)
    list(GET MIR2X_INSPECT_LUA_DOWNLOAD_STATUS 0 MIR2X_INSPECT_LUA_DOWNLOAD_CODE)
    list(GET MIR2X_INSPECT_LUA_DOWNLOAD_STATUS 1 MIR2X_INSPECT_LUA_DOWNLOAD_MESSAGE)
    if(NOT MIR2X_INSPECT_LUA_DOWNLOAD_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download inspect.lua: ${MIR2X_INSPECT_LUA_DOWNLOAD_MESSAGE}")
    endif()
endif()

message(STATUS "Using inspect.lua: ${MIR2X_INSPECT_LUA}")

install(FILES "${MIR2X_INSPECT_LUA}" DESTINATION server/script/3rdparty)
install(FILES "${MIR2X_INSPECT_LUA}" DESTINATION client/script/3rdparty)
