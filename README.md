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
1. only use class pointer.
2. only reference it by ``extern g_VarXXX".
3. no local function for operation on global variable only, means:
4. all operations over global variables should be self-contained.
