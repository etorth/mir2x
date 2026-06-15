message(STATUS "download https://github.com/slembcke/debugger.lua")
if(NOT EXISTS "${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua")
    file(DOWNLOAD "https://raw.githubusercontent.com/slembcke/debugger.lua/master/debugger.lua" "${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua")
endif()

message(STATUS "downloaded debugger.lua, in ${MIR2X_3RD_PARTY_DIR}/download/debugger")

install(FILES ${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua DESTINATION server/script/3rdparty)
install(FILES ${MIR2X_3RD_PARTY_DIR}/download/debugger/debugger.lua DESTINATION client/script/3rdparty)
