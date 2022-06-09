# mir2x

<a href="https://scan.coverity.com/projects/etorth-mir2x">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9270/badge.svg"/>
</a>
<a href="https://ci.appveyor.com/project/etorth/mir2x">
  <img alt="Appveyor Build Status"
       src="https://ci.appveyor.com/api/projects/status/github/etorth/mir2x?svg=true"/>
</a>
<a href="https://travis-ci.org/github/etorth/mir2x">
  <img alt="Travis CI Build Status"
       src="https://travis-ci.org/etorth/mir2x.svg?branch=master"/>
</a>
<a href="https://gitter.im/mir2x/community?utm_source=share-link&utm_medium=link&utm_campaign=share-link">
  <img alt="Gitter chat"
       src="https://badges.gitter.im/org.png"/>
</a>

mir2x is an experimental project that verifies actor-model based parallelism for MMORPG, it's c/s based with various platforms supported and contains all need components for game players and developers:

  - client
  - monoserver
  - pkgviewer
  - animaker
  - mapeditor

### Notes
- This repo uses C++20 coroutine, developer needs a compiler supports c++20 to build.
- This repo uses classic v1.45 mir2 as a reference implementation, you can try the original game:
  - Install [win-xp](https://github.com/etorth/winxp-zh) to host and run the game server/client, tested on real machine or virtualbox machine.
  - Install server/client from [mir2-v1.45](https://github.com/etorth/CBWCQ3).
  - Change screen resolution to 16bit mode to run the game.

### Public Server
- Check the tutorial [here](https://github.com/etorth/mir2x/wiki/Host-your-monoserver-on-Oracle-Cloud) for how to run the ```monoserver``` with Oracle Cloud as a public server.
- You can try the public test server ```192.9.241.118``` by
  ```shell
  client --server-ip=192.9.241.118
  ```

YouTube links: [1](https://youtu.be/Yz-bGOkDyEQ) [2](https://youtu.be/jl1LPxe2EAA) [3](https://youtu.be/TtGONA83Mb8)

<https://user-images.githubusercontent.com/1754214/162589720-7dd9453b-55e4-4119-a1ee-c879093cf017.mp4>


### Windows

For windows please download binaries from appveyor
```
https://ci.appveyor.com/project/etorth/mir2x/build/artifacts
```
If complains missing dll, you may need to copy .dll files from mir2x/bin to mir2x/client and mir2x/server.

If running on WSL/WSL2, check the following to configure PulseAudio to support sound effect, the sound may get played with noticable delay.
<img src="https://github.com/etorth/mir2x/raw/master/readme/pulseaudio.png"/>

### Building from source

mir2x game is developed for Linux-only environment, however I finished all coding and testing with Windows WSL, please refer [here](https://github.com/etorth/mir2x/wiki/How-to-compile-and-run-on-windows) how to setup and run everything on windows. mir2x requires [cmake](https://cmake.org/) v3.12 and [gcc](https://gcc.gnu.org/) support c++20 to run. Mir2x needs some pre-installed packages before compile:

```sh
libsdl2-dev
libsdl2-image-dev
libsdl2-mixer-dev
libsdl2-ttf-dev
libsdl2-gfx-dev
libpng-dev
liblua5.3-dev
libfltk1.3-dev
```

Cmake complains if libs are missing. After install all these dependencies, clone and compile the repo. By default cmake tries to install in /usr/local. use ``CMAKE_INSTALL_PREFIX" to customize.

```sh
$ git clone https://github.com/etorth/mir2x.git
$ cd mir2x
$ mkdir b
$ cd b
$ cmake .. -DCMAKE_INSTALL_PREFIX=install
$ make
$ make install
```
### First time run
To start the monoserver, find a linux machine to host the monoserver, I tried to host it on ```Oracle Cloud Infrastructure```, it works perfectly with the ```always-free``` plan. Click menu server/launch to start the service before start client:

```sh
$ cd mir2x/b/install/server
$ ./monoserver
```

Start client, currently you can use default account (id = test, pwd = 123456) to try it:

```sh
$ cd mir2x/b/install/client
$ ./client
```

### Code style

Global variables:

1. no globals of builtin types, they are lacking of multithread access control.
2. no globals of class instances, use pointer instead, for better construction/destruction control.
3. all member functions of globals should be:
    - simple
    - thread-safe
    - atomic operations
4. name all global pointers as g_XXXX and use them by extern, and
    - allocate them at beginning of main()
    - remain valid during the whole run, and ONLY free them at process exit.

Error handling:
1. use exception for good/bad path control, force catch at exit of main() or clone().
2. do strict parameters checking before doing actual logic, no assumptions.
3. let the crash happen ASAP if any fatal error detected

General rules:
1. make all member functions self-contained, avoid first/second half logic.
2. don't do optimization too early, perfer simple/clear logic.

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
* [libpng](http://www.libpng.org/pub/png/libpng.html) - The official PNG reference library.
* [ThreadPool](https://github.com/progschj/ThreadPool) - A simple C++11 Thread Pool implementation.
* [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) - SQLiteC++ (SQLiteCpp) is a smart and easy to use C++ SQLite3 wrapper.
