INCLUDE(ExternalProject)

ExternalProject_Add(
    mir2x_data

    GIT_REPOSITORY "https://github.com/etorth/mir2x_data.git"
    GIT_TAG        "main"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/mir2x_data"
)

SET(MIR2X_DATA_REPO_PATH "${MIR2X_3RD_PARTY_DIR}/mir2x_data")
ADD_DEPENDENCIES(mir2x_3rds mir2x_data)
