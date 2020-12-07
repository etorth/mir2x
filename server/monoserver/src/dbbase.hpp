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
#include <string>
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
        using DBDataType = std::variant<int64_t, double, std::u8string>;

    public:
        DBRecord() = default;
        virtual ~DBRecord() = default;

    public:
        // query with given sql statement, return true if result available
        //
        //      if(!hdl->QueryResult("select * where fld_id = \"%s\"", szId.c_str())){
        //          addLog("No record valid for id = %s", szId.c_str());
        //      }else{
        //          do{
        //              int64_t     id   = hdl->Get<int64_t>("fld_id");
        //              std::string name = hdl->Get<std::string>("fld_name");
        //              ...
        //          }while(hdl->Fetch());
        //      }
        //
        // if get result this function returns true and valid for Get()
        // if query failed it throws
        virtual bool QueryResult(const char *, ...) = 0;

    public:
        virtual bool Fetch() = 0;

    public:
        template<typename T> T Get(const char *szName)
        {
            return std::get<T>(GetData(szName));
        }

    public:
        // query row/column for query result
        // output:
        //    <  0: can't have a well implementation but no error happens
        //    >= 0: rows/columns
        //   throw: error
        virtual int RowCount() = 0;
        virtual int ColumnCount() = 0;

    protected:
        virtual DBDataType GetData(const char *) = 0;
};
