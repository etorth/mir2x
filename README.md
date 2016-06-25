# mir2x

<a href="https://scan.coverity.com/projects/etorth-mir2x">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9270/badge.svg"/>
</a>

client, server, tools for cross-platform mir2. Using asio, SDL, FLTK, libzip, etc..

<a href="https://scan.coverity.com/projects/etorth-mir2x">
  <img alt="Coverity Scan Build Status"
       src="https://img.shields.io/coverity/scan/9270.svg"/>
</a>


1. pkgviewer
2. animaker
3. shadowmaker
4. mapeditor
5. client
6. monoserver

Try to make all I learned into practice, as the screenshot:
![image](https://github.com/etorth/mir2x/raw/master/readme/screenshot.png)
![image](https://github.com/etorth/mir2x/raw/master/readme/mapeditor.png)

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

1. log system record the detailed info;
2. then just let it crash, or use exit(0) to force killing;

The function who throws always think it's a fatal error so it just throw, but how to handle this ``fatal" error or do catch sub-clause really takes it as fatal is decided not by the thrower, but the catcher.

For modules like mapeditor which doesn't have a log system, always put assertion to check parameters. If functions invoked with invalid parameters, fail assertion and let it crash.

General rules for functions:

1. put strict parameters check above doing actual logic;
2. take invalid argument as severe error, just log the error and let it crash;
3. never give assumption for argument;
4. try best to make each memeber function self-contained to avoid first-half / second-half splitted functions;

