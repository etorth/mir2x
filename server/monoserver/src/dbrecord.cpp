#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <mysql/mysql.h>

#include "dbrecord.hpp"
#include "dbconnection.hpp"

DBRecord::DBRecord(DBConnection * pConnection)
    : m_SQLRES(nullptr)
    , m_CurrentRow(nullptr)
    , m_Connection(pConnection)
    , m_ValidCmd(true) // if no cmd queried, we make true by default
    , m_QuerySucceed(false)
    , m_QueryBuf(128)
{}

bool DBRecord::Execute(const char *szQueryCmd, ...)
{
    m_QueryBuf.resize(128);

    va_list ap;
    while(true){
        va_start(ap, szQueryCmd);
        int nRes = std::vsnprintf(&(m_QueryBuf[0]), m_QueryBuf.size(), szQueryCmd, ap);
        va_end(ap);

        if(nRes >= 0){
            if((size_t)(nRes + 1) < m_QueryBuf.size()){ break; }
            else{ m_QueryBuf.resize(nRes + 1); }
        }else{
            m_ValidCmd = false;
            return false;
        }
    }

    return Query(&(m_QueryBuf[0])) && Valid() && StoreResult();
}

// TODO: we already put the query cmd in internal buffer, so here do I
//       need to put the argument szQueryCmd?
//
//       I was planning to make Query() public then we can do some
//       query without result store like
//
//          Query("use database mir2x");
//
//       but currently I haven't done it yet since I have no idea that
//       this is necessary or not
bool DBRecord::Query(const char *szQueryCmd)
{
    // 1. make a default false state
    m_QuerySucceed = false;

    // if we are using the internal buffer, we have to make sure the query
    // cmd inside is correct
    if((szQueryCmd == &(m_QueryBuf[0]))){
        if(!m_ValidCmd){ goto __DBRECORD_QUERY_DONE_QUERY_LABEL_1; }
    }

    // 2. check parameter
    if(!(szQueryCmd && (std::strlen(szQueryCmd) > 0))){
        goto __DBRECORD_QUERY_DONE_QUERY_LABEL_1;
    }

    // 3. validate the connection record
    if(!(m_Connection && m_Connection->m_SQL && m_Connection->Valid())){
        goto __DBRECORD_QUERY_DONE_QUERY_LABEL_1;
    }

    // ok now we can do the query

    // 4. query
    if(!mysql_query(m_Connection->m_SQL, szQueryCmd)){
        m_QuerySucceed = true;
        goto __DBRECORD_QUERY_DONE_QUERY_LABEL_1;
    }

__DBRECORD_QUERY_DONE_QUERY_LABEL_1:
    // here we already tried our best for this query
    return m_QuerySucceed;
}

bool DBRecord::Valid()
{
    // valid record set, means:
    // 1. proper connected
    // 2. executed successfully
    return true
        && m_Connection
        && m_Connection->m_SQL
        && m_Connection->Valid()
        && m_QuerySucceed;
}

bool DBRecord::StoreResult()
{
    if(m_SQLRES){
        mysql_free_result(m_SQLRES);
        m_SQLRES = nullptr;
    }

    return Valid() && ((m_SQLRES = mysql_store_result(m_Connection->m_SQL)) != nullptr);
}

bool DBRecord::Fetch()
{
    return true
        && Valid()                  // valid record
        && (m_SQLRES != nullptr)    // valid result set
        && ((m_CurrentRow = mysql_fetch_row(m_SQLRES)) != nullptr);  // don't have to free the row
}

const char *DBRecord::Get(const char *szColumnName)
{
    if(szColumnName && std::strlen(szColumnName)){
        if(m_CurrentRow){
            mysql_field_seek(m_SQLRES, 0);

            int nIndex = 0;
            MYSQL_FIELD *pCurrField = nullptr;
            while((pCurrField = mysql_fetch_field(m_SQLRES)) != nullptr){
                if((pCurrField->name) && (!std::strcmp(pCurrField->name, szColumnName))){
                    return m_CurrentRow[nIndex];
                }
                nIndex++;
            }
        }
    }
    return nullptr;
}

int DBRecord::RowCount()
{
    // only call this function after ``select"
    // for update, insert or other operations don't use it
    if(m_SQLRES){
        return (int)mysql_num_rows(m_SQLRES);
    }else{
        if(ColumnCount() >= 0){ return 0; }
    }
    return -1;
}

int DBRecord::ColumnCount()
{
    // mysql_store_result() sometimes returns nullptr even mysql_query() returns success
    // then we need to do more here, refer to:
    // http://dev.mysql.com/doc/refman/5.0/en/null-mysql-store-result.html

    if(m_QuerySucceed){
        if(m_SQLRES){
            // there are rows
            return (int)mysql_num_fields(m_SQLRES);
        }else{
            // mysql_store_result() returned nothing, but should it have?
            if(mysql_field_count(m_Connection->m_SQL) == 0){
                // query does not return data, it was not a SELECT
                return 0;
            }else{
                // mysql_store_result() should have returned data
                // report as error
                return -1;
            }
        }
    }

    return -1;
}

int DBRecord::ErrorID()
{
    if(m_ValidCmd){
        if(m_Connection){
            // -1, 0, ...
            return m_Connection->ErrorID();
        }else{
            // error in initialization
            return -2;
        }
    }else{
        return -3;
    }
}

const char *DBRecord::ErrorInfo()
{
    if(m_ValidCmd){
        if(m_Connection){
            // -1, 0, ...
            return m_Connection->ErrorInfo();
        }else{
            // error in initialization
            return "null connection pointer in current record";
        }
    }else{
        return "error in parsing execute command in vsnprintf()";
    }
}
