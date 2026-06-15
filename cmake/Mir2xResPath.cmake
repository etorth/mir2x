set(MIR2X_RES_REPO_URL "https://github.com/etorth/mir2x_res.git" CACHE STRING "mir2x resource repository URL")
set(MIR2X_RES_REPO_PATH "" CACHE PATH "Path to the mir2x resource repository used by install-time resource packing")

set(MIR2X_DEFAULT_RES_REPO_PATH "${CMAKE_BINARY_DIR}/assets/mir2x_res")

# Empty means the user owns MIR2X_RES_REPO_PATH; non-empty means CMake can clone it.
set(MIR2X_RES_REPO_MANAGED_PATH "" CACHE INTERNAL "CMake-managed mir2x resource repository path")

if(DEFINED MIR2X_RES_REPO_PATH_IS_DEFAULT)
    if(MIR2X_RES_REPO_PATH_IS_DEFAULT)
        set(MIR2X_RES_REPO_MANAGED_PATH "${MIR2X_RES_REPO_PATH}" CACHE INTERNAL "CMake-managed mir2x resource repository path" FORCE)
    endif()
    unset(MIR2X_RES_REPO_PATH_IS_DEFAULT CACHE)
endif()

if(NOT MIR2X_RES_REPO_PATH)
    set(MIR2X_RES_REPO_PATH "${MIR2X_DEFAULT_RES_REPO_PATH}" CACHE PATH "Path to the mir2x resource repository used by install-time resource packing" FORCE)
    set(MIR2X_RES_REPO_MANAGED_PATH "${MIR2X_RES_REPO_PATH}" CACHE INTERNAL "CMake-managed mir2x resource repository path" FORCE)
elseif(MIR2X_RES_REPO_MANAGED_PATH AND NOT "${MIR2X_RES_REPO_PATH}" STREQUAL "${MIR2X_RES_REPO_MANAGED_PATH}")
    set(MIR2X_RES_REPO_MANAGED_PATH "" CACHE INTERNAL "CMake-managed mir2x resource repository path" FORCE)
endif()

if(IS_DIRECTORY "${MIR2X_RES_REPO_PATH}")
    add_custom_target(mir2x_resources)
    message(STATUS "Using mir2x resource path: ${MIR2X_RES_REPO_PATH}")
elseif(MIR2X_RES_REPO_MANAGED_PATH)
    find_program(GIT_EXECUTABLE git REQUIRED)
    get_filename_component(MIR2X_RES_REPO_PARENT_DIR "${MIR2X_RES_REPO_PATH}" DIRECTORY)
    get_filename_component(MIR2X_RES_REPO_DIR_NAME "${MIR2X_RES_REPO_PATH}" NAME)
    file(MAKE_DIRECTORY "${MIR2X_RES_REPO_PARENT_DIR}")
    add_custom_command(
        OUTPUT "${MIR2X_RES_REPO_PATH}/.git/HEAD"
        COMMAND ${CMAKE_COMMAND} -E echo "WARNING: cloning mir2x resources can take a very long time; your build tool may hide git clone progress."
        COMMAND ${GIT_EXECUTABLE} clone ${MIR2X_RES_REPO_URL} ${MIR2X_RES_REPO_DIR_NAME}
        WORKING_DIRECTORY "${MIR2X_RES_REPO_PARENT_DIR}"
        VERBATIM)
    add_custom_target(mir2x_resources DEPENDS "${MIR2X_RES_REPO_PATH}/.git/HEAD")
    message(STATUS "mir2x resource path will be cloned at build time: ${MIR2X_RES_REPO_PATH}")
else()
    message(FATAL_ERROR "Invalid MIR2X_RES_REPO_PATH: ${MIR2X_RES_REPO_PATH}")
endif()
