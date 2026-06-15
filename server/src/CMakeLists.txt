SET(FLTK_HPP_SRC "")
SET(FLTK_CPP_SRC "")
FLTK_PARSE_FL_FILE(FLTK_HPP_SRC FLTK_CPP_SRC)

AUX_SOURCE_DIRECTORY(. SERVER_SRC)
ADD_EXECUTABLE(server ${SERVER_SRC} ${FLTK_CPP_SRC})
ADD_DEPENDENCIES(server zsdbmaker)

TARGET_INCLUDE_DIRECTORIES(server PRIVATE ${MIR2X_COMMON_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ${CMAKE_CURRENT_LIST_DIR})

TARGET_LINK_LIBRARIES(server common mir2x::sqlitecpp)

INSTALL(TARGETS server DESTINATION server)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/server/script DESTINATION server)
IF(MIR2X_RES_REPO_PATH)
    INSTALL(CODE "EXECUTE_PROCESS(
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
                --install-server
                --ignore-zsdbmaker-error
                --zsdbmaker=$<TARGET_FILE:zsdbmaker>
                --input-dir=${MIR2X_RES_REPO_PATH}
                --output-dir=${CMAKE_INSTALL_PREFIX}/server
        COMMAND_ERROR_IS_FATAL ANY
    )")
ENDIF()
