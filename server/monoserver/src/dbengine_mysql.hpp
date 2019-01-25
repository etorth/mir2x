/*
 * =====================================================================================
 *
 *       Filename: dbengine_mysql.hpp
 *        Created: 01/21/2019 07:24:10
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
#include "dbbase.hpp"
#include "mysqlinc.hpp"

class DBRecord_MySQL;
class DBEngine_MySQL: public DBConnection
{
    private:
        friend class DBRecord;
        friend class DBRecord_MySQL;

    private:
        MYSQL m_SQL;

    public:
        DBEngine_MySQL(const char *szHostName, const char *szUserName, const char *szPassword, const char *szDBName, unsigned int nPort)
            : DBConnection()
        {
            if(auto p = mysql_init(&m_SQL); p != &m_SQL){
                throw std::runtime_error(str_fflprintf(": mysql_init(%p) returns %p", &m_SQL, p));
            }

            mysql_options(&m_SQL, MYSQL_SET_CHARSET_NAME, "utf8"); 
            mysql_options(&m_SQL, MYSQL_INIT_COMMAND, "SET NAMES utf8"); 

            if(!mysql_real_connect(&m_SQL, szHostName, szUserName, szPassword, szDBName, nPort, nullptr, 0)){
                throw std::runtime_error(str_fflprintf(": mysql_real_connect() failed: %s", mysql_error(&m_SQL)));
            }
        }

    public:
        ~DBEngine_MySQL() override
        {
            mysql_close(&m_SQL);
        }

    public:
        const char *DBEngine() const override
        {
            return "mysql";
        }

    public:
        inline DBRecord *CreateDBRecord() override;

    public:
        inline void DestroyDBRecord(DBRecord *) override;
};

class DBRecord_MySQL: public DBRecord
{
    private:
        friend class DBConnection;
        friend class DBEngine_MySQL;

    private:
        MYSQL_RES *m_SQLRES;
        MYSQL_ROW  m_CurrentRow;

    private:
        DBEngine_MySQL *m_DBEngine;

    private:
        DBRecord_MySQL(DBConnection *pConnection)
            : m_SQLRES(nullptr)
            , m_CurrentRow(nullptr)
            , m_DBEngine(dynamic_cast<DBEngine_MySQL *>(pConnection))
        {
            if(!m_DBEngine){
                throw std::runtime_error(str_fflprintf(": DBEngine_MySQL(%p) failed", pConnection));
            }
        }

    public:
        ~DBRecord_MySQL() override
        {
            if(m_SQLRES){
                mysql_free_result(m_SQLRES);
            }

            m_SQLRES     = nullptr;
            m_CurrentRow = nullptr;
            m_DBEngine   = nullptr;
        }

    public:
        DBDataType GetData(const char *szColumnName) override
        {
            if(!szColumnName){
                throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
            }

            if(std::strlen(szColumnName) == 0){
                throw std::invalid_argument(str_fflprintf(": Invalid argument: \"\""));
            }

            if(!m_CurrentRow){
                throw std::runtime_error(str_fflprintf(": No valid row"));
            }

            mysql_field_seek(m_SQLRES, 0);

            int nIndex = 0;
            MYSQL_FIELD *pCurrField = nullptr;
            while((pCurrField = mysql_fetch_field(m_SQLRES)) != nullptr){
                if((pCurrField->name) && (std::strcmp(pCurrField->name, szColumnName) == 0)){
                    switch(pCurrField->type){
                        case MYSQL_TYPE_TINY:
                        case MYSQL_TYPE_SHORT:
                        case MYSQL_TYPE_LONG:
                        case MYSQL_TYPE_LONGLONG:
                            {
                                return (int64_t)(std::stoll(m_CurrentRow[nIndex]));
                            }
                        case MYSQL_TYPE_FLOAT:
                        case MYSQL_TYPE_DOUBLE:
                            {
                                return std::stod(m_CurrentRow[nIndex]);
                            }
                        case MYSQL_TYPE_STRING:
                            {
                                return std::string(m_CurrentRow[nIndex]);
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Field type not supported: %d", pCurrField->type));
                            }
                    }
                }
                nIndex++;
            }
            throw std::invalid_argument(str_fflprintf(": Can't find given column name: %s", szColumnName));
        }

    private:
        bool StoreResult()
        {
            if(m_SQLRES){
                mysql_free_result(m_SQLRES);
                m_SQLRES = nullptr;
            }

            // this result could be empty
            // see comments in DBRecord_MySQL::ColumnCount()

            m_SQLRES = mysql_store_result(&(m_DBEngine->m_SQL));
            if(m_SQLRES){
                return true;
            }else{
                return mysql_field_count(&(m_DBEngine->m_SQL)) == 0;
            }
        }

    public:
        bool QueryResult(const char *szQueryCmd, ...) override
        {
            va_list ap;
            va_start(ap, szQueryCmd);

            std::string szRetStr;
            try{
                szRetStr = str_vprintf(szQueryCmd, ap);
                va_end(ap);
            }catch(...){
                va_end(ap);
                throw std::runtime_error(str_fflprintf(": Call str_vprintf(%s) failed", szQueryCmd));
            }

            return Query(szRetStr.c_str()) && StoreResult() && Fetch();
        }

    private:
        bool Query(const char *szQueryCmd)
        {
            if(mysql_query(&(m_DBEngine->m_SQL), szQueryCmd)){
                throw std::runtime_error(str_fflprintf(": mysql_query(%s) failed: %s", szQueryCmd, mysql_error(&(m_DBEngine->m_SQL))));
            }
            return true;
        }

    public:
        bool Fetch() override
        {
            if(!m_SQLRES){
                throw std::runtime_error(str_fflprintf(": Call Fetch() before QueryResult(QueryCmd, ...)"));
            }
            return (m_CurrentRow = mysql_fetch_row(m_SQLRES)) != nullptr;
        }

    public:
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
                if(mysql_field_count(&(m_DBEngine->m_SQL)) == 0){
                    // query does not return data, it was not a SELECT
                    return 0;
                }else{
                    // mysql_store_result() should have returned data
                    // report as error
                    return -1;
                }
            }
        }
};

DBRecord *DBEngine_MySQL::CreateDBRecord()
{
    return new DBRecord_MySQL(this);
}

void DBEngine_MySQL::DestroyDBRecord(DBRecord *pDBRecord)
{
    delete pDBRecord;
}
