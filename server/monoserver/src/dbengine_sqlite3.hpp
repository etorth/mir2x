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
#include <cstring>
#include <stdexcept>
#include <sqlite3.h>
#include "strf.hpp"
#include "toll.hpp"
#include "fflerror.hpp"

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
                throw fflerror("sqlite3_open_v2(\"%s\") failed with ec = %d, errmsg: %s", szDBName, nRC, szErrorMsg.c_str());
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
        friend class DBRecord_SQLite3;
};

class DBRecord_SQLite3: public DBRecord
{
    private:
        friend class DBConnection;
        friend class DBEngine_SQLite3;

    private:
        DBEngine_SQLite3 *m_DBEngine;

    private:
        sqlite3_stmt *m_statement;

    private:
        void Prepare(const std::string &szQueryCmd)
        {
            if(m_statement){
                Finalize();
            }

            if(int nRC = sqlite3_prepare_v2(m_DBEngine->m_SQLite3, szQueryCmd.c_str(), szQueryCmd.length(), &m_statement, nullptr); nRC != SQLITE_OK){
                throw fflerror("sqlite3_prepare_v2(\"%s\") failed, errcode = %d, errmsg: %s", szQueryCmd.c_str(), nRC, sqlite3_errmsg(m_DBEngine->m_SQLite3));
            }
        }

    private:
        void Finalize()
        {
            if(!m_statement){
                return;
            }

            if(auto nRC = sqlite3_finalize(m_statement); nRC != SQLITE_OK){
                throw fflerror("sqlite3_finalize(%p) failed with errcode: %d", to_cvptr(m_statement), nRC);
            }
            m_statement = nullptr;
        }

    private:
        DBRecord_SQLite3(DBConnection *pConnection)
            : m_DBEngine(dynamic_cast<DBEngine_SQLite3 *>(pConnection))
            , m_statement(nullptr)
        {
            if(!m_DBEngine){
                throw fflerror("DBEngine_SQLite3(%p) failed", to_cvptr(pConnection));
            }
        }

    public:
        ~DBRecord_SQLite3() override
        {
            try{
                Finalize();
            }
            catch(const std::exception &e){
                //
            }
            catch(...){
                //
            }
        }

    public:
        DBRecord::DBDataType GetData(const char *szColumnName) override
        {
            if(!szColumnName){
                throw fflerror("invalid argument: (nullptr)");
            }

            if(std::strlen(szColumnName) == 0){
                throw fflerror("invalid argument: \"\"");
            }

            if(!m_statement){
                throw fflerror("no valid statement");
            }

            const int nDataCount   = sqlite3_data_count(m_statement);
            const int nColumnCount = sqlite3_column_count(m_statement);

            if(nDataCount <= 0){
                throw fflerror("invalid data count: %d", nDataCount);
            }

            if(nDataCount != nColumnCount){
                throw fflerror("invalid data column: %d", nColumnCount);
            }

            for(int nIndex = 0; nIndex < nColumnCount; ++nIndex){
                if(auto szName = sqlite3_column_name(m_statement, nIndex); std::strcmp(szName, szColumnName) == 0){
                    switch(auto nType = sqlite3_column_type(m_statement, nIndex)){
                        case SQLITE_INTEGER:
                            {
                                return (int64_t)(sqlite3_column_int64(m_statement, nIndex));
                            }
                        case SQLITE_FLOAT:
                            {
                                return (double)(sqlite3_column_double(m_statement, nIndex));
                            }
                        case SQLITE_TEXT:
                            {
                                return to_u8cstr(sqlite3_column_text(m_statement, nIndex));
                            }
                        default:
                            {
                                throw fflerror("data type %d not supported, should be int64_t, double, string", nType);
                            }
                    }
                }
            }
            throw fflerror("can't find given column name: %s", szColumnName);
        }

    public:
        bool QueryResult(const char *szQueryCmd, ...) override
        {
            va_list ap;
            va_start(ap, szQueryCmd);

            std::string szQueryFullCmd;
            try{
                szQueryFullCmd = str_vprintf(szQueryCmd, ap);
                va_end(ap);
            }catch(...){
                va_end(ap);
                throw fflerror("call str_vprintf(%s) failed", szQueryCmd);
            }

            Prepare(szQueryFullCmd);
            return Fetch();
        }

    public:
        bool Fetch()
        {
            if(!m_statement){
                throw fflerror("fetch() on empty statement");
            }

            switch(auto nRC = sqlite3_step(m_statement)){
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
                        throw fflerror("sqlite3_step(%p) failed: %d", to_cvptr(m_statement), nRC);
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
            return sqlite3_column_count(m_statement);
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
