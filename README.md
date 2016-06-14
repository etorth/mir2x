# mir2x
client, server, tools for cross-platform mir2. Using asio, SDL, FLTK, libzip, etc..

1. pkgviewer
2. animaker
3. shadowmaker
4. mapeditor
5. client
6. monoserver

Try to make all I learned into practice, as the screenshot:
![image](https://github.com/etorth/mir2x/raw/master/readme/screenshot.png)


global variables:

1. don't use global of build-in type or struct since no multithread control.
2. don't use global of class instanse since confusing construction/distruction.

actually:

1. only use class pointer;
2. only reference it by ``extern g_VarXXX";
3. no local function for operation on global variable only, means:
4. all operations over global variables should be self-contained;

Since there have both log system and exception system

1. log system handle all detailed info
2. exception system only throw/catch std::error_code()

The function who throws always think it's a fatal error so it just throw, but how to handle this ``fatal" error or do catch sub-clause really takes it as fatal is decided not by the thrower, but the catcher.

General rules:

1. only throw std::error_code() and log detailed information.
2. for functions which throws, the throwed type should be specified.
3. for constructor, it may throw different type of exceptions.

So if there are constructors in a normal function, we need catch-rethrow if there do exist exceptions, type rethrowed should only be std::error_code()

