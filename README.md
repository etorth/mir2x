# mir2x

<a href="https://github.com/etorth/mir2x/actions/workflows/build.yml">
  <img alt="GitHub Actions Build Status"
       src="https://github.com/etorth/mir2x/actions/workflows/build.yml/badge.svg"/>
</a>
<a href="https://scan.coverity.com/projects/etorth-mir2x">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9270/badge.svg"/>
</a>
<a href="https://gitter.im/mir2x/community?utm_source=share-link&utm_medium=link&utm_campaign=share-link">
  <img alt="Gitter chat"
       src="https://badges.gitter.im/org.png"/>
</a>

mir2x is an experimental project that verifies actor-model based parallelism for MMORPG, it's c/s based with various platforms supported and contains all need components for game players and developers:

  - client
  - server
  - pkgviewer
  - animaker
  - mapeditor

### Prebuilt binaries

Each push to the repository publishes a rolling `latest` GitHub release containing Linux and Windows MinGW UCRT64 install trees:

- [mir2x-linux-latest-build.zip](https://github.com/etorth/mir2x/releases/download/latest/mir2x-linux-latest-build.zip)
- [mir2x-windows-latest-build.zip](https://github.com/etorth/mir2x/releases/download/latest/mir2x-windows-latest-build.zip)

The full release page is at <https://github.com/etorth/mir2x/releases/tag/latest>.

### Notes
- This repo uses C++ coroutine to implement actor model, requires compiler to support c++23.
- This repo uses classic v1.45 mir2 as a reference implementation, you can try the original game:
  - Install [win-xp](https://github.com/etorth/winxp-zh) to host and run the game server/client, tested on real machine or virtualbox machine.
  - Install server/client from [mir2-v1.45](https://github.com/etorth/CBWCQ3).
  - Change screen resolution to 16bit mode to run the game.

### Public Server
- Check the tutorial [here](https://github.com/etorth/mir2x/wiki/Host-your-monoserver-on-Oracle-Cloud) for how to run the ```server``` with Oracle Cloud as a public server.
- You can try the public test server ```192.9.241.118``` by
  ```shell
  client --server-ip=192.9.241.118 # not maintained recently
  ```

YouTube links: [1](https://youtu.be/Yz-bGOkDyEQ) [2](https://youtu.be/jl1LPxe2EAA) [3](https://youtu.be/TtGONA83Mb8)

<https://user-images.githubusercontent.com/1754214/162589720-7dd9453b-55e4-4119-a1ee-c879093cf017.mp4>


An IME for SDL fullscreen mode:

<https://user-images.githubusercontent.com/1754214/213572554-785e826c-226d-43fa-a196-ee4f92112db2.mp4>


### Building from source

mir2x uses vcpkg manifest mode for third-party dependencies on 64-bit native Linux and 64-bit MSYS2 UCRT64/MinGW. The helper script clones and bootstraps a local vcpkg checkout in the current working directory, configures the CMake build, builds, and installs.


#### Linux (Ubuntu 26.04)

mir2x is built with GCC 16:

```sh
sudo apt update
sudo apt install -y \
    autoconf autoconf-archive automake \
    build-essential cmake curl \
    g++-16 gcc-16 gawk gettext git libtool ninja-build pkg-config \
    python3 tar unzip \
    libgl1-mesa-dev libglu1-mesa-dev \
    libice-dev libltdl-dev libsm-dev \
    libx11-dev libxcursor-dev libxext-dev libxfixes-dev \
    libxft-dev libxinerama-dev libxrender-dev
```

Then clone and build:

```sh
git clone https://github.com/etorth/mir2x.git
mkdir b_mir2x && cd b_mir2x
python3 /path/to/mir2x/build.py --c-compiler=gcc-16 --cxx-compiler=g++-16 --parallel=10
```

#### Windows (MSYS2 UCRT64)

Install [MSYS2](https://www.msys2.org/), then from a UCRT64 shell install the toolchain:

```sh
pacman -S --needed \
    mingw-w64-ucrt-x86_64-toolchain \
    mingw-w64-ucrt-x86_64-git \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-pkgconf \
    mingw-w64-ucrt-x86_64-python
```

Then clone and build from the same UCRT64 shell; the helper selects the `x64-mingw-static` vcpkg triplet by default:

```sh
git clone https://github.com/etorth/mir2x.git
make b_mir2x && cd b_mir2x
python3 /path/to/mir2x/build.py --build-dir=/path/to/b_mir2x --parallel=10
```

#### Helper script options

Builds are incremental by default: rerunning the same command keeps `<build-dir>/build`, including CMake object files, `vcpkg_installed`, and the default resource clone. Use `--fresh` only when you want a real clean build: it deletes `<build-dir>/build`, including `vcpkg_installed` and `<build-dir>/build/assets/mir2x_res`, so vcpkg dependencies are reinstalled/rebuilt and default resources are cloned again.

Install-time client/server resource packing always runs. If `--res-path` is omitted, the CMake build clones `https://github.com/etorth/mir2x_res.git` to `<build-dir>/build/assets/mir2x_res` during the build stage. To use an existing resource checkout, pass:

```sh
$ /path/to/mir2x/build.py [options] --res-path=/path/to/mir2x_res
```

Other useful options:

- `--c-compiler=<cc> --cxx-compiler=<cxx>` selects a compiler for both vcpkg ports and mir2x targets (enables `VCPKG_CHAINLOAD_TOOLCHAIN_FILE` internally).
- `--parallel=<N>` controls build parallelism.
- `--verbose` shows detailed CMake/vcpkg command output.
### First time run
To start the monoserver, find a linux machine to host the server, I tried to host it on ```Oracle Cloud Infrastructure```, it works perfectly with the ```always-free``` plan. Click menu server/launch to start the service before start client:

```sh
cd b_mir2x/install/server
./server --auto-launch
```

Start client, currently you can use default account (id = test, pwd = 123456) to try it:

```sh
cd b_mir2x/install/client
./client --server-ip=localhost --auto-login=test:123456
```

### Packages

mir2x uses a number of open source projects to work properly, and of course itself is open source with a public repository on github, please remind me if I missed anything.

* [SDL2](https://www.libsdl.org/) - A cross-platform development library designed to provide a hardware abstraction layer.
* [FLTK](http://www.fltk.org) - A cross-platform C++ GUI toolkit for UNIX®/Linux® (X11), Microsoft® Windows®, and MacOS® X.
* [asio](http://www.think-async.com/) - A cross-platform C++ library for network and low-level I/O programming.
* [g3log](https://github.com/KjellKod/g3log) - An asynchronous, "crash safe", logger that is easy to use.
* [lua](https://www.lua.org/) - A powerful, efficient, lightweight, embeddable scripting language.
* [sol2](https://github.com/ThePhD/sol2) - A fast, simple C++ and Lua binding.
* [tinyxml2](http://www.grinninglizard.com/tinyxml2/) - A simple, small, efficient, C++ XML parser.
* [utf8-cpp](http://utfcpp.sourceforge.net/) - A simple, portable and lightweigt C++ library for UTF-8 string handling.
* [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) - SQLiteC++ (SQLiteCpp) is a smart and easy to use C++ SQLite3 wrapper.
