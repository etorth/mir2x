MESSAGE(STATUS "download https://github.com/etorth/large_file_splitter")
IF(NOT EXISTS "${MIR2X_3RD_PARTY_DIR}/download/large_file_splitter/large_file_splitter.py")
    FILE(DOWNLOAD "https://raw.githubusercontent.com/etorth/large_file_splitter/main/large_file_splitter.py" "${MIR2X_3RD_PARTY_DIR}/download/large_file_splitter/large_file_splitter.py")
ENDIF()

MESSAGE(STATUS "downloaded large_file_splitter.py, in ${MIR2X_3RD_PARTY_DIR}/download/large_file_splitter")
