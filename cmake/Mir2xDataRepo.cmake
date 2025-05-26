INCLUDE(ExternalProject)

ExternalProject_Add(
    mir2x_data

    GIT_REPOSITORY "https://github.com/etorth/mir2x_data.git"
    GIT_TAG        "main"

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/mir2x_data"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

ExternalProject_Add_Step(
    mir2x_data
    requires_git_lfs_and_long_time_download_warning

    COMMAND ""
    COMMENT "Repository 'mir2x_data' requires git-lfs installed, and takes pretty long time to download"

    DEPENDEES mkdir
    DEPENDERS download
)

SET(MIR2X_DATA_REPO_PATH "${MIR2X_3RD_PARTY_DIR}/mir2x_data")
ADD_DEPENDENCIES(mir2x_3rds mir2x_data)
