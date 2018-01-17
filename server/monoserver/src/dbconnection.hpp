/*
 * =====================================================================================
 *
 *       Filename: dbconnection.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 01/16/2018 19:13:54
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
#include "mysqlinc.hpp"
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
        DBConnection(const char *, const char *, const char *, const char *, unsigned int);
       ~DBConnection();

    public:
        bool Valid(){ return m_Valid; }

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
