# find libpng (terminal input library) includes and library
#
# LIBPNG_INCLUDE_DIR - where the directory containing the LIBPNG headers can be found
# LIBPNG_LIBRARY     - full path to the LIBPNG library
# LIBPNG_FOUND       - TRUE if LIBPNG was found

FIND_PATH(LIBPNG_INCLUDE_DIR png.h)
FIND_LIBRARY(LIBPNG_LIBRARY NAMES png)

set(LIBPNG_FOUND 0)

if (LIBPNG_INCLUDE_DIR)
    if (LIBPNG_LIBRARY)
        set(LIBPNG_FOUND true)
        message(STATUS "Found libpng library: ${LIBPNG_LIBRARY}")
        message(STATUS "Include dir is: ${LIBPNG_INCLUDE_DIR}")
        include_directories(${LIBPNG_INCLUDE_DIR})
    endif()
endif()

if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "** libpng library not found!\n**")
endif()
