# find zip (terminal input library) includes and library
#
# ASIO_INCLUDE_DIR - where the directory containing the ASIO headers can be found
# ASIO_LIBRARY     - full path to the ASIO library
# ASIO_FOUND       - TRUE if ASIO was found

FIND_PATH(ASIO_INCLUDE_DIR asio)

set(ASIO_FOUND 0)

if (ASIO_INCLUDE_DIR)
    set(ASIO_FOUND true)
    message(STATUS "Found asio library: ${ASIO_LIBRARY}")
    message(STATUS "Include dir is: ${ASIO_INCLUDE_DIR}")
    include_directories(${ASIO_INCLUDE_DIR})
endif()

if (NOT ASIO_FOUND)
    message(FATAL_ERROR "** asio library not found!\n**")
endif()
