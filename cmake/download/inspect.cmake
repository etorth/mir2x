message(STATUS "https://github.com/kikito/inspect.lua")
if(NOT EXISTS "${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua")
    file(DOWNLOAD "https://raw.githubusercontent.com/kikito/inspect.lua/master/inspect.lua" "${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua")
endif()

message(STATUS "downloaded inspect.lua, in ${MIR2X_3RD_PARTY_DIR}/download/inspect")

install(FILES ${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua DESTINATION server/script/3rdparty)
install(FILES ${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua DESTINATION client/script/3rdparty)
