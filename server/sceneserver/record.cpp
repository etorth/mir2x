#pragma once
#include <cstring>
#include <mysql.h>
#include "record.hpp"
#include "connection.hpp"

DBRecord::DBRecord(DBConnection * pConnection)
    : m_SQLRES(nullptr)
    , m_Connection(pConnection)
    , m_CurrentRow(nullptr)
    , m_Valid(false)
    , m_QuerySucceed(false)
{}

DBRecord::~DBRecord()
{}

bool DBRecord::Execute(const char *szQueryCmd)
{
    return Query(szQueryCmd) && Valid() && StoreResult();
}

bool DBRecord::Query(const char *szQueryCmd)
{
    // szQueryCmd should be ``valid"
    // since this function won't check it
    if(m_Connection && m_Connection->m_SQL){
        if(!mysql_query(m_Connection->m_SQL, szQueryCmd)){
            m_QuerySucceed = true;
			return true;
        }
    }
    m_QuerySucceed = false;
	return false;
}

bool DBRecord::Valid()
{
    // valid record set, means:
    // 1. proper connected
    // 2. executed successfully
    return true
        && m_Connection
        && m_Connection->m_SQL
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
    return (m_CurrentRow = mysql_fetch_row(m_SQLRES)) != nullptr;
}

const char *DBRecord::Get(const char *szColumnName)
{
    if(m_CurrentRow){
        int nIndex = 0;
        MYSQL_FIELD *stCurrentField = nullptr;

        mysql_field_seek(m_SQLRES, 0);

        while((stCurrentField = mysql_fetch_field(m_SQLRES)) != nullptr){
            if(!std::strcmp(stCurrentField->name, szColumnName)){
                return m_CurrentRow[nIndex];
            }
            nIndex++;
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
}

int DBRecord::ErrorID()
{
    if(m_Connection){
        return m_Connection->ErrorID();
    }
    return 0;
}

const char *DBRecord::ErrorInfo()
{
    if(m_Connection){
        return m_Connection->ErrorInfo();
    }
    return "";
}
