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

mir2x requires [cmake](https://cmake.org/) v3+ and [gcc](https://gcc.gnu.org/) support c++14 to run. Mir2x needs some pre-installed packages before compile:

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

Generally cmake complains when libs are missing. After install all these dependencies and devDependencies, clone the repo and compile with cmake. By default it tries to install in /usr/local. use ``CMAKE_INSTALL_PREFIX" to customize.

```sh
$ git clone https://github.com/etorth/mir2x.git
$ cd mir2x
$ mkdir b
$ cd b
$ cmake .. -DCMAKE_INSTALL_PREFIX=${PWD}/install
$ make
$ make install
```

### Code style

global variables:

1. don't use global of build-in type or struct since no multithread control.
2. don't use global of class instanse since confusing construction/distruction.

actually:

1. only use class pointer;
2. only reference it by ``extern g_VarXXX";
3. no local function for operation on global variable only, means:
4. all operations over global variables should be self-contained;
5. all global variable pointers stay valid during the whole procedure;

Since I already have a powerful log system, I won't use exception. If un-recoverable error happens

1. log system record the detailed info by LOGTYPE_FATAL;
2. then just let it crash, or use exit(0) to do forced kill;

The function who throws always think it's a fatal error so it just throw, but how to handle this ``fatal" error or do catch sub-clause really takes it as fatal is decided not by the thrower, but the catcher.

For modules like mapeditor which doesn't have a log system, always put assertion to check parameters. If functions invoked with invalid parameters, fail assertion and let it crash.

General rules for functions:

1. put strict parameters check above doing actual logic;
2. take invalid argument as severe error, just log the error and let it crash;
3. never give assumption for argument;
4. try best to make each memeber function self-contained to avoid first-half / second-half splitted functions;

General rules for classes:

1. an object should be in legal state when created, by factory method or constructor;
2. an object should stay valid if no input provided;
3. put strict parameters check when provide input to objects, and reject immedately if not valid;
4. avoid to do state validation outside an object;
5. external call of member function should never break current object, keep log instead;


### Packages

mir2x uses a number of open source projects to work properly, and of course itself is open source with a public repository on github, please remind me if I missed anything.

* [SDL2](https://www.libsdl.org/) - A cross-platform development library designed to provide a hardware abstraction layer.
* [FLTK](http://www.fltk.org) - A cross-platform C++ GUI toolkit for UNIX®/Linux® (X11), Microsoft® Windows®, and MacOS® X.
* [asio](http://www.think-async.com/) - A cross-platform C++ library for network and low-level I/O programming.
* [theron](http://www.theron-library.com/) - A lightweight C++ concurrency library based on the Actor Model.
* [g3log](https://github.com/KjellKod/g3log) - An asynchronous, "crash safe", logger that is easy to use.
* [lua](https://www.lua.org/) - A powerful, efficient, lightweight, embeddable scripting language.
* [sol2](https://github.com/ThePhD/sol2) - A fast, simple C++ and Lua binding.
* [mariadb](https://mariadb.org/) - A community-developed fork of the MySQL relational database management system.
* [luasql](https://keplerproject.github.io/luasql/) - LuaSQL is a simple interface from Lua to a DBMS.
* [tinyxml2](http://www.grinninglizard.com/tinyxml2/) - A simple, small, efficient, C++ XML parser.
* [utf8-cpp](http://utfcpp.sourceforge.net/) - A simple, portable and lightweigt C++ library for UTF-8 string handling.
* [libpng](http://www.libpng.org/pub/png/libpng.html) - The official PNG reference library.
* [libzip](https://nih.at/libzip/) - C library for reading, creating, and modifying zip archives.
* [ThreadPool](https://github.com/progschj/ThreadPool) - A simple C++11 Thread Pool implementation
* [astar-algorithm](https://github.com/justinhj/astar-algorithm-cpp) - Implementation of the A* algorithm in C++ and C#
