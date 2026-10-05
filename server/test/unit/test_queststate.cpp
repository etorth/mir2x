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

    class TestServerObject final: public ServerObject
    {
        public:
            TestServerObject()
                : ServerObject(uidf::getQuestUID(1))
            {}

        public:
            corof::awaitable<> onActorMsg(const ActorMsgPack &) override
            {
                return {};
            }
    };

    constexpr static unsigned char questScript []
    {
        #embed "../../src/quest.lua" suffix(,)
        '\0'
    };

    // stands for the C++ bindings of Quest::LuaThreadRunner that setQuestState() uses, the quest database is the lua table TEST.db
    constexpr const char *questBindings = R"###(
        TEST = {db = {}}

        local lastKey = 10000
        function rollKey()
            lastKey = lastKey + 1
            return lastKey
        end

        function getMainScriptThreadKey()
            return 200
        end

        function getQuestName()
            return 'queststate_test'
        end

        function dbGetQuestField(uid, field)
            return (TEST.db[uid] or {})[field]
        end

        function dbSetQuestField(uid, field, value)
            TEST.db[uid] = TEST.db[uid] or {}
            TEST.db[uid][field] = value
        end

        function _RSVD_NAME_setQuestDesp(uid, despTable, fsm, desp)
            dbSetQuestField(uid, 'fld_desp', despTable)
        end

        function _RSVD_NAME_dbSetQuestStateDone(uid)
            TEST.db[uid] = {fld_states = {[SYS_QSTFSM] = {SYS_DONE}}}
        end
    )###";

    // every state records the key of its state runner as TEST.key_<uid>_<state>, and sets TEST.closed_<uid>_<state> when its state runner is closed
    // a state that calls setQuestState() sets TEST.after_<uid>_<state> if setQuestState() returns
    constexpr const char *questFSM = R"###(
        local function enter(uid, state)
            TEST['key_' .. uid .. '_' .. state] = getThreadKey()
            return setmetatable({}, {__close = function()
                TEST['closed_' .. uid .. '_' .. state] = true
            end})
        end

        local function after(uid, state)
            TEST['after_' .. uid .. '_' .. state] = true
        end

        setQuestFSMTable(
        {
            [SYS_ENTER] = function(uid, args)
                pause(SYS_POSINF)
            end,

            a = function(uid, args)
                local guard <close> = enter(uid, 'a')
                setQuestState{uid=uid, state='b'}
                after(uid, 'a')
            end,

            b = function(uid, args)
                local guard <close> = enter(uid, 'b')
                pause(SYS_POSINF)
            end,

            c = function(uid, args)
                local guard <close> = enter(uid, 'c')
                pause(SYS_POSINF)
            end,

            d = function(uid, args)
                local guard <close> = enter(uid, 'd')
                setQuestState{uid=uid, state='e'}
                after(uid, 'd')
            end,

            e = function(uid, args)
                local guard <close> = enter(uid, 'e')
                setQuestState{uid=uid, state='f'}
                after(uid, 'e')
            end,

            f = function(uid, args)
                local guard <close> = enter(uid, 'f')
                pause(SYS_POSINF)
            end,

            g = function(uid, args)
                local guard <close> = enter(uid, 'g')
            end,

            h = function(uid, args)
                local guard <close> = enter(uid, 'h')
                runQuestThread(function()
                    setQuestState{uid=uid, state='b'}
                end)
                after(uid, 'h')
            end,

            i = function(uid, args)
                local guard <close> = enter(uid, 'i')
                local switcher <close> = setmetatable({}, {__close = function()
                    setQuestState{uid=uid, state='failed'}
                end})

                if args == 'pause' then
                    pause(SYS_POSINF)
                end
                setQuestState{uid=uid, state='b'}
            end,

            j = function(uid, args)
                local guard <close> = enter(uid, 'j')

                local okWrap, errWrap = pcall(coroutine.wrap(function()
                    setQuestState{uid=uid, state='b'}
                end))

                local okSort, errSort = pcall(table.sort, {2, 1}, function(x, y)
                    setQuestState{uid=uid, state='b'}
                    return x < y
                end)

                TEST['wrapRaised_' .. uid] = (not okWrap) and string.find(errWrap, 'switching its own state by setQuestState() from a coroutine created in it', 1, true) ~= nil
                TEST['sortRaised_' .. uid] = (not okSort) and string.find(errSort, "switching its own state by setQuestState() where it can't yield", 1, true) ~= nil

                after(uid, 'j')
                pause(SYS_POSINF)
            end,

            k = function(uid, args)
                local guard <close> = enter(uid, 'k')
                local switcher <close> = setmetatable({}, {__close = function()
                    setQuestState{uid=uid, state='failed'}
                end})
                error('state k failed')
            end,

            failed = function(uid, args)
                local guard <close> = enter(uid, 'failed')
                pause(SYS_POSINF)
            end,

            other = function(uid, args)
                local guard <close> = enter(uid, 'other')
                setQuestState{uid=args, state='b'}
                after(uid, 'other')
                pause(SYS_POSINF)
            end,
        })

        setQuestFSMTable('sub',
        {
            [SYS_ENTER] = function(uid, args)
                pause(SYS_POSINF)
            end,

            s1 = function(uid, args)
                local guard <close> = enter(uid, 's1')
                pause(SYS_POSINF)
            end,

            s2 = function(uid, args)
                local guard <close> = enter(uid, 's2')
                setQuestState{uid=uid, state=SYS_DONE}
                after(uid, 's2')
            end,
        })
    )###";

    struct QuestFixture
    {
        TestServerObject so;

        // the pod is never attached to an actor pool, and ~ActorPod() detaches from g_actorPool, which this test doesn't have
        // so the pod is left alive on purpose
        ServerLuaCoroutineRunner runner{new ActorPod(&so)};

        // driver threads stand for remote calls from players or NPCs, each one gets a fresh key
        uint64_t driverKey = 300;

        QuestFixture()
        {
            require(runner.execRawString(questBindings).valid(), "failed to setup quest bindings");
            require(runner.execRawString(to_rawcstr(questScript)).valid(), "failed to load quest.lua");

            // setQuestFSMTable() only works in the main script thread
            runner.spawn(200, std::string(questFSM));
            require(!runner.hasKey(200), "main script thread doesn't finish");
            require(runner.execRawString("TEST.ready = hasQuestState(SYS_QSTFSM, 'a') and hasQuestState('sub', 's1')").valid(), "failed to check quest FSM table");
            require(isTrue("ready"), "quest FSM table is not setup");
        }

        void drive(const std::string &code)
        {
            const auto key = driverKey++;
            runner.spawn(key, code);
            require(!runner.hasKey(key), "driver thread doesn't finish");
        }

        sol::object get(const std::string &name)
        {
            return runner.getState()["TEST"][name];
        }

        bool isNil(const std::string &name)
        {
            return get(name).get_type() == sol::type::lua_nil;
        }

        bool isTrue(const std::string &name)
        {
            const auto obj = get(name);
            return obj.is<bool>() && obj.as<bool>();
        }

        std::string str(const std::string &name)
        {
            const auto obj = get(name);
            require(obj.get_type() == sol::type::string, "not a string");
            return obj.as<std::string>();
        }

        bool strHas(const std::string &name, const std::string &s)
        {
            return str(name).find(s) != std::string::npos;
        }

        uint64_t key(const std::string &name)
        {
            const auto obj = get(name);
            require(obj.is<lua_Integer>(), "state never entered");
            return obj.as<uint64_t>();
        }

        bool alive(const std::string &name)
        {
            return runner.hasKey(key(name));
        }

        // fsm and state are lua expressions, i.e. "SYS_QSTFSM", "'b'", "SYS_DONE"
        bool inState(uint64_t uid, const char *fsm, const char *state)
        {
            const auto code = std::string("TEST.inState = dbGetQuestState(") + std::to_string(uid) + ", " + fsm + ") == " + state;
            require(runner.execRawString(code.c_str()).valid(), "failed to get quest state");
            return isTrue("inState");
        }
    };

    void testRunnerGoesToNextState()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=1, state='a'}");

        require(f.key("key_1_a") != f.key("key_1_b"), "next state runs on the same thread");
        require(!f.alive("key_1_a") && f.isTrue("closed_1_a"), "state runner going to next state is not closed");
        require(f.isNil("after_1_a"), "state runner continued after going to next state");
        require(f.alive("key_1_b") && f.isNil("closed_1_b"), "state runner of next state is not running");
        require(f.inState(1, "SYS_QSTFSM", "'b'"), "quest state is not saved");
    }

    void testOtherThreadChangesState()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=2, state='b'}");
        const auto oldKey = f.key("key_2_b");

        f.drive("setQuestState{uid=2, state='c'}");
        require(!f.runner.hasKey(oldKey) && f.isTrue("closed_2_b"), "suspended state runner of old state is not closed");
        require(f.alive("key_2_c"), "state runner of new state is not running");
    }

    void testRunnerSetsOtherUID()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=3, state='other', args=4}");

        require(f.isTrue("after_3_other") && f.alive("key_3_other") && f.isNil("closed_3_other"), "state runner is closed by setting state of another uid");
        require(f.alive("key_4_b") && f.inState(4, "SYS_QSTFSM", "'b'"), "state of another uid is not set");

        f.drive("setQuestState{uid=3, state='c'}");
        require(!f.alive("key_3_other") && f.isTrue("closed_3_other"), "state runner is unregistered by setting state of another uid");
        require(f.alive("key_4_b"), "state runner of another uid is closed");
    }

    void testSynchronousChain()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=5, state='d'}");

        require(!f.alive("key_5_d") && !f.alive("key_5_e") && f.alive("key_5_f"), "only the last state of a chain should be running");
        require(f.isTrue("closed_5_d") && f.isTrue("closed_5_e"), "state runners in a chain are not closed");
        require(f.isNil("after_5_d") && f.isNil("after_5_e"), "state runners in a chain continued after going to next state");
        require(f.inState(5, "SYS_QSTFSM", "'f'"), "quest state is not the last state of a chain");

        f.drive("setQuestState{uid=5, state='c'}");
        require(!f.alive("key_5_f") && f.isTrue("closed_5_f"), "last state runner of a chain is not registered");
    }

    void testQuestDoneClosesAllFSM()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=6, state='b'} setQuestState{uid=6, fsm='sub', state='s1'}");
        require(f.alive("key_6_b") && f.alive("key_6_s1"), "state runners of two FSMs are not running together");

        f.drive("setQuestState{uid=6, state=SYS_DONE}");
        require(!f.alive("key_6_b") && f.isTrue("closed_6_b"), "quest done doesn't close state runner of main FSM");
        require(!f.alive("key_6_s1") && f.isTrue("closed_6_s1"), "quest done doesn't close state runner of sub FSM");
        require(f.inState(6, "SYS_QSTFSM", "SYS_DONE"), "quest is not done");
    }

    void testSubFSMRunnerSetsQuestDone()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=7, state='b'} setQuestState{uid=7, fsm='sub', state='s2'}");

        require(!f.alive("key_7_b") && f.isTrue("closed_7_b"), "quest done by sub FSM doesn't close state runner of main FSM");
        require(!f.alive("key_7_s2") && f.isTrue("closed_7_s2"), "sub FSM state runner setting quest done is not closed");
        require(f.isNil("after_7_s2"), "sub FSM state runner continued after setting quest done");
        require(f.inState(7, "SYS_QSTFSM", "SYS_DONE"), "quest is not done");
    }

    void testRestoreClosesOldRunner()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=8, state='b'}");
        const auto oldKey = f.key("key_8_b");

        f.drive("_RSVD_NAME_restoreQuestState(8, SYS_QSTFSM, 'b', nil)");
        require(!f.runner.hasKey(oldKey) && f.isTrue("closed_8_b"), "restore doesn't close old state runner");
        require(f.key("key_8_b") != oldKey && f.alive("key_8_b"), "restore doesn't start state again");
    }

    void testFinishedRunner()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=9, state='g'}");
        require(!f.alive("key_9_g") && f.isTrue("closed_9_g"), "state runner is not erased when its state function returns");

        f.drive("setQuestState{uid=9, state='b'}");
        require(f.alive("key_9_b"), "state change after a state function returned doesn't work");
    }

    void testRunnerClosedByThreadItStarts()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=10, state='h'}");

        require(f.isNil("after_10_h"), "state runner continued after a thread it started switched its state");
        require(!f.alive("key_10_h") && f.isTrue("closed_10_h"), "state runner switched by a thread it started is not closed");
        require(f.alive("key_10_b") && f.isNil("closed_10_b") && f.inState(10, "SYS_QSTFSM", "'b'"), "state switched to by a thread the state runner started is not running");

        f.drive("setQuestState{uid=10, state='c'}");
        require(!f.alive("key_10_b") && f.isTrue("closed_10_b"), "state runner started by a thread of the old state runner is not registered");
        require(f.alive("key_10_c"), "state change after the old state runner got closed by a thread it started doesn't work");
    }

    void testNoStateSwitchInSelfClose()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=11, state='i'}");

        require(!f.alive("key_11_i") && f.isTrue("closed_11_i"), "state runner switching its own state is not closed");
        require(f.isNil("key_11_failed"), "<close> handler of a state runner closing itself switched state");
        require(f.alive("key_11_b") && f.inState(11, "SYS_QSTFSM", "'b'"), "state switch of a state runner closing itself is undone by its <close> handler");
        require(capture.has("Error in <close> handler while closing runner"), "error in <close> handler is not logged");
        require(capture.has("setQuestState() is not allowed while a thread is being closed, i.e. in a <close> handler: uid 11,"), "state switch in <close> handler doesn't raise");

        f.drive("setQuestState{uid=11, state='c'}");
        require(!f.alive("key_11_b") && f.isTrue("closed_11_b") && f.alive("key_11_c"), "state runner started by a state runner closing itself is not registered");
    }

    void testNoStateSwitchInCloseByOther()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=12, state='i', args='pause'}");
        require(f.alive("key_12_i"), "state runner doesn't pause");

        f.drive("setQuestState{uid=12, state='c'}");
        require(!f.alive("key_12_i") && f.isTrue("closed_12_i"), "suspended state runner is not closed");
        require(f.isNil("key_12_failed"), "<close> handler of a state runner closed by other thread switched state");
        require(f.alive("key_12_c") && f.inState(12, "SYS_QSTFSM", "'c'"), "state switch by other thread is undone by <close> handler of the old state runner");
        require(capture.has("Error in <close> handler while closing runner"), "error in <close> handler is not logged");
        require(capture.has("setQuestState() is not allowed while a thread is being closed, i.e. in a <close> handler: uid 12,"), "state switch in <close> handler doesn't raise");

        f.drive("setQuestState{uid=12, state='b'}");
        require(!f.alive("key_12_c") && f.isTrue("closed_12_c") && f.alive("key_12_b"), "state runner started by other thread is not registered");
    }

    void testNoStateSwitchInQuestDone()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=13, state='i', args='pause'}");
        require(f.alive("key_13_i"), "state runner doesn't pause");

        f.drive("setQuestState{uid=13, state=SYS_DONE}");
        require(!f.alive("key_13_i") && f.isTrue("closed_13_i"), "quest done doesn't close state runner");
        require(f.isNil("key_13_failed") && f.inState(13, "SYS_QSTFSM", "SYS_DONE"), "quest done is undone by <close> handler of the old state runner");
        require(capture.has("setQuestState() is not allowed while a thread is being closed, i.e. in a <close> handler: uid 13,"), "state switch in <close> handler doesn't raise");
    }

    void testNoStateSwitchAfterError()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=14, state='k'}");

        require(!f.alive("key_14_k") && f.isTrue("closed_14_k"), "state runner raised is not closed");
        require(f.isNil("key_14_failed") && f.inState(14, "SYS_QSTFSM", "'k'"), "<close> handler of a state runner that raised switched state");
        require(capture.has("Runner error replaced by error in <close> handler"), "error replaced by <close> handler is not logged");
        require(capture.has("state k failed"), "original error is not logged");
        require(capture.has("setQuestState() is not allowed while a thread is being closed, i.e. in a <close> handler: uid 14,"), "state switch in <close> handler doesn't raise");

        f.drive("setQuestState{uid=14, state='c'}");
        require(f.alive("key_14_c"), "state change after a state function raised doesn't work");
    }

    void testSelfCloseCheckedFirst()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=15, state='j'}");

        require(f.isTrue("wrapRaised_15"), "state runner switching its own state from a coroutine created in it doesn't raise");
        require(f.isTrue("sortRaised_15"), "state runner switching its own state where it can't yield doesn't raise");
        require(f.isNil("key_15_b") && f.inState(15, "SYS_QSTFSM", "'j'"), "state switch that can't close the state runner has changed quest state");
        require(f.isTrue("after_15_j") && f.alive("key_15_j") && f.isNil("closed_15_j"), "state runner is closed by a state switch that raised");

        f.drive("setQuestState{uid=15, state='c'}");
        require(!f.alive("key_15_j") && f.isTrue("closed_15_j"), "state runner is unregistered by a state switch that raised");
        require(f.alive("key_15_c"), "state change after a state switch that raised doesn't work");
    }

    void runTests()
    {
        testRunnerGoesToNextState();
        testOtherThreadChangesState();
        testRunnerSetsOtherUID();
        testSynchronousChain();
        testQuestDoneClosesAllFSM();
        testSubFSMRunnerSetsQuestDone();
        testRestoreClosesOldRunner();
        testFinishedRunner();
        testRunnerClosedByThreadItStarts();
        testNoStateSwitchInSelfClose();
        testNoStateSwitchInCloseByOther();
        testNoStateSwitchInQuestDone();
        testNoStateSwitchAfterError();
        testSelfCloseCheckedFirst();
    }
}

int main()
{
    try{
        char arg0[] = "queststate_test";
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
        Log log("mir2x-queststate-test");
        std::cout.rdbuf(savedCoutBuf);
        g_mir2xLog = &log;

        Server server;
        g_server = &server;

        runTests();
        std::printf("Quest state runner passed: go to next state, state changed by other thread, set state of other uid, synchronous chain, quest done closes all FSMs, sub FSM sets quest done, restore, finished state, runner closed by a thread it starts, no state switch while closing, and self close checked before any change.\n");

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
