# find g3log (terminal input library) includes and library
#
# G3LOG_INCLUDE_DIR - where the directory containing the G3LOG headers can be found
# G3LOG_LIBRARY     - full path to the G3LOG library
# G3LOG_FOUND       - TRUE if G3LOG was found

FIND_PATH(G3LOG_INCLUDE_DIR g3log)
FIND_LIBRARY(G3LOG_LIBRARY NAMES g3logger)

set(G3LOG_FOUND 0)

if (G3LOG_INCLUDE_DIR)
    if (G3LOG_LIBRARY)
        set(G3LOG_FOUND true)
        message(STATUS "Found g3log library: ${G3LOG_LIBRARY}")
        message(STATUS "Include dir is: ${G3LOG_INCLUDE_DIR}")
        include_directories(${G3LOG_INCLUDE_DIR})
    endif()
endif()

if (NOT G3LOG_FOUND)
    message(FATAL_ERROR "** g3log library not found!\n**")
endif()
