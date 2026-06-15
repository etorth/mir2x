get_property(MIR2X_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE)
if(I_AM_BUILD_PY)
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES I_AM_BUILD_PY)
    list(REMOVE_DUPLICATES CMAKE_TRY_COMPILE_PLATFORM_VARIABLES)
endif()

if(NOT I_AM_BUILD_PY AND NOT MIR2X_IN_TRY_COMPILE)
    message(FATAL_ERROR "Use build.py to start the build")
endif()

if(NOT ((DEFINED ENV{CC} AND NOT "$ENV{CC}" STREQUAL "") OR (DEFINED ENV{CXX} AND NOT "$ENV{CXX}" STREQUAL "")))
    message(FATAL_ERROR "Mir2xVcpkgChainload.cmake requires build.py to provide CC or CXX.")
endif()

if(DEFINED ENV{CC} AND NOT "$ENV{CC}" STREQUAL "")
    set(CMAKE_C_COMPILER "$ENV{CC}" CACHE FILEPATH "C compiler selected by build.py" FORCE)
endif()

if(DEFINED ENV{CXX} AND NOT "$ENV{CXX}" STREQUAL "")
    set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE FILEPATH "C++ compiler selected by build.py" FORCE)
endif()
