# The caller is vcpkg.cmake; retain its Darwin platform settings.
get_filename_component(MIR2X_VCPKG_BUILDSYSTEMS_DIR "${CMAKE_PARENT_LIST_FILE}" DIRECTORY)
include("${MIR2X_VCPKG_BUILDSYSTEMS_DIR}/../toolchains/osx.cmake")

set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE FILEPATH "C compiler for native macOS dependencies" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE FILEPATH "C++ compiler for native macOS dependencies" FORCE)
set(CMAKE_OBJC_COMPILER "/usr/bin/clang" CACHE FILEPATH "Objective-C compiler for native macOS dependencies" FORCE)
set(CMAKE_OBJCXX_COMPILER "/usr/bin/clang++" CACHE FILEPATH "Objective-C++ compiler for native macOS dependencies" FORCE)
