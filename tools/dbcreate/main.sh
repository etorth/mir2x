#!/bin/bash

echo '****************************************************'
echo '*              Database creation script            *'
echo '*                                                  *'
echo '*                                                  *'
echo '* Usage: main.sh [-f]                              *'
echo '*                                                  *'
echo '*   1. if "-f" provided, re-create the whole db.   *'
echo '*   2. otherwise do check-and-create.              *'
echo '*                                                  *'
echo '****************************************************'

/usr/bin/lua5.1 ./database.lua
