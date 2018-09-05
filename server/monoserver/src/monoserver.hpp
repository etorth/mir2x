/*
 * =====================================================================================
 *
 *       Filename: monoserver.hpp
 *        Created: 02/27/2016 16:45:49
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
#include <queue>
#include <vector>
#include <chrono>
#include <cstdint>
#include <sol/sol.hpp>
#include <type_traits>
#include <unordered_map>

#include "log.hpp"
#include "message.hpp"
#include "taskhub.hpp"
#include "database.hpp"
#include "eventtaskhub.hpp"
#include "commandluamodule.hpp"

class ServiceCore;
class ServerObject;
class MonoServer final
{
    struct UIDLockRecord
    {
        std::mutex Lock;
        std::unordered_map<uint32_t, const ServerObject *> Record;
    };

    private:
        std::mutex m_LogLock;
        std::vector<char> m_LogBuf;

    private:
        std::mutex m_CWLogLock;
        std::vector<char> m_CWLogBuf;

    private:
        std::mutex m_NotifyGUILock;
        std::queue<std::string> m_NotifyGUIQ;

    private:
        ServiceCore *m_ServiceCore;

    private:
        std::atomic<uint32_t> m_GlobalUID;

    private:
        std::array<UIDLockRecord, 17> m_UIDArray;

    private:
        const std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

    public:
        void NotifyGUI(std::string);
        void ParseNotifyGUIQ();

    public:
        void FlushBrowser();
        void FlushCWBrowser();

    public:
        MonoServer();
       ~MonoServer() = default;

    public:
        void ReadHC();

        void Launch();
        void Restart();

    private:
        void RunASIO();
        void CreateDBConnection();
        void LoadMapBinDBN();

    public:
        void AddCWLog(uint32_t,         // command window id
                int,                    // log color in command window
                                        // we don't support fileName/functionName here
                                        // since AddCWLog() is dedicated for lua command ``printLine"
                const char *,           // prompt
                const char *, ...);     // variadic argument list support std::vsnprintf()

        void AddLog(const std::array<std::string, 4> &,     // argument list, compatible to Log::AddLog()
                const char *, ...);                         // variadic argument list supported by std::vsnprintf()

    private:
        bool AddPlayer(uint32_t, uint32_t);

    private:
        void StartNetwork();
        void StartServiceCore();

    public:
        std::vector<int>   GetMapList();
        sol::optional<int> GetMonsterCount(int, int);

    public:
        bool AddMonster(uint32_t,       // monster id
                uint32_t,               // map id
                int,                    // x
                int,                    // y
                bool);                  // do random throw if (x, y) is invalid

    public:
        std::chrono::time_point<std::chrono::steady_clock> GetTimeNow()
        {
            return std::chrono::steady_clock::now();
        }

        uint32_t GetTimeDiff(std::chrono::time_point<std::chrono::steady_clock> stBegin, const char *szUnit)
        {
            if(!std::strcmp(szUnit, "ns")){
                return std::chrono::duration_cast<std::chrono::microseconds>(GetTimeNow() - stBegin).count();
            }

            if(!std::strcmp(szUnit, "ms")){
                return std::chrono::duration_cast<std::chrono::milliseconds>(GetTimeNow() - stBegin).count();
            }

            if(!std::strcmp(szUnit, "s") || !std::strcmp(szUnit, "sec")){
                return std::chrono::duration_cast<std::chrono::seconds>(GetTimeNow() - stBegin).count();
            }
            return 0;
        }

        uint32_t GetTimeTick() const
        {
            return (uint32_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_StartTime).count());
        }


    public:
        bool RegisterLuaExport(CommandLuaModule *, uint32_t);

    public:
        uint32_t SleepEx(uint32_t);
};
