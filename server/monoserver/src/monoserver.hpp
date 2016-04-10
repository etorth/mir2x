/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/09/2016 22:30:45
 *
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
#include <type_traits>
#include <cstdint>

#include "log.hpp"
#include "mapthread.hpp"
#include <unordered_map>
#include "sessionio.hpp"
#include "database.hpp"
#include "message.hpp"
#include "asynchub.hpp"
#include "asyncobject.hpp"
#include "objectlockguard.hpp"


class MonoServer final
{
    public:
        MonoServer();
        ~MonoServer();

    public:
        void Launch();
        void ReadHC();

    private:
        void RunASIO();
        void CreateDBConnection();

    private:
        void AddLog(int, const char *, ...);

    private:
        void ExtendLogBuf(size_t);

    private:
        // for network
        SessionIO   *m_SessionIO;

    private:
        // for log
        size_t   m_LogBufSize;
        char    *m_LogBuf;

    private:
        // for DB
        DBConnection *m_DBConnection;

    private:
        bool AddPlayer(int, uint32_t);

        AsyncHub<CharObject> m_CharObjectHub;

    public:
        // for gui
        bool GetValidMapV(std::vector<std::pair<int, std::string>> &);
        bool GetValidMonsterV(int, std::vector<std::pair<int, std::string>> &);
        int  GetValidMonsterCount(int, int);

    private:
        void OnReadHC(uint8_t, Session *);

        void OnPing (Session *);
        void OnLogin(Session *);

    public:
        // Locked object check out
        template<typename T, bool bLockIt = true> ObjectLockGuard<T> CheckOut(
                uint32_t nID, uint32_t nAddTime)
        {
            if(nID == 0 || nAddTime == 0){
                return {nullptr, false};
            }

            if(std::is_same<T, CharObject>::value){
                return CheckHub<CharObject, bLockIt>(&m_CharObjectHub, &m_CharObjectHubLock);
            }

            if(std::is_same<T, Session>::value){
                return CheckHub<Session, bLockIt>(&m_SessionHub, &m_SessionHubLock);
            }

            return {nullptr, false};
        }

        std::unordered_map<uint64_t, std::tuple<CharObject *, std::mutex *, 


    private:
        template<typename T, bool bLockIt = true> ObjectLockGuard<T> Lock(
                std::unordered_map<T> *pHub, std::mutex *pHubLock)
        {
            if(pHub && pHubLock){

                std::lock_guard<std::mutex> stLockGuard<m_CharObjectHubLock>;
                auto p = m_CharObjectHub.find(nID);

                if(p == m_CharObjectHub.end()){
                    return {nullptr, false};
                }else{
                    if(nAddTime != p.second.second){
                        return {nullptr, false};
                    }
                }

                // OK now we get it
                if(bLockIt){
                    p.second.first->Lock();
                }

                return {p.second.first, bLockIt};
            }
        }

    public:
        // copy from class Log to support LOGTYPE_XXX
        template<typename... U> void AddLog(const std::array<std::string, 4> &stLoc, U&&... u)
        {
            extern Log *g_Log;
            g_Log->AddLog(stLoc, std::forward<U>(u)...);

            int nLevel = std::atoi(stLoc[0].c_str());
            AddLog(nLevel, std::forward<U>(u)...);
        }
};
