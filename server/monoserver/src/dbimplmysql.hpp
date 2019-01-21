/*
 * =====================================================================================
 *
 *       Filename: dbimplmysql.hpp
 *        Created: 01/21/2019 07:24:10
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

#pragma once
#include <new>
#include "mysqlinc.hpp"
#include "dbrecord.hpp"
namespace DBImpl
{

class DBConnection
{
    // don't use it to create new database
    // this should be done by me manually
    //
    // DBConnection only query information from current database
    // then each DBConnection has a specified database name
    //
    public:
        DBConnection(const char *, const char *, const char *, const char *, unsigned int);
       ~DBConnection();

    public:
        bool Valid(){ return m_Valid; }

    public:
        const char *DBEngine() const;

    public:
        int ErrorID();
        const char *ErrorInfo();

    public:
        DBRecord *CreateDBRecord(DBRecord *pBuf = nullptr)
        {
            return pBuf ? (new (pBuf) DBRecord(this)) : (new DBRecord(this));
        }

        void DestroyDBRecord(DBRecord *);

    private:
        MYSQL   *m_SQL;
        bool     m_Valid;

    public:
        friend class DBRecord;
};
}


#include "mysqlinc.hpp"
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
