#pragma once
#include <mutex>
#include <thread>
#include <memory>
#include <string>
#include <atomic>
#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

class DBPod final
{
    public:
        class Statement: public SQLite::Statement
        {
            public:
                using SQLite::Statement::Statement;

            public:
                void bindBlob(const int  index, const void *data, size_t size) { bindBlob(index, as_sv(data, size)); }
                void bindBlob(const char *name, const void *data, size_t size) { bindBlob( name, as_sv(data, size)); }

            public:
                void bindBlob(const int  index, const std::string_view &data) { SQLite::Statement::bind(index, to_cvptr(data.data()), data.size()); }
                void bindBlob(const char *name, const std::string_view &data) { SQLite::Statement::bind( name, to_cvptr(data.data()), data.size()); }

            public:
                void bindBlob(const char *name, const std::string &data) { SQLite::Statement::bind(name, to_cvptr(data.data()), data.size()); }
                void bindBlob(const int  index, const std::string &data) { SQLite::Statement::bind(index, to_cvptr(data.data()), data.size()); }
        };

    public:
        static_assert(sizeof(DBPod::Statement) == sizeof(SQLite::Statement));

    private:
        std::unique_ptr<SQLite::Database> m_dbPtr;

    public:
        DBPod() = default;

    public:
        DBPod::Statement createQuery(const char *format, ...)
        {
            checkDBEx();
            std::string s;
            str_format(format, s);
            return DBPod::Statement(*m_dbPtr, s);
        }

        DBPod::Statement createQuery(const char8_t *format, ...)
        {
            checkDBEx();
            std::u8string s;
            str_format(format, s);
            return DBPod::Statement(*m_dbPtr, to_cstr(s));
        }

    public:
        SQLite::Transaction createTransaction()
        {
            checkDBEx();
            return SQLite::Transaction(*m_dbPtr);
        }

    public:
        void launch(const char *dbName)
        {
            m_dbPtr = std::make_unique<SQLite::Database>(dbName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        }

    public:
        int exec(const char *format, ...)
        {
            checkDBEx();
            std::string s;
            str_format(format, s);
            return m_dbPtr->exec(s);
        }

        int exec(const char8_t *format, ...)
        {
            checkDBEx();
            std::u8string s;
            str_format(format, s);
            return m_dbPtr->exec(to_cstr(s));
        }

    private:
        void checkDBEx() const
        {
            if(!m_dbPtr){
                throw fflerror("no SQLite3 database opened");
            }
        }
};
