# find tinyxml2 (terminal input library) includes and library
#
# TINYXML2_INCLUDE_DIR - where the directory containing the TINYXML2 headers can be found
# TINYXML2_LIBRARY     - full path to the TINYXML2 library
# TINYXML2_FOUND       - TRUE if TINYXML2 was found

FIND_PATH(TINYXML2_INCLUDE_DIR tinyxml2.h)
FIND_LIBRARY(TINYXML2_LIBRARY NAMES tinyxml2)

set(TINYXML2_FOUND 0)

if (TINYXML2_INCLUDE_DIR)
    if (TINYXML2_LIBRARY)
        set(TINYXML2_FOUND true)
        message(STATUS "Found tinyxml2 library: ${TINYXML2_LIBRARY}")
        message(STATUS "Include dir is: ${TINYXML2_INCLUDE_DIR}")
        include_directories(${TINYXML2_INCLUDE_DIR})
    endif()
endif()

if (NOT TINYXML2_FOUND)
    message(FATAL_ERROR "** tinyxml2 library not found!\n**")
endif()
