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
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <sol/sol.hpp>
#include <unordered_map>

#include "log.hpp"
#include "message.hpp"
#include "taskhub.hpp"
#include "raiitimer.hpp"
#include "eventtaskhub.hpp"
#include "commandluamodule.hpp"

class ServiceCore;
class ServerObject;
class MonoServer final
{
    private:
        std::mutex m_logLock;
        std::vector<char> m_logBuf;

    private:
        std::mutex m_CWLogLock;
        std::vector<char> m_CWLogBuf;

    private:
        std::mutex m_notifyGUILock;
        std::queue<std::string> m_notifyGUIQ;

    private:
        ServiceCore *m_serviceCore;

    private:
        std::exception_ptr m_currException;

    private:
        hres_timer m_hrtimer;

    public:
        void notifyGUI(std::string);
        void parseNotifyGUIQ();

    public:
        void FlushBrowser();
        void FlushCWBrowser();

    public:
        MonoServer();
       ~MonoServer() = default;

    public:
        void ReadHC();

        void Launch();
        void restart(const std::string & = {});

    private:
        void RunASIO();
        void CreateDBConnection();
        void LoadMapBinDB();

    private:
        bool hasDatabase() const;
        bool hasCharacter(const char *) const;

    private:
        void createDefaultDatabase();

    public:
        int  createAccount(const char *, const char *);
        bool createAccountCharacter(const char *, const char *, bool, const char *);

    public:
        void checkException();
        void propagateException() noexcept;
        void logException(const std::exception &, std::string * = nullptr) noexcept;

    public:
        void addCWLog(uint32_t,         // command window id
                int,                    // log color in command window
                                        // we don't support fileName/functionName here
                                        // since addCWLog() is dedicated for lua command ``printLine"
                const char *,           // prompt
                const char *, ...);     // variadic argument list support std::vsnprintf()

        void addLog(const std::array<std::string, 4> &,     // argument list, compatible to Log::addLog()
                const char *, ...);                         // variadic argument list supported by std::vsnprintf()

    private:
        void StartNetwork();
        void StartServiceCore();

    public:
        std::vector<int>   GetMapList();
        sol::optional<int> GetMonsterCount(int, int);

    public:
        bool addMonster(uint32_t,       // monster id
                uint32_t,               // map id
                int,                    // x
                int,                    // y
                bool);                  // use strict loc

    public:
        bool addNPChar(uint16_t, uint32_t, int, int, bool);

    public:
        uint32_t getCurrTick() const
        {
            return (uint32_t)(m_hrtimer.diff_msec());
        }

    public:
        void regLuaExport(CommandLuaModule *, uint32_t);

    public:
        uint64_t sleepExt(uint64_t);
};
