mysql = require "luasql.mysql"

local env  = assert(mysql.mysql())
local conn = assert(env:connect('mir2x', 'root', '123456', "localhost", 3306))
print(env, conn)

conn:execute("set names utf8")

status, errmsg = conn:execute [[
    create database if not exists mir2x character set utf8
]]



-- create table for user account info
status, errmsg = conn:execute [[
    create table if not exists tbl_account
    (
        fld_id       int unsigned not null auto_increment primary key,
        fld_account  char(32) not null,
        fld_password char(32) not null
    )
]]

if errmsg then print(status, errmsg) end

conn:execute [[use mir2x]]

-- print all tables in current db
-- cursor, errmsg = conn:execute [[show tables]]
-- row = cursor:fetch({}, "a")
--
-- while row do
--     for key, value in pairs(row) do print(value) end
--     row = cursor:fetch(row, "a")
-- end

-- try to add an account
conn:execute [[
    insert tbl_account (fld_account, fld_password) values("test", "123456")
]]

-- create table for guid
status, errmsg = conn:execute [[
    create table if not exists tbl_guid
    (
        fld_guid int unsigned not null auto_increment primary key,
        fld_name char(32) not null,
        fld_id   int unsigned not null
    )
]]

-- try to add guid
conn:execute [[
    insert tbl_guid (fld_name, fld_id) values
        ("mark"  , 1),
        ("john"  , 1),
        ("linda" , 2),
        ("steven", 2)
]]
