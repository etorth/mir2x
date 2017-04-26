/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
 *  Last Modified: 04/21/2017 00:22:39
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
#include <chrono>
#include <atomic>
#include <cstdint>
#include <type_traits>
#include <unordered_map>

#include "log.hpp"
#include "message.hpp"
#include "taskhub.hpp"
#include "database.hpp"
#include "servermap.hpp"
#include "eventtaskhub.hpp"
#include "monsterginforecord.hpp"

class ServiceCore;
class MonoServer final
{
    private:
        std::mutex m_LogLock;
        std::vector<char> m_LogBuf;

    private:
        ServiceCore *m_ServiceCore;

    private:
        std::atomic<uint32_t> m_GlobalUID;
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;

    private:
        std::unordered_map<uint32_t, MonsterGInfoRecord> m_MonsterGInfoRecord;

    public:
        void FlushBrowser();

    public:
        const MonsterGInfoRecord &MonsterGInfo(uint32_t nMonsterID) const
        {
            auto pGInfoFind = m_MonsterGInfoRecord.find(nMonsterID);
            if(pGInfoFind == m_MonsterGInfoRecord.end()){
                return MonsterGInfoRecord::Null();
            }

            return pGInfoFind->second;
        }

    public:
        MonoServer();
       ~MonoServer() = default;

    public:
        void ReadHC();

        void Launch();
        void Restart();

    private:
        void RunASIO();
        void CreateServiceCore();
        void CreateDBConnection();

    public:
        void AddLog(const std::array<std::string, 4> &, const char *, ...);

    private:
        bool AddPlayer(uint32_t, uint32_t);

    private:
        bool InitMonsterRace();
        bool InitMonsterItem();
        bool LoadMonsterRecord()
        {
           return InitMonsterRace() && InitMonsterItem();
        }

    private:
        void StartNetwork();

    public:
        // for gui callbacks
        std::vector<uint32_t> GetActiveMapList();
        std::vector<uint32_t> GetValidMonsterList(uint32_t);
        int  GetValidMonsterCount(int, int);


    public:
        void AddMonster(uint32_t, uint32_t, int, int);

    public:
        uint32_t GetUID()
        {
            return m_GlobalUID.fetch_add(1);
        }

        uint32_t GetTimeTick()
        {
            return (uint32_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_StartTime).count());
        }
};
