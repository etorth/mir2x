/*
 * =====================================================================================
 *
 *       Filename: dbconnection.cpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 03/27/2017 13:20:07
 *
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include <mysql/mysql.h>
#include "dbrecord.hpp"
#include "dbconnection.hpp"

DBConnection::DBConnection(
        const char * szHostName,
        const char * szUserName,
        const char * szPassword,
        const char * szDBName,
        unsigned int nPort)
    : m_SQL(nullptr)
    , m_Valid(false)
{
    m_Valid = false;
    m_SQL   = mysql_init(nullptr);

    mysql_options(m_SQL, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(m_SQL, MYSQL_INIT_COMMAND, "SET NAMES utf8");

    if(m_SQL){
        if(mysql_real_connect(m_SQL, szHostName, szUserName, szPassword, szDBName, nPort, nullptr, 0)){
            m_Valid = true;
        }
    }
}

DBConnection::~DBConnection()
{
    if(m_SQL){ mysql_close(m_SQL); }
}

void DBConnection::DestroyDBRecord(DBRecord *pDBRecord)
{
    delete pDBRecord;
}

const char *DBConnection::ErrorInfo()
{
    return m_SQL ? mysql_error(m_SQL) : "no valid SQL handler for current connection";
}

int DBConnection::ErrorID()
{
    return m_SQL ? (int)(mysql_errno(m_SQL)) : -1;
}
