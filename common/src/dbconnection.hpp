/*
 * =====================================================================================
 *
 *       Filename: dbconnection.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 05/26/2016 16:41:32
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
#pragma once
#include <new>
#include <mariadb/mysql.h>

#include "dbrecord.hpp"

class DBConnection
{
    // don't use it to create new database
    // this should be done by me manually
    //
    // DBConnection only query information from current database
    // then each DBConnection has a specified database name
    //
    public:
        DBConnection( const char *, const char *, const char *, const char *, unsigned int);
        ~DBConnection();

    public:
        bool    Valid();

    public:
        int ErrorID();
        const char *ErrorInfo();

    public:
        DBRecord *CreateDBRecord(DBRecord *pBuf = nullptr)
        {
            if(pBuf){ return (new (pBuf) DBRecord(this)); }
            return (new DBRecord(this));
        }

        void DestroyDBRecord(DBRecord *);

    private:
        MYSQL   *m_SQL;
        bool     m_Valid;

    public:
        friend class DBRecord;
};
