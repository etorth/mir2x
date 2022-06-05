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
#include "totype.hpp"
#include "raiitimer.hpp"
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
        ServiceCore *m_serviceCore = nullptr;

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
        MonoServer() = default;
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
        void addCWLogString(uint32_t, int, const char *, const char *);

    public:
        void addLog(const Log::LogTypeLoc &, const char *, ...);

    private:
        void StartNetwork();
        void StartServiceCore();

    public:
        bool loadMap(const std::string &);
        std::vector<int> getMapList();
        sol::optional<int> getMonsterCount(int, int);

    public:
        bool addMonster(uint32_t,       // monster id
                uint32_t,               // map id
                int,                    // x
                int,                    // y
                bool);                  // use strict loc

    public:
        bool addNPChar(const char *,    // NPC name
                uint16_t,               // look id
                uint32_t,               // map id
                int,                    // map x
                int,                    // map y
                int,                    // NPC gfx dir, may not be 8-dir
                const char *);          // NPC script full name

    public:
        uint32_t getCurrTick() const
        {
            return to_u32(m_hrtimer.diff_msec());
        }

    public:
        void regLuaExport(CommandLuaModule *, uint32_t);

    public:
        uint64_t sleepExt(uint64_t);
};
