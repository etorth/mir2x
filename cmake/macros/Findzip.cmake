# find zip (terminal input library) includes and library
#
# ZIP_INCLUDE_DIR - where the directory containing the ZIP headers can be found
# ZIP_LIBRARY     - full path to the ZIP library
# ZIP_FOUND       - TRUE if ZIP was found

FIND_PATH(ZIP_INCLUDE_DIR zip.h)
FIND_LIBRARY(ZIP_LIBRARY NAMES zip)

set(ZIP_FOUND 0)

if (ZIP_INCLUDE_DIR)
    if (ZIP_LIBRARY)
        set(ZIP_FOUND true)
        message(STATUS "Found zip library: ${ZIP_LIBRARY}")
        message(STATUS "Include dir is: ${ZIP_INCLUDE_DIR}")
        include_directories(${ZIP_INCLUDE_DIR})
    endif()
endif()

if (NOT ZIP_FOUND)
    message(FATAL_ERROR "** zip library not found!\n**")
endif()
