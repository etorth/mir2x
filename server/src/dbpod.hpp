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
    private:
        std::unique_ptr<SQLite::Database> m_dbPtr;

    public:
        DBPod() = default;

    public:
        SQLite::Statement createQuery(const char *format, ...)
        {
            checkDBEx();
            std::string s;
            str_format(format, s);
            return SQLite::Statement(*m_dbPtr, s);
        }

        SQLite::Statement createQuery(const char8_t *format, ...)
        {
            checkDBEx();
            std::u8string s;
            str_format(format, s);
            return SQLite::Statement(*m_dbPtr, to_cstr(s));
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
