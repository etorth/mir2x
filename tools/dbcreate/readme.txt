to install luasql-mysql, run ``luarocks install luasql-mysql"

1. luarocks may ask to specify  MYSQL_INCDIR and MYSQL_DIR, read the error report
   to set it and it will compile to get luasql.so, by default luarocks will search
   mysql.h and libmysqlclient in /usr/local/xxx.

2. when running lua intepretor may report ``can't find luasql.so", check you lua
   version, if you built luasql.so with lua5.1 but ran lua5.3 it couldn't work, to
   solve this explicitly use specified /usr/bin/lua5.x to run lua
