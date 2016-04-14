/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/13/2016 19:24:35
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
        int GetMonsterCount(uint32_t, uint32_t);

    private:
        bool AddObject();

    public:
        // all version of check out
        template<bool bLockIt = true> ObjectLockGuard<CharObject> CheckOut<CharObject, bLockIt>(
                uint32_t nID, uint32_t nAddTime)
        {
            return m_CharObjectHub.CheckOut<bLockIt>(((uint64_t)nID << 32) + nAddTime);
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

    public:
        // all methods to add new monster
        bool AddMonster(uint32_t, uint32_t, int, int, bool, uint32_t *, uint32_t *);

        bool AddMonster(
                uint32_t nMonsterInex, uint32_t nMapID, int nX, int nY, bool bStrict = false)
        {
            return AddMonster(nMonsterInex, nMapID, nX, nY, bStrict, nullptr, nullptr);
        }

        bool MonoServer::AddMonster(
                uint32_t nMonsterInex, uint32_t nMapID, uint32_t *pUID, uint32_t *pAddTime)
        {
            return AddMonster(nMonsterInex, nMapID, -1, -1, false, pUID, pAddTime);
        }

        bool MonoServer::AddMonster(uint32_t nMonsterIndex, uint32_t nMapID)
        {
            return AddMonster(nMonsterInex, nMapID, -1, -1, false, nullptr, nullptr);
        }

};
