# find mariadb (terminal input library) includes and library
#
# MARIADB_INCLUDE_DIR - where the directory containing the MARIADB headers can be found
# MARIADB_LIBRARY     - full path to the MARIADB library
# MARIADB_FOUND       - TRUE if MARIADB was found

FIND_PATH(MARIADB_INCLUDE_DIR mysql.h
    /usr/local/include/mariadb
    /usr/include/mariadb
)

FIND_LIBRARY(MARIADB_LIBRARY NAMES mariadb)

set(MARIADB_FOUND 0)

if (MARIADB_INCLUDE_DIR)
    if (MARIADB_LIBRARY)
        set(MARIADB_FOUND true)
        message(STATUS "Found mariadb library: ${MARIADB_LIBRARY}")
        message(STATUS "Include dir is: ${MARIADB_INCLUDE_DIR}")
        include_directories(${MARIADB_INCLUDE_DIR})
    endif()
endif()

if (NOT MARIADB_FOUND)
    message(FATAL_ERROR "** mariadb library not found!\n**")
endif()
