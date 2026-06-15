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
  - server
  - pkgviewer
  - animaker
  - mapeditor

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


### Windows

For windows please download binaries from appveyor
```
https://ci.appveyor.com/project/etorth/mir2x/build/artifacts
```
If complains missing dll, you may need to copy .dll files from mir2x/bin to mir2x/client and mir2x/server.

If running on WSL/WSL2, check the following to configure PulseAudio to support sound effect, the sound may get played with noticable delay.
<img src="https://github.com/etorth/mir2x/raw/master/readme/pulseaudio.png"/>

### Building from source

mir2x uses vcpkg manifest mode for third-party dependencies on 64-bit native Linux and 64-bit MSYS2 UCRT64/MinGW. The helper script clones and bootstraps a local vcpkg checkout in the current working directory, configures the CMake build, builds, and installs.

```sh
$ git clone https://github.com/etorth/mir2x.git
$ mkdir b_mir2x
$ cd b_mir2x
$ /path/to/mir2x/build.py
```

Builds are incremental by default: rerunning the same command keeps `<build-dir>/build`, including CMake object files, `vcpkg_installed`, and the default resource clone. Use `--fresh` only when you want a real clean build: it deletes `<build-dir>/build`, including `vcpkg_installed` and `<build-dir>/build/assets/mir2x_res`, so vcpkg dependencies are reinstalled/rebuilt and default resources are cloned again.

Install-time client/server resource packing always runs. If `--res-path` is omitted, the CMake build clones `https://github.com/etorth/mir2x_res.git` to `<build-dir>/build/assets/mir2x_res` during the build stage. To use an existing resource checkout, pass:

```sh
$ /path/to/mir2x/build.py --res-path=/path/to/mir2x_res
```

To choose a compiler for both vcpkg ports and mir2x targets, pass it through the helper. This enables `VCPKG_CHAINLOAD_TOOLCHAIN_FILE` internally:

```sh
$ /path/to/mir2x/build.py --c-compiler=gcc-16 --cxx-compiler=g++-16
```

To control build parallelism, use `--parallel=<N>`:

```sh
$ /path/to/mir2x/build.py --parallel=16
```

To show detailed CMake/vcpkg command output, add `--verbose`.

On Windows, run the same command from an MSYS2 UCRT64 shell; the helper selects the `x64-mingw-static` vcpkg triplet by default.
### First time run
To start the monoserver, find a linux machine to host the server, I tried to host it on ```Oracle Cloud Infrastructure```, it works perfectly with the ```always-free``` plan. Click menu server/launch to start the service before start client:

```sh
$ cd mir2x/b/install/server
$ ./server --auto-launch
```

Start client, currently you can use default account (id = test, pwd = 123456) to try it:

```sh
$ cd mir2x/b/install/client
$ ./client --server-ip=localhost --auto-login=test:123456
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
* [ThreadPool](https://github.com/progschj/ThreadPool) - A simple C++11 Thread Pool implementation.
* [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) - SQLiteC++ (SQLiteCpp) is a smart and easy to use C++ SQLite3 wrapper.

### running WSL2

wsl2 sucks! if in WSL2 you can ping 8.8.8.8 but can not ping google.com, that means you DNS is wrong.
1. in windows run
   ```shell
   ipconfig
   ```
   You will see line as ``Ethernet adapter vEthernet (WSL (Hyper-V firewall))``, the ``IPv4 Address`` of this section is used as your DNS in WSL2 ``/etc/resolv.conf``, everytime when WSL2 reboots, it automatically create this ``/etc/resolv.conf``, you need to disable the automagicall overwrite, there is comment in file ``/etc/resolv.conf`` explaining how to do it.

2. in windows run
   ```shell
   ipconfig /all
   ```
   You will find the section ``Wireless LAN adapter Wi-Fi``, inside the section find ``DNS Server`` and copy all IPs to ``/etc/resolv.conf``, reboot WSL2, it should be good now.
