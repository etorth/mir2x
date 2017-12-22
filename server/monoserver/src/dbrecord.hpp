#pragma once
#include <vector>
#include <mysql/mysql.h>

class DBConnection;
class DBRecord final
{
    private:
        MYSQL_RES      *m_SQLRES;
        MYSQL_ROW       m_CurrentRow;
        DBConnection   *m_Connection;

    private:
        bool m_ValidCmd;
        bool m_QuerySucceed;
        std::vector<char> m_QueryBuf;

    private:
        DBRecord(DBConnection *);
       ~DBRecord() = default;

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
        bool Query(const char *);
        bool StoreResult();

    public:
        friend class DBConnection;
};
