#
# FindMariaDB.cmake
#
# Try to find the include directory

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(PFILES $ENV{ProgramW6432})   
ELSE()
  SET(PFILES $ENV{ProgramFiles})   
ENDIF()

IF (NOT MARIADB_INCLUDE_DIR)
  IF(WIN32)
    FIND_PATH(MARIADB_INCLUDE_DIR
        NAMES
            mysql.h
        PATHS
            $ENV{MARIADB_INCLUDE_DIR}
            $ENV{MARIADB_DIR}/include
            $ENV{MARIADB_DIR}/include/mariadb
            "C:/Tools/vcpkg/installed/x64-windows/include"
            "C:/Tools/vcpkg/installed/*/include"
            ${PFILES}/MariaDB/*/include
         DOC
            "Specify the directory containing mysql.h")
  ELSE()
    FIND_PATH(MARIADB_BIN_DIR mariadb_config
              $ENV{MARIADB_DIR}/bin
              ${MARIADB_DIR}/bin)
    IF(MARIADB_BIN_DIR)
      EXEC_PROGRAM(${MARIADB_BIN_DIR}/mariadb_config 
                   ARGS "--include"
                   OUTPUT_VARIABLE MARIADB_INCLUDE_DIR)
      ADD_DEFINITIONS(${MARIADB_INCLUDE_DIR})
    ELSE()
      FIND_PATH(MARIADB_INCLUDE_DIR mysql.h
        $ENV{MARIADB_INCLUDE_DIR}
        $ENV{MARIADB_DIR}/include
        $ENV{MARIADB_DIR}/include/mariadb)
    ENDIF()
  ENDIF()
ENDIF()

IF(MARIADB_INCLUDE_DIR)
  MESSAGE(STATUS "Found MariaDB includes: ${MARIADB_INCLUDE_DIR}")
ENDIF()

IF(NOT MARIADB_LIBRARY_DIR)
  IF(WIN32)
  # Try to find mariadb client libraries
    FIND_PATH(MARIADB_LIBRARY_DIR ${MARIADB_CLIENT_FILENAME}
        $ENV{MARIADB_LIBRARY}
        ${PFILES}/MariaDB/*/lib
        C:/Tools/vcpkg/installed/*/lib
        $ENV{MARIADB_DIR}/lib/mariadb
        $ENV{MARIADB_DIR}/lib
        $ENV{MARIADB_DIR}/libmariadb)

    IF(MARIADB_LIBRARY)
      GET_FILENAME_COMPONENT(MARIADB_LIBRARY_DIR ${MARIADB_LIBRARY} PATH)
    ENDIF()

  ELSE()
    FIND_PATH(MARIADB_BIN_DIR mariadb_config
              $ENV{MARIADB_DIR}/bin
              ${MARIADB_DIR}/bin)
    IF(MARIADB_BIN_DIR)
      EXEC_PROGRAM(${MARIADB_BIN_DIR}/mariadb_config 
                   ARGS "--libs"
                   OUTPUT_VARIABLE MARIADB_LIBRARY_DIR)
      # since we use the static library we need the directory part only

      # TODO
      # need to parse the lib name
      SET(MARIADB_LIBRARIES "mariadb")

      STRING(SUBSTRING ${MARIADB_LIBRARY_DIR} 2 -1 MARIADB_LIBRARY_DIR)
      STRING(FIND ${MARIADB_LIBRARY_DIR} " -l" MY_LENGTH)
      STRING(SUBSTRING ${MARIADB_LIBRARY_DIR} 0 ${MY_LENGTH} MARIADB_LIBRARY_DIR)

    ENDIF()
  ENDIF()
ENDIF()

IF(MARIADB_LIBRARY_DIR AND MARIADB_INCLUDE_DIR)
  MESSAGE(STATUS "Found MariaDB libraries: ${MARIADB_LIBRARY_DIR}")
  SET(MARIADB_FOUND TRUE)
ELSE()
  MESSAGE(STATUS "MariaDB not found. Includes: ${MARIADB_INCLUDE_DIR}, Libs: ${MARIADB_LIBRARY_DIR}")
ENDIF()
