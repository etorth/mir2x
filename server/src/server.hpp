#pragma once
#include <ctime>
#include <mutex>
#include <queue>
#include <atomic>
#include <vector>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <sol/sol.hpp>
#include <unordered_map>

#include "log.hpp"
#include "message.hpp"
#include "totype.hpp"
#include "raiitimer.hpp"
#include "commandluamodule.hpp"

class ServiceCore;
class ServerObject;
class Server final
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
        ServiceCore *m_serviceCore = nullptr;

    private:
        std::atomic_flag m_hasExcept;
        std::exception_ptr m_currException;

    private:
        hres_timer m_hrtimer;

        // wall clock when the server came up, which is all the in-game clock needs as an
        // origin. elapsed time comes off m_hrtimer instead so that adjusting the system
        // clock can not move the game's day and night around
        const std::time_t m_launchTime = std::time(nullptr);

    public:
        void notifyGUI(std::string);
        void parseNotifyGUIQ();

    public:
        void FlushBrowser();
        void FlushCWBrowser();

    public:
        Server() = default;
       ~Server() = default;

    public:
       void mainLoop();

    public:
        void launch();
        void restart(const std::string & = {});

    private:
        void RunASIO();
        void createDBConnection();

    public:
        void loadMapBinDB();

    private:
        bool hasDatabase() const;
        bool hasCharacter(const char *) const;

    private:
        void createDefaultDatabase();

    public:
        int  createAccount(const char *, const char *);
        bool createAccountCharacter(const char *, const char *, bool, int);

    public:
        void checkException();
        void propagateException() noexcept;
        void logException(const std::exception &, std::string * = nullptr) noexcept;

    public:
        void addCWLogString(uint32_t, int, const char *, const char *);

    public:
        [[noreturn]] void addFatal(const char *, ...);

    public:
        void addLog(const Log::LogTypeLoc &, const char *, ...);

    public:
        bool loadBaseMap(uint32_t);
        std::vector<int> getMapList();
        sol::optional<size_t> getMonsterCount(uint32_t, uint64_t);

    public:
        bool addMonster(uint32_t,       // monster id
                uint32_t,               // map id
                int,                    // x
                int,                    // y
                bool);                  // use strict loc

    public:
        uint32_t getCurrTick() const
        {
            return to_u32(m_hrtimer.diff_msec());
        }

    public:
        std::time_t launchTime() const
        {
            return m_launchTime;
        }

        // seconds the server has been up, monotonic. getCurrTick() is a uint32 of msec and
        // wraps after seven weeks, this does not
        uint64_t uptime() const
        {
            return m_hrtimer.diff_sec();
        }

    public:
        void regLuaExport(CommandLuaModule *, uint32_t);

    public:
        uint64_t sleepExt(uint64_t);
};
