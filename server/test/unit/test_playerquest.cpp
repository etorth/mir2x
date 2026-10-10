#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sol/sol.hpp>
#include "argf.hpp"
#include "corof.hpp"
#include "log.hpp"
#include "server.hpp"
#include "serverargparser.hpp"
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"
#include "totype.hpp"
#include "uidf.hpp"

class PeerConfig;
class ActorPool;
class DBPod;
class MapBinDB;
class ScriptWindow;
class ProfilerWindow;
class MainWindow;
class ServerPasswordWindow;
class ServerConfigureWindow;
class PodMonitorWindow;
class ActorMonitorWindow;

ServerArgParser       *g_serverArgParser = nullptr;
PeerConfig            *g_peerConfig = nullptr;
Log                   *g_mir2xLog = nullptr;
ActorPool             *g_actorPool = nullptr;
DBPod                 *g_dbPod = nullptr;
MapBinDB              *g_mapBinDB = nullptr;
ScriptWindow          *g_scriptWindow = nullptr;
ProfilerWindow        *g_profilerWindow = nullptr;
MainWindow            *g_mainWindow = nullptr;
Server                *g_server = nullptr;
ServerPasswordWindow  *g_serverPasswordWindow = nullptr;
ServerConfigureWindow *g_serverConfigureWindow = nullptr;
PodMonitorWindow      *g_podMonitorWindow = nullptr;
ActorMonitorWindow    *g_actorMonitorWindow = nullptr;

namespace
{
    void require(bool condition, const char *message)
    {
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    class CoutCapture final
    {
        private:
            std::ostringstream m_buf;
            std::streambuf * const m_saved;

        public:
            CoutCapture()
                : m_saved(std::cout.rdbuf(m_buf.rdbuf()))
            {}

            ~CoutCapture()
            {
                std::cout.rdbuf(m_saved);
            }

        public:
            bool has(const std::string &s) const
            {
                return m_buf.str().find(s) != std::string::npos;
            }
    };

    class TestPlayerObject final: public ServerObject
    {
        public:
            TestPlayerObject()
                : ServerObject(uidf::getPlayerUID(1))
            {}

        public:
            corof::awaitable<> onActorMsg(const ActorMsgPack &) override
            {
                return {};
            }
    };

    constexpr static unsigned char charObjectScript []
    {
        #embed "../../src/charobject.lua" suffix(,)
        '\0'
    };

    constexpr static unsigned char playerScript []
    {
        #embed "../../src/player.lua" suffix(,)
        '\0'
    };

    // stands for the C++ bindings of the player and the quests that _RSVD_NAME_setupQuests() and the quest triggers use
    // quest A raises in its restore and in its trigger, as a quest with a saved NPC behavior that no longer loads would
    // the C++ side puts "local questA, questB = ..." in front of it
    constexpr const char *playerBindings = R"###(
        TEST = {restored = {}, triggered = {}, calls = {}}

        _G['_RSVD_NAME_queryQuestUIDList' .. SYS_COOP] = function(onDone)
            onDone({questA, questB})
        end

        _G['_RSVD_NAME_queryQuestTriggerList' .. SYS_COOP] = function(triggerType, onDone)
            onDone({questA, questB})
        end

        -- TEST.loadFirst: every restore loads the quest context before it restores the states
        _G['_RSVD_NAME_remoteCall' .. SYS_COOP] = function(uid, code, args, onDone)
            local restore = string.find(code, '_RSVD_NAME_restoreQuestState', 1, true)
            local trigger = string.find(code, '_RSVD_NAME_trigger', 1, true) ~= nil

            if restore then
                local load = string.find(code, '_RSVD_NAME_loadQuestContext(playerUID)', 1, true)
                TEST.loadFirst = (load ~= nil) and (load < restore) and (TEST.loadFirst ~= false)
            end

            -- the restores and the triggers in the order they're called, a trigger with its type
            if restore then
                table.insert(TEST.calls, 'restore')
            elseif trigger then
                table.insert(TEST.calls, 'trigger ' .. args[1])
            end

            if restore or trigger then
                table.insert(restore and TEST.restored or TEST.triggered, uid)
                if uid == questA then
                    onDone(SYS_EXECERROR, 'quest A raised')
                else
                    onDone(SYS_EXECDONE)
                end
            else
                if TEST.completed then
                    onDone(SYS_EXECDONE, 'quest_' .. uid, SYS_DONE, TEST.completedDesp)
                else
                    onDone(SYS_EXECDONE, 'quest_' .. uid, 'running', {[SYS_QSTFSM] = 'desp'})
                end
            end
        end

        function _RSVD_NAME_reportQuestDespList(questDespList)
            TEST.despList = questDespList
        end

        function TEST.calledBoth(list)
            return list[1] == questA and list[2] == questB and list[3] == nil
        end

        function TEST.listedBoth()
            return TEST.despList ~= nil and TEST.despList['quest_' .. questA] ~= nil and TEST.despList['quest_' .. questB] ~= nil
        end
    )###";

    struct PlayerFixture
    {
        TestPlayerObject so;

