# Tests

C++ suites use Catch2 v3 from the project's vcpkg registry baseline. CMake's
`catch_discover_tests()` registers individual cases with CTest, prefixed by their
suite name. Tests assert behavior directly; there are no stdout gold files or
Python comparison wrapper.

Build the project using the supported out-of-tree configuration:

```bash
/home/anhong/mir2x/build.py \
    --build-dir=/home/anhong/b_mir2x_vcpkg \
    --parallel=10 \
    --c-compiler=gcc-16 \
    --cxx-compiler=g++-16 \
    --res-path=/home/anhong/b_mir2x/3rdparty/mir2x_res \
    --build-type=Debug \
    --no-ccache
```

Test executables are excluded from the normal build. Append `--run-test` to the
`build.py` command above to build all test executables and run the CMake `check`
target, which invokes CTest.

Rerun all cases, or select one suite:

```bash
ctest --test-dir /home/anhong/b_mir2x_vcpkg/build --output-on-failure
ctest --test-dir /home/anhong/b_mir2x_vcpkg/build -R '^queststate/' --output-on-failure
```

Lua quest regression scripts run inside a Catch2 executable using the vcpkg Lua
library; a separate system Lua executable is not needed. The Lua assertions and
mock fixtures remain in `server/test/unit/test_*quests.lua` and
`test_questscripts.lua`. A failed script reports its Lua error through Catch2.
The same executable also runs `server/test/merchant.lua` against all NPC scripts.

Server Lua-runner suites load scripts from the installed server directory.
The client geometry suite requires installed font/emoji resources and a working
SDL video backend. The `build.py` command above installs the build before running
the tests; do not use `--no-install` when the installed scripts or resources
need refreshing.

`MIR2X_TEST_RES_DIR` can override the client suite's installed resource directory.
Catch2 owns executable arguments, so positional resource-path arguments are not
used. External-command integration tests, when added, run directly under CTest
with a timeout and their process exit status.
