#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <mariadb/mysql.h>
#include "dbrecord.hpp"
#include "dbconnection.hpp"

DBRecord::DBRecord(DBConnection * pConnection)
    : m_SQLRES(nullptr)
    , m_CurrentRow(nullptr)
    , m_Connection(pConnection)
    , m_QueryBuf(nullptr)
    , m_QueryBufLen(0)
    , m_ValidExecuteString(true)
    , m_Valid(false)
    , m_QuerySucceed(false)
{}

DBRecord::~DBRecord()
{
    delete m_QueryBuf;
}

void DBRecord::ExtendQueryBuf(size_t nNewLen)
{
    if(nNewLen > m_QueryBufLen){
        delete m_QueryBuf;
        size_t nNewLen8 = ((nNewLen + 7) / 8) * 8;

        m_QueryBuf    = new char[nNewLen8];
        m_QueryBufLen = nNewLen8;
    }
}

bool DBRecord::Execute(const char *szQueryCmd, ...)
{
    ExtendQueryBuf(256);

    va_list ap;

    while(1){

        va_start(ap, szQueryCmd);

        int nRes = std::vsnprintf(m_QueryBuf, m_QueryBufLen, szQueryCmd, ap);

        va_end(ap);

        if(nRes > -1 && (size_t)nRes < m_QueryBufLen){
            // additional '\0' takes one char
            // everything works
            break;
        }else if(nRes < 0){
            // error occurs
            return false;
        }else{
            // we need a larger buffer
            ExtendQueryBuf(nRes + 1);
        }
    }

    return Query(m_QueryBuf) && Valid() && StoreResult();
}

bool DBRecord::Query(const char *szQueryCmd)
{
    // 1. make a default false state
    m_QuerySucceed = false;

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
    // 1. parameter check
    if(!(szColumnName && std::strlen(szColumnName))){ return nullptr; }

    // 2. check cursor
    if(!m_CurrentRow){ return nullptr; }

    // ok find the index of the field name

    // 3. reset the seek point
    mysql_field_seek(m_SQLRES, 0);

    int nIndex = 0;
    MYSQL_FIELD *stCurrentField = nullptr;
    while((stCurrentField = mysql_fetch_field(m_SQLRES)) != nullptr){
        if((stCurrentField->name) && (!std::strcmp(stCurrentField->name, szColumnName))){
            return m_CurrentRow[nIndex];
        }
        nIndex++;
    }

    // 5. ooop we didn't find it
    return nullptr;
}

int DBRecord::RowCount()
{
    // only call this function after ``select"
    // for update, insert or other operations don't use it
    if(m_SQLRES){
        return (int)mysql_num_rows(m_SQLRES);
    }else{
        if(ColumnCount() >= 0){
            return 0;
        }
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
    if(m_ValidExecuteString){
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

    // dead code here
    return 0;
}

const char *DBRecord::ErrorInfo()
{
    if(m_ValidExecuteString){
        if(m_Connection){
            // -1, 0, ...
            return m_Connection->ErrorInfo();
        }else{
            // error in initialization
            return "null connection pointer in record";
        }
    }else{
        return "error in parsing execute command in vsnprintf()";
    }

    // dead code
    return "no error";
}
