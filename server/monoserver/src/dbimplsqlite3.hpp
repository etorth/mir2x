/*
 * =====================================================================================
 *
 *       Filename: dbimplsqlite3.hpp
 *        Created: 01/21/2019 07:25:29
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

namespace DBImpl
{
    class DBConnection_SQLite3
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

            ~DBConnection_SQLite3()
            {
                sqlite3_close_v2(m_SQLite3);
            }
    };

    class DBRecord_SQLite3
    {
        private:
            friend class DBImpl::DBConnection_SQLite3;

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

        public:
        public:
            bool Query();
            bool QueryResult();


bool DBImpl::DBRecord_SQLite3::Query(const char *szSQL)
{
    char *pszErrMsg = nullptr;
    if(auto nRC = sqlite3_exec(m_Connection->m_SQLite3, szSQL, nullptr, nullptr, &pszErrMsg); (nRC != SQLITE_OK) || pszErrMsg){
        std::string szErrMsg {pszErrMsg ? pszErrMsg : ""};
        sqlite3_free(pszErrMsg);
        throw std::runtime_error(str_fflprintf(": sqlite3_exec(\"%s\") failed with errcode = %d, errmsg: %s", szSQL, nRC, szErrMsg.empty() ? "unknown" : szErrMsg.c_str()));
    }
}

bool DBImpl::DBRecord_SQLite3::QueryResult(const std::string &szSQL)
{
    auto nRC = sqlite3_prepare_v2(m_DBConnectioin->m_SQLite3, szSQL.data(), (int)(szSQL.size()), &m_Statement, nullptr);
    if(nRC != SQLITE_OK){
        return false;
    }
}

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

const char *DBImpl::DBRecord_SQLite3::Get(const char *szColumnName)
{
    if(!szColumnName){
        return nullptr;
    }
}

    };
}
