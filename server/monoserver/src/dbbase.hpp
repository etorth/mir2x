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

#pragma once
#include <cstdint>
#include <variant>

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
    protected:
        using DBDataType = std::variant<int64_t, double, const char *>;

    public:
        DBRecord() = default;
        virtual ~DBRecord() = default;

    public:
        virtual void Execute(const char *, ...) = 0;

    public:
        virtual DBDataType GetData(const char *) = 0;

    public:
        template<> int64_t Get<int64_t>(const char *szName)
        {
            return GetInt64(szName);
        }

        template<> double Get<double>(const char *szName)
        {
            return GetDouble(szName);
        }

        template<> const char * Get<const char *>(const char *szName)
        {
            return GetString(szName);
        }

        template<> std::string Get<std::string>(const char *szName)
        {
            return std::string(GetString(szName));
        }

    public:
        template<typename T> T Get<T>(const char *);
};

#if defined(MIR2X_ENABLE_SQLITE3)
    #include "dbengine_sqlite3.hpp"
#endif
