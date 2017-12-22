/*
 * =====================================================================================
 *
 *       Filename: dbpod.hpp
 *        Created: 05/20/2016 14:31:19
 *  Last Modified: 06/05/2016 21:40:13
 *
 *    Description: so many db interaction, so enable the multi-thread support
 *                 when got a DBRecord, the corresponding DBConnection would
 *                 be locked, this is the reason I have to introduce DBHDR, by
 *                 the way I hate the name DBPod<N>::DBHDR
 *
 *                 do I really need a memory pool inside? NO, but since I need
 *                 put RTTI support for the db unlock, which needs unique
 *                 pointer with deleter, so I just make a memory pool inside.
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

#include "dbrecord.hpp"
#include "dbconnection.hpp"
#include "memoryblockpn.hpp"

// ConnectionSize gives the number of internal connection handler
//       0 : invalid
//       1 : single thread
//    >= 2 : support multi-thread
//
template<size_t ConnectionSize = 4>
class DBPod final
{
    private:
        // the lock is on DBConnection and its DBRecordPN both, so here
        // we just use a un-locking PN
        using DBRecordPN = MemoryBlockPN<sizeof(DBRecord), 1024, 1>;
        class InnDeleter
        {
            private:
                std::mutex *m_Lock;
                DBRecordPN *m_DBRPN;

            public:
                InnDeleter(std::mutex *pLock = nullptr, DBRecordPN *pDBRPN = nullptr)
                    : m_Lock(pLock)
                    , m_DBRPN(pDBRPN)
                {}

                // this class won't own any resource
                // so just use default destructor
                ~InnDeleter() = default;

            void operator()(DBRecord *pBuf)
            {
                // 0. screen out null operation
                if(!pBuf){ return; }

                // 1. free the buffer allocated from the corresponding PN
                if(m_DBRPN){ m_DBRPN->Free(pBuf); }

                // 2. unlock the corresponding DBConnection
                //    not a good design since lock() / unlock() are in different scope
                //    be careful when using it
                if(ConnectionSize > 1 && m_Lock){ m_Lock->unlock(); }
            }
        };

    public:
        using DBHDR = std::unique_ptr<DBRecord, InnDeleter>;

    private:
        std::string m_HostName;
        std::string m_UserName;
        std::string m_Password;
        std::string m_DBName;

        unsigned int m_Port;

        size_t m_Count;
        std::mutex *m_LockV[ConnectionSize];
        DBRecordPN m_DBRPNV[ConnectionSize];
        DBConnection *m_DBConnV[ConnectionSize];

    public:
        // I didn't check validation of connection here
        DBPod()
            : m_Count(0)
        {
            static_assert(ConnectionSize > 0, "DBPod should contain at least one connection handler");

            for(size_t nIndex = 0; nIndex < ConnectionSize; ++nIndex){
                m_LockV[nIndex]   = nullptr;
                m_DBConnV[nIndex] = nullptr;
            }
        }

        // launch the db connection
        // return value
        //      0: OK
        //      1: invalid argument
        //      2: failed in connection
        //      3: mysterious errors
        int Launch(const char *szHostName, const char *szUserName,
                const char *szPassword, const char *szDBName, unsigned int nPort)
        {
            // TODO add argument check here
            if(false){ return 1; }

            m_HostName = szHostName;
            m_UserName = szUserName;
            m_Password = szPassword;
            m_DBName   = szDBName;
            m_Port     = nPort;

            for(int nIndex = 0; nIndex < (int)ConnectionSize; ++nIndex){
                auto pConn = new DBConnection(szHostName, szUserName, szPassword, szDBName, nPort);
                if(!pConn->Valid()){ delete pConn; return 2;}

                m_DBConnV[nIndex] = pConn;
                if(ConnectionSize > 1){ m_LockV[nIndex] = new std::mutex(); }
            }

            return 0;
        }

        // make sure it's non-throw
        DBHDR InnCreateDBHDR(size_t nPodIndex)
        {
            DBRecord *pRecord;
            try{
                auto pBuf = m_DBRPNV[nPodIndex].Get();
                if(!pBuf){ return DBHDR(nullptr, InnDeleter()); }

                pRecord = m_DBConnV[nPodIndex]->CreateDBRecord((DBRecord *)pBuf);
            }catch(...){
                pRecord = nullptr;
            }

            return DBHDR(pRecord, InnDeleter(m_LockV[nPodIndex], &(m_DBRPNV[nPodIndex])));
        }

        DBHDR CreateDBHDR()
        {
            // for single thread only, we don't need the lock protection
            if(ConnectionSize == 1){ return InnCreateDBHDR(0); }

            // ok we need to handle multi-thread
            size_t nIndex = (m_Count++) % ConnectionSize;
            while(true){
                if(m_LockV[nIndex]->try_lock()){
                    // see here, we don't unlock when return
                    // it unlocks when HDR is destructing
                    return InnCreateDBHDR(nIndex);
                }

                nIndex = (nIndex + 1) % ConnectionSize;
            }

            // never be here
            return DBHDR(nullptr, InnDeleter());
        }
};

using DBPodN = DBPod<4>;
