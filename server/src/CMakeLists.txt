set(FLTK_HPP_SRC "")
set(FLTK_CPP_SRC "")
fltk_parse_fl_file(FLTK_HPP_SRC FLTK_CPP_SRC)

set(SERVER_MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
mir2x_list_source_recursive_filtered(SERVER_OBJ_SRCS
    EXCLUDE ${SERVER_MAIN_SRC})

add_library(server_objs OBJECT ${SERVER_OBJ_SRCS} ${FLTK_CPP_SRC})
add_dependencies(server_objs zsdbmaker mir2x_resources)
target_include_directories(server_objs PUBLIC ${MIR2X_COMMON_SOURCE_DIR})
target_include_directories(server_objs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(server_objs PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(server_objs PUBLIC
    common
    mir2x::sqlitecpp)

add_executable(server ${SERVER_MAIN_SRC})
target_link_libraries(server PRIVATE server_objs)

install(TARGETS server DESTINATION server)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/server/script DESTINATION server)
install(CODE "execute_process(
    COMMAND ${MIR2X_PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/installres/src/install_res.py
            --install-server
            --ignore-zsdbmaker-error
            --zsdbmaker=$<TARGET_FILE:zsdbmaker>
            --input-dir=${MIR2X_RES_REPO_PATH}
            --output-dir=${CMAKE_INSTALL_PREFIX}/server
    COMMAND_ERROR_IS_FATAL ANY
)")
