#pragma once
#include <mysql.h>

class DBRecord;

class DBConnection
{
    // don't use it to create new database
    // this should be done by me manually
    //
    // DBConnection only query information from current database
    // then each DBConnection has a specified database name
    //
    public:
        DBConnection(
                const char * szHostName,
                const char * szUserName,
                const char * szPassword,
                const char * szDBName,
                unsigned int nPort)
            : m_SQL(nullptr)
            , m_Valid(false)
        {
            m_SQL = mysql_init(nullptr);
            if(m_SQL){
                if(mysql_real_connect(m_SQL, szHostName,
                            szUserName, szPassword,szDBName, nPort, nullptr, 0)){
                    m_Valid = true;
                }
            }
            m_Valid = false;
        }

        ~DBConnection()
        {
            if(m_SQL){
                mysql_close(m_SQL);
            }
        }

    public:
        bool    Valid();

    public:
        DBRecord *CreateDBRecord()
        {
            return (new DBRecord(this));
        }
        void DestroyDBRecord(DBRecord *pDBRecord)
        {
            delete pDBRecord;
        }

    private:
        MYSQL   *m_SQL;
        bool     m_Valid;
};

class DBRecord
{
    private:
        DBRecord(DBConnection * pConnection)
            : m_SQLRES(nullptr)
            , m_Connection(pConnection)
            , m_CurrentRow(nullptr)
            , m_Valid(false)
        {}
        ~DBRecord();

    private:
        MYSQL_RES       *m_SQLRES;
        DBConnection    *m_Connection;
        MYSQL_ROW       *m_CurrentRow;

    private:
        bool    m_Valid;

    public:
        bool Execute(const char *szQueryCmd)
        {
            return Query(szQueryCmd) && Valid() && StoreResult();
        }

        bool Query(const char *szQueryCmd)
        {
            // szQueryCmd should be ``valid"
            // since this function won't check it
            if(m_Connection && m_Connection->m_SQL){
                if(!mysql_query(m_Connection->m_SQL, szQueryCmd)){
                    m_QuerySucceed = true;
                }
            }
            m_QuerySucceed = false;
            return m_QuerySucceed;
        }

        bool Valid()
        {
            // valid record set, means:
            // 1. proper connected
            // 2. executed successfully
            return true
                && m_Connection
                && m_Connection->m_SQL
                && m_QuerySucceed;
        }

        bool StoreResult()
        {
            if(m_SQLRES){
                mysql_free_result(m_SQLRES);
                m_SQLRES = nullptr;
            }

            return Valid() && ((m_SQLRES = mysql_store_result(m_Connection->m_SQL)) != nullptr);
        }

        bool Fetch()
        {
            return (m_CurrentRow = mysql_fetch_row(m_SQLRES)) != nullptr;
        }

        const char *Get(const char *szColumnName)
        {
            if(m_CurrentRow){
                int nIndex = 0;
                MYSQL_FIELD *stCurrentField = nullptr;

                mysql_field_seek(m_SQLRES, 0);

                while((stCurrentField = mysql_fetch_field()) != nullptr){
                    if(!std::strcmp(stCurrentField->name, szColumnName)){
                        return m_CurrentRow[nIndex];
                    }
                    nIndex++;
                }
            }
            return nullptr;
        }

        int RowCount()
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

        int ColumnCount()
        {
            // mysql_store_result() sometimes returns nullptr even mysql_query() returns success
            // then we need to do more here, refer to:
            // http://dev.mysql.com/doc/refman/5.0/en/null-mysql-store-result.html

            if(m_QuerySucceed){
                if(m_SQLRES){
                    // there are rows
                    return (int)mysql_num_fields(result);
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

        int ErrorID()
        {
            if(m_Connection){
                return m_Connection->ErrorID();
            }
            return 0;
        }

        const char *ErrorInfo()
        {
            if(m_Connection){
                return m_Connection->ErrorInfo();
            }
            return "";
        }

    public:
        friend class DBConnection;
};