        // the pod is never attached to an actor pool, and ~ActorPod() detaches from g_actorPool, which this test doesn't have
        // so the pod is left alive on purpose
        ServerLuaCoroutineRunner runner{new ActorPod(&so)};

        uint64_t driverKey = 300;

        PlayerFixture()
        {
            require(runner.execRawString(to_rawcstr(charObjectScript)).valid(), "failed to load charobject.lua");
            require(runner.execRawString(to_rawcstr(playerScript)).valid(), "failed to load player.lua");

            const auto bindings = "local questA, questB = " + std::to_string(uidf::getQuestUID(2)) + ", " + std::to_string(uidf::getQuestUID(3)) + "\n" + playerBindings;
            require(runner.execRawString(bindings.c_str()).valid(), "failed to setup player bindings");
        }

        void drive(const std::string &code)
        {
            bool done = false;
            bool valid = false;
            const auto key = driverKey++;

            runner.spawn(key, code, {}, [&done, &valid](const sol::protected_function_result &pfr)
            {
                done = true;
                valid = pfr.valid();
            });

            require(done && !runner.hasKey(key), "driver thread doesn't finish");
            require(valid, "driver thread raised");
        }

        bool check(const std::string &expr)
        {
            require(runner.execRawString(("TEST.result = " + expr).c_str()).valid(), "failed to check lua expression");
            const sol::object obj = runner.getState()["TEST"]["result"];
            return obj.is<bool>() && obj.as<bool>();
        }
    };

    void testRestoreGoesOn()
    {
        PlayerFixture f;
        CoutCapture capture;
        f.drive("_RSVD_NAME_setupQuests()");

        require(f.check("TEST.calledBoth(TEST.restored)"), "restore stops at a quest that raised");
        require(f.check("TEST.loadFirst"), "the login doesn't load the quest context before it restores the states");
        require(f.check("TEST.listedBoth()"), "quest list misses a quest, or isn't reported, after a quest raised in its restore");

        const auto online = std::string("'trigger ' .. SYS_ON_ONLINE");
        require(f.check("(#TEST.calls == 4) and (TEST.calls[1] == 'restore') and (TEST.calls[2] == 'restore') and (TEST.calls[3] == " + online + ") and (TEST.calls[4] == " + online + ")"), "the login doesn't fire SYS_ON_ONLINE to every quest after their restores");
        require(capture.has("Quest QST_2 failed to restore player PLY_1") && capture.has("Remote call to QST_2 failed: quest A raised"), "restore error of a quest is not logged");
    }

    void testTriggerGoesOn()
    {
        PlayerFixture f;
        CoutCapture capture;
        f.drive("_RSVD_NAME_trigger(SYS_ON_KILL, 123)");

        require(f.check("TEST.calledBoth(TEST.triggered)"), "trigger stops at a quest that raised");
        require(capture.has("Quest QST_2 failed to run trigger ") && capture.has("Remote call to QST_2 failed: quest A raised"), "trigger error of a quest is not logged");
    }

    void testCompletedDescriptions()
    {
        PlayerFixture f;
        CoutCapture capture;
        const auto questName = "'quest_" + std::to_string(uidf::getQuestUID(3)) + "'";
        f.drive("TEST.completed = true; TEST.completedDesp = {[SYS_QSTFSM] = '完成了原来的委托。'}; _RSVD_NAME_setupQuests()");
        require(f.check("TEST.despList[" + questName + "][SYS_QSTFSM] == '完成了原来的委托。'"), "login replaces the completed quest description");

        f.drive("TEST.completedDesp = nil; _RSVD_NAME_setupQuests()");
        require(f.check("TEST.despList[" + questName + "][SYS_QSTFSM] == '任务已完成'"), "completed quest without a description loses its default");
    }

    void runTests()
    {
        testRestoreGoesOn();
        testTriggerGoesOn();
        testCompletedDescriptions();
    }
}

int main()
{
    try{
        char arg0[] = "playerquest_test";
        char arg1[] = "--slave";
        char arg2[] = "--master-ip=127.0.0.1";
        char *argv[] = {arg0, arg1, arg2, nullptr};

        const argf::parser parser(3, argv);
        ServerArgParser serverArgs(parser);
        serverArgs.setSharedConfig(ServerArgParser::MasterSharedConfig
        {
            .logicalFPS = 1,
            .summonCount = 1,
        });
        g_serverArgParser = &serverArgs;

        // Log's constructor prints a banner with the pid and the log file path, which a gold file can't match
        std::ostringstream logBannerDiscard;
        auto * const savedCoutBuf = std::cout.rdbuf(logBannerDiscard.rdbuf());
        Log log("mir2x-playerquest-test");
        std::cout.rdbuf(savedCoutBuf);
        g_mir2xLog = &log;

        Server server;
        g_server = &server;

        runTests();
        std::printf("Player quest passed: restore loads the quest context first, goes on after a quest raised, and fires SYS_ON_ONLINE after it, and triggers go on after a quest raised.\n");

        g_server = nullptr;
        g_mir2xLog = nullptr;
        g_serverArgParser = nullptr;
        return 0;
    }
    catch(const std::exception &e){
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
