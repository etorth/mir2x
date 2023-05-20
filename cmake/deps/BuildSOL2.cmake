INCLUDE(ExternalProject)

ExternalProject_Add(
    sol2

    GIT_REPOSITORY "https://github.com/ThePhD/sol2.git"
    # GIT_TAG        "develop"
    # GIT_TAG        "v2.20.6"
    GIT_TAG        "v3.3.0"
  
    SOURCE_DIR "${MIR2X_3RD_PARTY_DIR}/sol2"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""

    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
)

ADD_COMPILE_DEFINITIONS(SOL_SAFE_NUMERICS=1)
SET(SOL2_INCLUDE_DIRS "${MIR2X_3RD_PARTY_DIR}/sol2/include")
INCLUDE_DIRECTORIES(SYSTEM ${SOL2_INCLUDE_DIRS})
ADD_DEPENDENCIES(mir2x_3rds sol2)
