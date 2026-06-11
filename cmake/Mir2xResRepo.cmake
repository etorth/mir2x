INCLUDE(ExternalProject)

ExternalProject_Add(
    mir2x_res

    GIT_REPOSITORY "https://github.com/etorth/mir2x_res.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE

    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/mir2x_res"

    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

ExternalProject_Add_Step(
    mir2x_res
    requires_python_and_long_time_download_warning

    COMMAND ""
    COMMENT "Repository 'mir2x_res' requires python to create mir2x resource database, and takes pretty long time to download"

    DEPENDEES mkdir
    DEPENDERS download
)

SET(MIR2X_RES_REPO_PATH "${MIR2X_3RD_PARTY_DIR}/mir2x_res")
ADD_DEPENDENCIES(mir2x_3rds mir2x_res)
