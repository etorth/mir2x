/*
 * =====================================================================================
 *
 *       Filename: dbpod.hpp
 *        Created: 05/20/2016 14:31:19
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
#include <mutex>
#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include "strf.hpp"
#include "database.hpp"

template<size_t ConnectionCount = 4> class DBPodConnector final
{
    private:
        class InnDeleter
        {
            private:
                DBConnection *m_DBConnection;

            private:
                std::mutex *m_lock;

            public:
                InnDeleter(DBConnection *pDBConnection = nullptr, std::mutex *pLock = nullptr)
                    : m_DBConnection(pDBConnection)
                    , m_lock(pLock)
                {}

                ~InnDeleter() = default;

            public:
                void operator()(DBRecord *pRecord)
                {
                    if(!pRecord){
                        return;
                    }

                    if(m_DBConnection){
                        m_DBConnection->DestroyDBRecord(pRecord);
                    }

                    if(ConnectionCount > 1 && m_lock){
                        m_lock->unlock();
                    }
                    m_lock = nullptr;
                }
        };

    public:
        using DBHDR = std::unique_ptr<DBRecord, InnDeleter>;

    private:
        std::atomic<size_t> m_current;

    private:
        std::mutex                    m_connLockVec[ConnectionCount];
        std::unique_ptr<DBConnection> m_connVec    [ConnectionCount];

    public:
        DBPodConnector()
            : m_current {0}
        {
            static_assert(ConnectionCount > 0, "DBPodConnector should contain at least one connection handler");
        }

        void LaunchMySQL(
                [[maybe_unused]] const char *szHostName,
                [[maybe_unused]] const char *szUserName,
                [[maybe_unused]] const char *szPassword,
                [[maybe_unused]] const char *szDBName,
                [[maybe_unused]] unsigned int nPort)
        {
#if defined(MIR2X_ENABLE_MYSQL)
            for(size_t nIndex = 0; nIndex < ConnectionCount; ++nIndex){
                m_connVec[nIndex] = std::make_unique<DBEngine_MySQL>(szHostName, szUserName, szPassword, szDBName, nPort);
            }
#else
            throw fflerror("launchMySQL(...) not supported in current build");
#endif
        }

        void LaunchSQLite3([[maybe_unused]] const char *szDBName)
        {
#if defined(MIR2X_ENABLE_SQLITE3)
            for(size_t nIndex = 0; nIndex < ConnectionCount; ++nIndex){
                m_connVec[nIndex] = std::make_unique<DBEngine_SQLite3>(szDBName);
            }
#else
            throw fflerror("launchSQLite3(...) not supported in current build");
#endif
        }

        DBHDR InnCreateDBHDR(size_t nPodIndex)
        {
            if(auto p = m_connVec[nPodIndex]->CreateDBRecord()){
                return DBHDR(p, InnDeleter(m_connVec[nPodIndex].get(), m_connLockVec + nPodIndex));
            }
            return DBHDR(nullptr, InnDeleter());
        }

        DBHDR CreateDBHDR()
        {
            if(ConnectionCount == 1){
                return InnCreateDBHDR(0);
            }

            // call try_lock here
            // will be unlocked if this DBHDR get constructed

            for(size_t nIndex = m_current.fetch_add(1) % ConnectionCount;; nIndex = (nIndex + 1) % ConnectionCount){
                if(m_connLockVec[nIndex].try_lock()){
                    return InnCreateDBHDR(nIndex);
                }
            }
            return DBHDR(nullptr, InnDeleter()); // never reach
        }

    public:
        static bool Enabled(const char *szDBEngineName)
        {
            if(std::strcmp(szDBEngineName, "mysql") == 0){
#if defined(MIR2X_ENABLE_MYSQL)
                return true;
#else
                return false;
#endif
            }

            if(std::strcmp(szDBEngineName, "sqlite3") == 0){
#if defined(MIR2X_ENABLE_SQLITE3)
                return true;
#else
                return false;
#endif
            }
            throw fflerror("invalid dbengine name: %s", szDBEngineName);
        }

    public:
        const char *DBEngine() const
        {
            return m_connVec[0] ? m_connVec[0]->DBEngine() : nullptr;
        }
};

using DBPod = DBPodConnector<4>;
