MESSAGE(STATUS "https://github.com/kikito/inspect.lua")
IF(NOT EXISTS "${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua")
    FILE(DOWNLOAD "https://raw.githubusercontent.com/kikito/inspect.lua/master/inspect.lua" "${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua")
ENDIF()

MESSAGE(STATUS "downloaded inspect.lua, in ${MIR2X_3RD_PARTY_DIR}/download/inspect")

INSTALL(FILES ${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua DESTINATION server/script/3rdparty)
INSTALL(FILES ${MIR2X_3RD_PARTY_DIR}/download/inspect/inspect.lua DESTINATION client/script/3rdparty)
