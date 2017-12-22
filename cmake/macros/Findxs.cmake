# find libxs (terminal input library) includes and library
#
# XS_INCLUDE_DIR - where the directory containing the XS headers can be found
# XS_LIBRARY     - full path to the XS library
# XS_FOUND       - TRUE if XS was found

FIND_PATH(XS_INCLUDE_DIR xs.h)
FIND_LIBRARY(XS_LIBRARY NAMES xs)

set(XS_FOUND 0)

if (XS_INCLUDE_DIR)
    if (XS_LIBRARY)
        set(XS_FOUND true)
        message(STATUS "Found libxs library: ${XS_LIBRARY}")
        message(STATUS "Include dir is: ${XS_INCLUDE_DIR}")
        include_directories(${XS_INCLUDE_DIR})
    endif()
endif()

if (NOT XS_FOUND)
    message(FATAL_ERROR "** libxs library not found!\n**")
endif()
