#pragma once
#include <mariadb/mysql.h>

class DBConnection;
class DBRecord final
{
    private:
        MYSQL_RES      *m_SQLRES;
        MYSQL_ROW       m_CurrentRow;
        DBConnection   *m_Connection;

    private:
        char   *m_QueryBuf;
        size_t  m_QueryBufLen;
        bool    m_ValidCmd;
        bool    m_QuerySucceed;

    private:
        DBRecord(DBConnection *);
        ~DBRecord();

    public:
        bool Execute(const char *, ...);
        bool Valid();
        bool Fetch();
        int  RowCount();
        int  ColumnCount();

    public:
        const char *Get(const char *);

    public:
        int ErrorID();
        const char *ErrorInfo();

    private:
        void ExtendQueryBuf(size_t);
        bool Query(const char *);
        bool StoreResult();

    public:
        friend class DBConnection;
};
