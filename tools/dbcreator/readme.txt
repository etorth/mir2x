to install luasql-mysql, run ``luarocks install luasql-mysql"

1. luarocks may ask to specify  MYSQL_INCDIR and MYSQL_DIR, read the error report
   to set it and it will compile to get luasql.so, by default luarocks will search
   mysql.h and libmysqlclient in /usr/local/xxx.

2. when running lua intepretor may report ``can't find luasql.so", check you lua
   version, if you built luasql.so with lua5.1 but ran lua5.3 it couldn't work, to
   solve this explicitly use specified /usr/bin/lua5.x to run lua

3. if you can't login mysql using root with a normal user session, i.e. you have to do

        sudo mysql -u root

   to get rid of using sudo:

    1. connect in sudo mysql:

            sudo mysql -u root

    2. delete current root@localhost account

            mysql> DROP USER 'root'@'localhost';
            Query OK, 0 rows affected (0,00 sec)

    3. recreate your user with password '123456'

            mysql> CREATE USER 'root'@'%' IDENTIFIED BY '123456';
            Query OK, 0 rows affected (0,00 sec)

    4. give permissions to your user (don't forget to flush privileges)

            mysql> GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' WITH GRANT OPTION;
            Query OK, 0 rows affected (0,00 sec)

            mysql> FLUSH PRIVILEGES;
            Query OK, 0 rows affected (0,01 sec)

    5. exit mysql and try to reconnect without sudo
