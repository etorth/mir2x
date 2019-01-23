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
#include "dtorcrashlog.hpp"

class DBRecord_SQLite3;
class DBEngine_SQLite3: public DBConnection
{
    private:
        sqlite3 *m_SQLite3;

    public:
        DBConnection_SQLite3(const char *szDBName)
            : m_SQLite3(nullptr)
        {
            if(auto nRC = sqlite3_open_v2(szDBName, &m_SQLite3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE); nRC != SQLITE_OK){
                sqlite3_close_v2(m_SQLite3);
                throw std::runtime_error(str_fflprintf(": sqlite3_open_v2(\"%s\") failed with ec = %d, errmsg: %s", szDBName, ));
            }
        }

    public:
        ~DBConnection_SQLite3() override
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

    public:
        void DestroyDBRecord(DBRecord *pDBRecord) override
        {
            delete pDBRecord;
        }

    public:
        friend class DBRecord;
};

class DBRecord_SQLite3: public DBRecord
{
    private:
        MYSQL_RES *m_SQLRES;
        MYSQL_ROW  m_CurrentRow;

    private:
        DBEngine_SQLite3 *m_DBEngine;

    private:
        struct StatementFinalizer final
        {
            sqlite3_stmt *m_Statement;
            StatementFinalizer(sqlite3_stmt *pStatement)
                : m_Statement(pStatement)
            {}

            ~StatementFinalizer()
            {
                if(auto nRC = sqlite3_finalize(m_Statement); nRC != SQLITE_OK){
                    DtorCrashLog("sqlite3_finalize(%p) failed with errcode: %d", m_Statement, nRC);
                }
            }
        };

    private:
        DBRecord_SQLite3(DBConnection *pConnection)
            : m_SQLRES(nullptr)
            , m_CurrentRow(nullptr)
            , m_DBEngine(dynamic_cast<DBEngine_SQLite3 *>(pConnection))
        {
            if(!m_DBEngine){
                throw std::runtime_error(str_fflprintf(": DBEngine_SQLite3(%p) failed", pConnection));
            }
        }

    public:
        ~DBRecord_SQLite3() override
        {
            if(m_SQLRES){
                mysql_free_result(m_SQLRES);
            }

            m_SQLRES     = nullptr;
            m_CurrentRow = nullptr;
            m_DBEngine   = nullptr;
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
       DBDataType GetData(const char *szColumnName) override
       {
           if(!szColumnName){
               throw std::invalid_argument(ffl_printf(": Invalid argument: (nullptr)"));
           }

           if(std::strlen(szColumnName) == 0){
               throw std::invalid_argument(ffl_printf(": Invalid argument: \"\""));
           }

           if(!m_Statement){
               throw;
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
                   switch(auto nType = sqlite3_column_type()){
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

    private:
       void Query(const char *szQueryCmd)
       {
           char *pszErrMsg = nullptr;
           if(auto nRC = sqlite3_exec(m_Connection->m_SQLite3, szSQL, nullptr, nullptr, &pszErrMsg); (nRC != SQLITE_OK) || pszErrMsg){
               std::string szErrMsg {pszErrMsg ? pszErrMsg : ""};
               sqlite3_free(pszErrMsg);
               throw std::runtime_error(str_fflprintf(": sqlite3_exec(\"%s\") failed with errcode = %d, errmsg: %s", szSQL, nRC, szErrMsg.empty() ? "unknown" : szErrMsg.c_str()));
           }
       }

    private:
        bool StoreResult()
        {
            if(m_SQLRES){
                mysql_free_result(m_SQLRES);
                m_SQLRES = nullptr;
            }

            // this result could be empty
            // see comments in DBRecord_SQLite3::ColumnCount()

            m_SQLRES = mysql_store_result(m_DBEngine->m_SQL);
            if(m_SQLRES){
                return true;
            }else{
                return mysql_field_count(m_DBEngine->m_SQL) == 0;
            }
        }

        bool DBImpl::DBRecord_SQLite3::QueryResult(const std::string &szSQL)
        {
            auto nRC = sqlite3_prepare_v2(m_DBConnectioin->m_SQLite3, szSQL.data(), (int)(szSQL.size()), &m_Statement, nullptr);
            if(nRC != SQLITE_OK){
                return false;
            }
        }

    public:
        bool DBImpl::DBRecord_SQLite3::Fetch()
        {
            switch(auto nRC = sqlite3_step(m_Statement)){
                case SQLITE_DONE:
                    {
                        return false;
                    }
                case SQLITE_ROW:
                    {
                        return true;
                    }
                default:
                    {
                        throw std::runtime_error(str_fflprintf(": sqlite3_step() failed: errcode = %d", nRC));
                    }
            }
        }

        int RowCount() override
        {
            if(m_SQLRES){
                return (int)(mysql_num_rows(m_SQLRES));
            }else{
                if(ColumnCount() >= 0){
                    return 0;
                }else{
                    return -1;
                }
            }
        }

        int ColumnCount() override
        {
            // mysql_store_result() sometimes returns nullptr even mysql_query() returns success
            // then we need to do more here, refer to:
            // http://dev.mysql.com/doc/refman/5.0/en/null-mysql-store-result.html

            if(m_SQLRES){
                return (int)(mysql_num_fields(m_SQLRES));
            }else{
                // mysql_store_result() returned nothing, but should it have?
                if(mysql_field_count(m_DBEngine->m_SQL) == 0){
                    // query does not return data, it was not a SELECT
                    return 0;
                }else{
                    // mysql_store_result() should have returned data
                    // report as error
                    return -1;
                }
            }
        }
}

DBRecord *DBEngine_SQLite3::CreateDBRecord()
{
    return new DBRecord_SQLite3(this);
}
