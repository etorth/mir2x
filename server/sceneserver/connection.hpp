#pragma once
#include <mysql.h>
#include "record.hpp"

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
        DBRecord *CreateDBRecord();
        void DestroyDBRecord(DBRecord *);

    private:
        MYSQL   *m_SQL;
        bool     m_Valid;

    public:
        friend class DBRecord;
};
