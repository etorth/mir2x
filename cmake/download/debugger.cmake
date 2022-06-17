MESSAGE(STATUS "download https://github.com/slembcke/debugger.lua")
IF(NOT EXISTS "${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua")
    FILE(DOWNLOAD "https://raw.githubusercontent.com/slembcke/debugger.lua/master/debugger.lua" "${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua")
ENDIF()

MESSAGE(STATUS "download debugger.lua, in ${MIR2X_3RD_PARTY_DIR}/download/debugger")

INSTALL(FILES ${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua DESTINATION server/script/3rdparty)
INSTALL(FILES ${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua DESTINATION client/script/3rdparty)
