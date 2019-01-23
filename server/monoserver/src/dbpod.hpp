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
#include <memory>
#include <string>

#include "strfunc.hpp"
#include "database.hpp"

template<size_t ConnectionCount = 4> class DBPod final
{
    private:
        class InnDeleter
        {
            private:
                DBConnection *m_DBConnection;

            private:
                std::mutex *m_Lock;

            public:
                InnDeleter(DBConnection *pDBConnection = nullptr, std::mutex *pLock = nullptr)
                    : m_DBConnection(pDBConnection)
                    , m_Lock(pLock)
                {}

                ~InnDeleter() = default;

            public:
                void operator()(DBRecord *pRecord)
                {
                    if(!pRecord){
                        return;
                    }

                    if(pDBConnection){
                        pDBConnection->DestroyDBRecord(pRecord);
                    }

                    if(ConnectionCount > 1 && m_Lock){
                        m_Lock->unlock();
                    }
                    m_Lock = nullptr;
                }
        };

    public:
        using DBHDR = std::unique_ptr<DBRecord, InnDeleter>;

    private:
        std::atomic<size_t> m_Current;

    private:
        std::mutex                m_ConnLockVec[ConnectionCount];
        std::unique_ptr<DBEngine> m_ConnVec    [ConnectionCount];

    public:
        DBPod()
            : m_Current {0}
        {
            static_assert(ConnectionCount > 0, "DBPod should contain at least one connection handler");
        }

        void LaunchMySQL(const char *szHostName, const char *szUserName, const char *szPassword, const char *szDBName, unsigned int nPort)
        {
#if defined(MIR2X_ENABLE_MYSQL)
            for(size_t nIndex = 0; nIndex < ConnectionCount; ++nIndex){
                m_ConnVec_MySQL[nIndex] = std::make_unique<DBEngine_MySQL>(szHostName, szUserName, szPassword, szDBName, nPort);
            }
#else
            throw std::runtime_error(str_fflprintf(": LaunchMySQL() not supported in current build"));
#endif
        }

        void LaunchSQLite3(const char *szDBName)
        {
#if defined(MIR2X_ENABLE_SQLITE3)
            for(size_t nIndex = 0; nIndex < ConnectionCount; ++nIndex){
                m_ConnVec_SQLite3[nIndex] = std::make_unique<DBEngine_SQLite3>(szDBName);
            }
#else
            throw std::runtime_error(str_fflprintf(": LaunchSQLite3() not supported in current build"));
#endif
        }

        DBHDR InnCreateDBHDR(size_t nPodIndex)
        {
            if(auto p = m_ConnVec[nPodIndex]->CreateDBRecord()){
                return DBHDR(p, InnDeleter(m_ConnVec[nPodIndex].get(), m_ConnLockVec + nPodIndex));
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

            for(size_t nIndex = m_Current.fetch_add(1) % ConnectionCount;; nIndex = (nIndex + 1) % ConnectionCount){
                if(m_LockV[nIndex]->try_lock()){
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
            throw std::invalid_argument(str_fflprintf(": Invalid dbengine name: %s", szDBName));
        }
};

using DBPodN = DBPod<4>;
