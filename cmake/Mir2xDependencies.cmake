find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Lua REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(g3log CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(sdl2-gfx CONFIG REQUIRED)
find_package(FLTK CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(unofficial-tiny-aes-c CONFIG REQUIRED)
find_package(argh CONFIG REQUIRED)

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
pkg_check_modules(LIBPINYIN REQUIRED IMPORTED_TARGET libpinyin)
pkg_get_variable(MIR2X_LIBPINYIN_DATA_DIR libpinyin pkgdatadir)
find_library(LIBDB_LIBRARY REQUIRED
    NAMES db db-5.3
    HINTS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
    NO_DEFAULT_PATH)

find_path(LIBPOPCNT_INCLUDE_DIR REQUIRED NAMES libpopcnt.h)
find_path(PHMAP_INCLUDE_DIR REQUIRED NAMES parallel_hashmap/phmap.h)

if(NOT FLTK_FLUID_EXECUTABLE)
    find_program(FLTK_FLUID_EXECUTABLE
        NAMES fluid
        PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/fltk"
        NO_DEFAULT_PATH)
endif()
if(NOT FLTK_FLUID_EXECUTABLE)
    message(FATAL_ERROR "Failed to find FLTK fluid executable")
endif()

add_library(mir2x_project_options INTERFACE)
add_library(mir2x::project_options ALIAS mir2x_project_options)
target_compile_definitions(mir2x_project_options INTERFACE
    ASIO_STANDALONE
    SDL_MAIN_HANDLED
    SOL_ALL_SAFETIES_ON=1
    SOL_SAFE_NUMERICS=1
    ZSTD_MULTITHREAD
    $<$<CONFIG:Debug>:MIR2X_DEBUG_MODE>)

if(WIN32)
    target_compile_definitions(mir2x_project_options INTERFACE
        WINVER=0x0601
        _WIN32_WINNT=0x0601
        WIN32_LEAN_AND_MEAN
        NOMINMAX)
endif()

if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
    # GCC 16's C++26 constexpr exception declarations conflict with static libstdc++ on MinGW Debug builds.
    target_compile_definitions(mir2x_project_options INTERFACE $<$<CONFIG:Debug>:__cpp_lib_constexpr_exceptions=0>)
endif()

if(MSVC)
    target_compile_options(mir2x_project_options INTERFACE /W4)
    target_compile_definitions(mir2x_project_options INTERFACE _HAS_STD_BYTE=0)
else()
    target_compile_options(mir2x_project_options INTERFACE
        -fcoroutines
        -fno-strict-aliasing
        -Wall
        -Wfatal-errors
        -Wextra
        -Wunused
        -Werror
        -Wno-missing-field-initializers)
endif()

if(MINGW)
    target_link_options(mir2x_project_options INTERFACE -static -static-libgcc -static-libstdc++)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(mir2x_project_options INTERFACE -static-libgcc -static-libstdc++)
endif()

add_library(mir2x_sdl2 INTERFACE)
add_library(mir2x::sdl2 ALIAS mir2x_sdl2)
target_link_libraries(mir2x_sdl2 INTERFACE
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)

add_library(mir2x_sdl2_ext INTERFACE)
add_library(mir2x::sdl2_ext ALIAS mir2x_sdl2_ext)
target_link_libraries(mir2x_sdl2_ext INTERFACE
    mir2x::sdl2
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    SDL2::SDL2_gfx)

add_library(mir2x_lua INTERFACE)
add_library(mir2x::lua ALIAS mir2x_lua)
if(TARGET Lua::Lua)
    target_link_libraries(mir2x_lua INTERFACE Lua::Lua)
else()
    target_include_directories(mir2x_lua SYSTEM INTERFACE ${LUA_INCLUDE_DIR})
    target_link_libraries(mir2x_lua INTERFACE ${LUA_LIBRARIES})
endif()

add_library(mir2x_fltk INTERFACE)
add_library(mir2x::fltk ALIAS mir2x_fltk)
target_include_directories(mir2x_fltk SYSTEM INTERFACE ${FLTK_INCLUDE_DIRS})
foreach(MIR2X_FLTK_TARGET IN ITEMS fltk_images fltk_gl fltk)
    if(TARGET ${MIR2X_FLTK_TARGET})
        target_link_libraries(mir2x_fltk INTERFACE ${MIR2X_FLTK_TARGET})
    endif()
endforeach()
target_link_libraries(mir2x_fltk INTERFACE OpenGL::GL)

add_library(mir2x_header_deps INTERFACE)
add_library(mir2x::header_deps ALIAS mir2x_header_deps)
target_include_directories(mir2x_header_deps SYSTEM INTERFACE
    ${LIBPOPCNT_INCLUDE_DIR}
    ${PHMAP_INCLUDE_DIR})
target_link_libraries(mir2x_header_deps INTERFACE
    argh
    asio::asio
    cereal::cereal
    sol2::sol2
    utf8::cpp)
if(WIN32)
    target_link_libraries(mir2x_header_deps INTERFACE
        ws2_32
        mswsock)
endif()

add_library(mir2x_common_deps INTERFACE)
add_library(mir2x::common_deps ALIAS mir2x_common_deps)
target_link_libraries(mir2x_common_deps INTERFACE
    mir2x::project_options
    mir2x::sdl2
    mir2x::fltk
    mir2x::lua
    mir2x::header_deps
    Threads::Threads
    ZLIB::ZLIB
    PNG::PNG
    lz4::lz4
    zstd::libzstd
    tinyxml2::tinyxml2
    g3log
    unofficial::tiny-aes-c::tiny-aes-c
    ${CMAKE_DL_LIBS})

add_library(mir2x_libpinyin INTERFACE)
add_library(mir2x::libpinyin ALIAS mir2x_libpinyin)
target_link_libraries(mir2x_libpinyin INTERFACE
    PkgConfig::LIBPINYIN
    ${LIBDB_LIBRARY})

add_library(mir2x_sqlitecpp INTERFACE)
add_library(mir2x::sqlitecpp ALIAS mir2x_sqlitecpp)
target_link_libraries(mir2x_sqlitecpp INTERFACE
    SQLiteCpp
    unofficial::sqlite3::sqlite3)
