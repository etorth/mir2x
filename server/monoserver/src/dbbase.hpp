/*
 * =====================================================================================
 *
 *       Filename: dbbase.hpp
 *        Created: 01/21/2019 22:03:34
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

class DBRecord;
class DBConnection
{
    public:
        DBConnection() = default;
        virtual ~DBConnection() = default;

    public:
        virtual DBRecord *CreateDBRecord() = 0;
        virtual void      DestroyDBRecord(DBRecord *) = 0;

    public:
        virtual const char *DBEngine() const = 0;
};

class DBRecord
{
    public:
        DBRecord() = default;
        virtual ~DBRecord() = default;

    public:
        virtual void Execute(const char *, ...) = 0;
};

#if defined(MIR2X_ENABLE_SQLITE3)
    #include "dbengine_sqlite3.hpp"
#endif
