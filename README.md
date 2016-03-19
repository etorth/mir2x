# mir2x
client, server, tools for cross-platform mir2. Using SDL, FLTK, libzip, etc..

1. pkgviewer
2. shadowmaker
3. mapeditor
4. client
5. monoserver

Try to make all I learned into practice

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
2. exception system only throw/catch std::error_code

And when throwing error_code:

1. for non-fatal exception: throw std::error_code(0)
2. for fatal exception throw std::error_code(1) and process exit(0)
3. don't use LOGTYPE_FATAL if not in main()
