/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/11/2016 02:09:10
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
#include <cstdint>
#include <chrono>
#include <type_traits>

#include "log.hpp"
#include "monster.hpp"
#include "mapthread.hpp"
#include <unordered_map>
#include "sessionio.hpp"
#include "database.hpp"
#include "message.hpp"
#include "asynchub.hpp"
#include "charobject.hpp"
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
        std::vector<MONSTERRACEINFO> m_MonsterRaceInfoV;

    private:
        bool InitMonsterRace();
        bool InitMonsterItem();

    public:
        // for gui
        bool GetValidMapV(std::vector<std::pair<int, std::string>> &);
        bool GetValidMonsterV(int, std::vector<std::pair<int, std::string>> &);
        int  GetValidMonsterCount(int, int);

    public:
        uint32_t GetTickCount();

    protected:
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;

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
                return {m_CharObjectHub.Retrieve(
                        ((uint64_t)nID << 32) + nAddTime, bLockIt), bLockIt};
            }

            return {nullptr, false};
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
