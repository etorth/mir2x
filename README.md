# mir2x

<a href="https://scan.coverity.com/projects/etorth-mir2x">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9270/badge.svg"/>
</a>

mir2x is a c/s based mir2ei implementation with various platforms supported. It contains all need components for game players and developers:

  - client
  - monoserver
  - pkgviewer
  - animaker
  - mapeditor
  - dbcreator

![image](https://github.com/etorth/mir2x/raw/master/readme/screencapture.gif)
![image](https://github.com/etorth/mir2x/raw/master/readme/mapeditor.png)


### Building from source

mir2x requires [cmake](https://cmake.org/) v3+ and [gcc](https://gcc.gnu.org/) support c++17 to run. Mir2x needs some pre-installed packages before compile:

```sh
libsdl2-dev
libsdl2-image-dev
libsdl2-ttf-dev
libsdl2-mixer-dev
libpng-dev
liblua5.3-dev
libmysqlclient-dev
libfltk1.3-dev
```

Cmake complains if libs are missing. After install all these dependencies, clone and compile the repo. By default cmake tries to install in /usr/local. use ``CMAKE_INSTALL_PREFIX" to customize.

```sh
$ git clone https://github.com/etorth/mir2x.git
$ cd mir2x
$ mkdir b
$ cd b
$ cmake .. -DCMAKE_INSTALL_PREFIX=${PWD}/install
$ make
$ make install
```
### First time run
The above steps install binaries in mir2x/b/install. Before run the server/client you need to start mysql server on your host. Then create the default database:

```sh
$ cd mir2x/b/install/tools/dbcreator/
$ ./main.sh
```

Start the monoserver, click menu server/launch to start the service before start client:

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
* [mariadb](https://mariadb.org/) - A community-developed fork of the MySQL relational database management system.
* [luasql](https://keplerproject.github.io/luasql/) - LuaSQL is a simple interface from Lua to a DBMS.
* [tinyxml2](http://www.grinninglizard.com/tinyxml2/) - A simple, small, efficient, C++ XML parser.
* [utf8-cpp](http://utfcpp.sourceforge.net/) - A simple, portable and lightweigt C++ library for UTF-8 string handling.
* [libpng](http://www.libpng.org/pub/png/libpng.html) - The official PNG reference library.
* [ThreadPool](https://github.com/progschj/ThreadPool) - A simple C++11 Thread Pool implementation
* [astar-algorithm](https://github.com/justinhj/astar-algorithm-cpp) - Implementation of the A* algorithm in C++ and C#
