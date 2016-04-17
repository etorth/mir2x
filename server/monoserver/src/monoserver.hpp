/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/17/2016 01:11:06
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
#include <atomic>
#include <cstdint>
#include <chrono>
#include <type_traits>

#include "log.hpp"
#include "taskhub.hpp"
#include "monster.hpp"
#include "mapthread.hpp"
#include <unordered_map>
#include "sessionio.hpp"
#include "database.hpp"
#include "message.hpp"
#include "asynchub.hpp"
#include "charobject.hpp"
#include "asyncobject.hpp"
#include "eventtaskhub.hpp"
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
        std::atomic<uint32_t> m_ObjectUID;
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

    private:
        // I didn't put lock on every map
        std::mutex m_MapVLock;
        std::unordered_map<uint32_t, std::shared_ptr<ServerMap>> m_MapV;

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
        // get the monster number of the monster
        // parameters
        //     1. nMapID       :  map id, 0 for all map, 
        //     2. nMonsterIndex:  monster index, 0 for all monster
        //     3. bStrict      :  true for exact number, false for estimation
        int GetMonsterCount(uint32_t nMapID, uint32_t nMonsterIndex, bool bStrict = false)
        {
            if(nMapID == 0 || nMonsterIndex == 0 || bStrict){
                return -1;
            }
            return 0;
        }

    public:
        ServerMap *GetMap(uint32_t nMapID)
        {
            if(nMapID){
                auto stInst = m_MapV.find(nMapID);
                if(stInst != m_MapV.end()){
                    return stInst->second.get();
                }
            }
            return nullptr;
        }

    private:
        bool AddObject();

    public:
        // helper function to run char object
        // TODO this funciton causes dead lock.... but I don't get the reason
        void Operate(uint32_t nUID, uint32_t nAddTime, uint32_t nDelay)
        {
            // TODO any side effect to make it static? YES, don't use it here
            // 
            // 1. get an dynamically allocated handler
            auto *fnOperate = new std::function<void()>();

            // 2. assign to it
            *fnOperate = [nUID, nAddTime, nDelay, fnOperate, this](){
                // TODO
                // this may cause dead lock?
                // lock an object A, then try to lock the map in Operate
                //
                // another object B lock the map, and when exam objects in the
                // map cell, try to lock object A
                //
                // think of this in more details
                auto pGuard = CheckOut<CharObject>(nUID, nAddTime);
                if(pGuard && pGuard->State(STATE_INCARNATED)){
                    pGuard->Operate();
                    extern EventTaskHub *g_EventTaskHub;
                    g_EventTaskHub->Add(nDelay, *fnOperate);
                    return;
                }

                // this object is no longer active
                if(pGuard && pGuard->State(STATE_PHANTOM)){
                    Remove<CharObject>(nUID, nAddTime);
                    delete fnOperate;
                }
            };

            extern EventTaskHub *g_EventTaskHub;
            g_EventTaskHub->Add(nDelay, *fnOperate);
        }

    public:
        template<typename T> void Remove(uint32_t nID, uint32_t nAddTime)
        {
            if(std::is_same<T, CharObject>::value){
                m_CharObjectHub.Remove(((uint64_t)nID << 32) + nAddTime);
            }
        }

    public:
        // all version of check out
        template<typename T, bool bLockIt = true> ObjectLockGuard<T> CheckOut(uint32_t nID, uint32_t nAddTime)
        {
            if(std::is_same<T, CharObject>::value){
                return m_CharObjectHub.CheckOut<bLockIt>(((uint64_t)nID << 32) + nAddTime);
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

    public:
        // all methods to add new monster
        bool AddMonster(uint32_t, uint32_t, int, int, bool, uint32_t *, uint32_t *);

        bool AddMonster(
                uint32_t nMonsterInex, uint32_t nMapID, int nX, int nY, bool bStrict = false)
        {
            return AddMonster(nMonsterInex, nMapID, nX, nY, bStrict, nullptr, nullptr);
        }

        bool AddMonster(
                uint32_t nMonsterInex, uint32_t nMapID, uint32_t *pUID, uint32_t *pAddTime)
        {
            return AddMonster(nMonsterInex, nMapID, -1, -1, false, pUID, pAddTime);
        }

        bool AddMonster(uint32_t nMonsterIndex, uint32_t nMapID)
        {
            return AddMonster(nMonsterIndex, nMapID, -1, -1, false, nullptr, nullptr);
        }
};
