/*
 * =====================================================================================
 *
 *       Filename: dbengine_sqlite3.hpp
 *        Created: 01/22/2019 07:42:12
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
#include <stdexcept>
#include <sqlite3.h>
#include "strfunc.hpp"
#include "dtorcrashlog.hpp"

class DBRecord_SQLite3;
class DBEngine_SQLite3: public DBConnection
{
    private:
        sqlite3 *m_SQLite3;

    public:
        DBEngine_SQLite3(const char *szDBName)
            : m_SQLite3(nullptr)
        {
            if(auto nRC = sqlite3_open_v2(szDBName, &m_SQLite3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr); nRC != SQLITE_OK){
                std::string szErrorMsg = sqlite3_errmsg(m_SQLite3);
                sqlite3_close_v2(m_SQLite3);
                throw std::runtime_error(str_fflprintf(": sqlite3_open_v2(\"%s\") failed with ec = %d, errmsg: %s", szDBName, nRC, szErrorMsg.c_str()));
            }
        }

    public:
        ~DBEngine_SQLite3() override
        {
            sqlite3_close_v2(m_SQLite3);
        }

    public:
        const char *DBEngine() const override
        {
            return "sqlite3";
        }

    public:
        inline DBRecord *CreateDBRecord() override;
        inline void      DestroyDBRecord(DBRecord *) override;

    public:
        friend class DBRecord;
};

class DBRecord_SQLite3: public DBRecord
{
    private:
        DBEngine_SQLite3 *m_DBEngine;

    private:
        sqlite3_stmt *m_Statement;

    private:
        void Prepare(const char *szSQL, size_t nSQLLen)
        {
            if(m_Statement){
                Finalize();
            }

            if(int nRC = sqlite3_prepare_v2(m_SQLite3, szSQL, nSQLLen, &m_Statement, nullptr); nRC != SQLITE_OK){
                throw std::runtime_error(str_fflprintf(": sqlite3_prepare_v2(%s) failed, errcode = %d", szSQL, nRC));
            }
            m_StepCount = 0;
        }

    private:
        void Finalize()
        {
            if(!m_Statement){
                return;
            }

            if(auto nRC = sqlite3_finalize(m_Statement); nRC != SQLITE_OK){
                throw std::runtime_error(str_fflprintf("sqlite3_finalize(%p) failed with errcode: %d", m_Statement, nRC));
            }
            m_Statement = nullptr;
        }

    private:
        DBRecord_SQLite3(DBConnection *pConnection)
            : m_DBEngine(dynamic_cast<DBEngine_SQLite3 *>(pConnection))
            , m_Statement(nullptr)
        {
            if(!m_DBEngine){
                throw std::runtime_error(str_fflprintf(": DBEngine_SQLite3(%p) failed", pConnection));
            }
        }

    public:
        ~DBRecord_SQLite3() override
        {
            try{
                Finalize();
            }catch(const std::exception &e){
                //
            }catch(...){
                //
            }
        }

    public:
        void Execute(const char *szExecCmd, ...) override
        {
            va_list ap;
            va_start(ap, szExecCmd);

            std::string szRetStr;
            try{
                szRetStr = str_vprintf(szExecCmd, ap);
                va_end(ap);
            }catch(...){
                va_end(ap);
                throw std::runtime_error(str_fflprintf(": str_vprintf(\"%s\") failed", szExecCmd));
            }
            return Query(szRetStr.c_str());
        }

    public:
        DBRecord::DBDataType GetDataAt(size_t nIndex)
        {
            switch(auto nType = sqlite3_column_type(m_Statement, nIndex)){
                case SQLITE_INTEGER:
                    {
                        return (int64_t)(sqlite3_column_int64(m_Statement, nIndex));
                    }
                case SQLITE_FLOAT:
                    {
                        return (double)(sqlite3_column_double(m_Statement, nIndex));
                    }
                case SQLITE_TEXT:
                    {
                        return (const char *)(sqlite3_column_text(m_Statement, nIndex));
                    }
                default:
                    {
                        throw std::runtime_error(str_fflprintf(": Data type %d not supported, should be int64_t, double, string", nType));
                    }
            }
        }

        DBRecord::DBDataType GetData(const char *szColumnName) override
        {
            if(!szColumnName){
                throw std::invalid_argument(ffl_printf(": Invalid argument: (nullptr)"));
            }

            if(std::strlen(szColumnName) == 0){
                throw std::invalid_argument(ffl_printf(": Invalid argument: \"\""));
            }

            if(!m_Statement){
                throw std::runtime_error(str_fflprintf(": No valid statement"));
            }

            int nDataCount   = sqlite3_data_count(m_Statement);
            int nColumnCount = sqlite3_column_count(m_Statement);

            if(nDataCount <= 0){
                throw;
            }

            if(nDataCount != nColumnCount){
                throw;
            }

            for(int nIndex = 0; nIndex < nColumnCount; ++nIndex){
                if(auto szName = sqlite3_column_name(m_Statement, nIndex); std::strcmp(szName, szColumnName) == 0){
                    switch(auto nType = sqlite3_column_type(m_Statement, nIndex)){
                        case SQLITE_INTEGER:
                            {
                                return (int64_t)(sqlite3_column_int64(m_Statement, nIndex));
                            }
                        case SQLITE_FLOAT:
                            {
                                return (double)(sqlite3_column_double(m_Statement, nIndex));
                            }
                        case SQLITE_TEXT:
                            {
                                return (const char *)(sqlite3_column_text(m_Statement, nIndex));
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Data type %d not supported, should be int64_t, double, string", nType));
                            }
                    }
                }
            }
        }

    public:
        bool QueryResult(const char *szQueryCmd)
        {
            Prepare();
            Fetch();
        }

    public:
        bool Fetch()
        {
            m_QueryResult = sqlite3_step();

            switch(m_QueryResult){
                case SQLITE_ROW:
                    {
                        return true;
                    }
                case SQLITE_DONE:
                    {
                        return false;
                    }
                default:
                    {
                        throw std::runtime_error(str_fflprintf(": sqlite3_step(%s) failed: %d", m_QueryResult));
                    }
            }
        }

    public:
        int RowCount() override
        {
            return -1;
        }

        int ColumnCount() override
        {
            return sqlite3_column_count(m_Statement);
        }
};

DBRecord *DBEngine_SQLite3::CreateDBRecord()
{
    return new DBRecord_SQLite3(this);
}

void DBEngine_SQLite3::DestroyDBRecord(DBRecord *pDBRecord)
{
    delete pDBRecord;
}
