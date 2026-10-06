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

        -- quest done looks up the map of a recorded grid trigger, it waits here till TEST.mapLoaded, as for a slow map, then finds no map
        function loadBaseMap(mapName)
            while not TEST.mapLoaded do
                coroutine.yield()
            end
            return nil
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

        local function trace(uid, s)
            TEST['trace_' .. uid] = (TEST['trace_' .. uid] or '') .. s .. ','
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

            l = function(uid, args)
                local guard <close> = enter(uid, 'l')
                local order <close> = setmetatable({}, {__close = function()
                    trace(uid, 'close')
                end})

                if args == 'typo' then
                    setQuestState{uid=uid, state='nosuchstate'}
                end
                error('state l failed')
            end,

            n = stateWithFallback(function(uid, args)
                local guard <close> = enter(uid, 'n')
                trace(uid, 'func')
                error('state n failed')
            end,

            function(uid, args, err)
                TEST['fallbackN_' .. uid] = err
                trace(uid, 'fallback')
                if args == 'raise' then
                    error('fallback n failed')
                elseif args ~= 'stay' then
                    setQuestState{uid=uid, state='failed'}
                    after(uid, 'n')
                end
            end),

            r = stateWithFallback(function(uid, args)
                local guard <close> = enter(uid, 'r')
                uidRemoteCall(args, [[ return true ]])
                after(uid, 'r')
            end,

            function(uid, args, err)
                TEST['fallbackR_' .. uid] = err
                setQuestState{uid=uid, state='failed'}
            end),

            q = function(uid, args)
                local guard <close> = enter(uid, 'q')
                TEST['fromOther_' .. uid] = setQuestState{uid=uid, from='c', state='b'}
                after(uid, 'q')
                setQuestState{uid=uid, from='q', state='b'}
                TEST['afterSwitch_' .. uid] = true
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

            t1 = function(uid, args)
                local guard <close> = enter(uid, 't1')
                local order <close> = setmetatable({}, {__close = function()
                    trace(uid, 'exit t1')
                end})
                setQuestState{uid=uid, state='t2'}
            end,

            t2 = function(uid, args)
                local guard <close> = enter(uid, 't2')
                trace(uid, 'enter t2')
                pause(SYS_POSINF)
            end,

            cyc = function(uid, args)
                TEST['cycCount_' .. uid] = (TEST['cycCount_' .. uid] or 0) + 1
                setQuestState{uid=uid, state='cyc'}
            end,

            t3 = function(uid, args)
                local guard <close> = enter(uid, 't3')
                local order <close> = setmetatable({}, {__close = function()
                    trace(uid, 'exit t3')
                end})
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

    void testFallbackOnRaise()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=20, state='l', args='x', fallback=function(uid, args, err) TEST.fbUID_20 = uid TEST.fbArgs_20 = args TEST.fbErr_20 = err TEST.trace_20 = (TEST.trace_20 or '') .. 'fallback,' setQuestState{uid=uid, state='failed'} TEST.fbAfter_20 = true end}");

        require(!f.alive("key_20_l") && f.isTrue("closed_20_l"), "state runner is not closed after its fallback switched state");
        require(f.isNil("fbAfter_20"), "fallback continued after switching state");
        require(f.alive("key_20_failed") && f.inState(20, "SYS_QSTFSM", "'failed'"), "fallback didn't switch state");
        require(f.str("trace_20") == "close,fallback,", "<close> handler doesn't run before fallback");
        require(f.get("fbUID_20").as<int>() == 20 && f.str("fbArgs_20") == "x", "fallback gets wrong uid or args");
        require(f.strHas("fbErr_20", "state l failed") && f.strHas("fbErr_20", "stack traceback"), "fallback gets no error with traceback");
        require(capture.has("Quest state raised: uid 20, fsm ") && capture.has("state l failed"), "error caught for fallback is not logged");

        f.drive("setQuestState{uid=20, state='c'}");
        require(!f.alive("key_20_failed") && f.isTrue("closed_20_failed") && f.alive("key_20_c"), "state runner started by fallback is not registered");
    }

    void testFallbackNotCalled()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=21, state='b', fallback=function() TEST.fb_21 = true end}");
        require(f.alive("key_21_b") && f.isNil("fb_21"), "fallback is called for a state function that pauses");

        f.drive("setQuestState{uid=21, state='a', fallback=function() TEST.fb_21 = true end}");
        require(!f.alive("key_21_a") && f.isTrue("closed_21_a") && f.isNil("after_21_a"), "state runner with fallback is not closed after going to next state");
        require(f.alive("key_21_b") && f.isNil("fb_21"), "fallback is called for a state function that goes to next state");

        f.drive("setQuestState{uid=21, state='c', fallback=function() TEST.fb_21 = true end}");
        f.drive("setQuestState{uid=21, state='b'}");
        require(!f.alive("key_21_c") && f.isTrue("closed_21_c") && f.isNil("fb_21"), "fallback is called for a state runner closed by other thread");

        f.drive("setQuestState{uid=21, state='g', fallback=function() TEST.fb_21 = true end, exitfunc=function() TEST.exit_21 = true end}");
        require(f.isNil("fb_21") && f.isTrue("exit_21"), "fallback is called, or exitfunc is not, for a state function that returns");
    }

    void testFallbackNoSwitch()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=22, state='l', fallback=function() TEST.fb_22 = true end, exitfunc=function() TEST.exit_22 = true end}");

        require(f.isTrue("fb_22") && f.isNil("exit_22"), "fallback is not called, or exitfunc is, for a state function that raises");
        require(!f.alive("key_22_l") && f.isTrue("closed_22_l") && f.inState(22, "SYS_QSTFSM", "'l'"), "state runner is not done after fallback returned without switching state");

        f.drive("setQuestState{uid=22, state='c'}");
        require(f.alive("key_22_c"), "state change after fallback returned without switching state doesn't work");
    }

    void testFallbackCatchesBadSwitch()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=23, state='l', args='typo', fallback=function(uid, args, err) TEST.fbErr_23 = err setQuestState{uid=uid, state='failed'} end}");

        require(f.strHas("fbErr_23", "Invalid arguments: fsm ") && f.strHas("fbErr_23", "state nosuchstate"), "fallback doesn't catch an invalid state switch");
        require(!f.alive("key_23_l") && f.alive("key_23_failed") && f.inState(23, "SYS_QSTFSM", "'failed'"), "fallback doesn't recover from an invalid state switch");
    }

    void testFallbackNeedsStateFunction()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=24, state='b'} TEST.doneOK_24, TEST.doneErr_24 = pcall(setQuestState, {uid=24, state=SYS_DONE, fallback=function() end})");

        require(f.get("doneOK_24").is<bool>() && !f.isTrue("doneOK_24") && f.strHas("doneErr_24", "fallback given to fsm "), "fallback for a state without state function doesn't raise");
        require(f.alive("key_24_b") && f.inState(24, "SYS_QSTFSM", "'b'"), "state switch with invalid fallback has changed quest state");
    }

    void testCloseHandlerSwitchBeforeFallback()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=29, state='k', fallback=function() TEST.fb_29 = true end}");

        require(f.isNil("fb_29"), "fallback is called after a <close> handler switched state");
        require(!f.alive("key_29_k") && f.isTrue("closed_29_k"), "state runner is not closed after its <close> handler switched state");
        require(f.alive("key_29_failed") && f.inState(29, "SYS_QSTFSM", "'failed'"), "<close> handler can't switch state while xpcall() unwinds");
        require(capture.has("Quest state raised: uid 29, fsm ") && capture.has("state k failed"), "error is lost when a <close> handler switched state");
    }

    void testStateWithFallback()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=25, state='n'}");

        require(f.strHas("fallbackN_25", "state n failed") && f.strHas("fallbackN_25", "stack traceback"), "fallback of stateWithFallback() gets no error with traceback");
        require(!f.alive("key_25_n") && f.isNil("after_25_n"), "state runner is not closed after fallback of stateWithFallback() switched state");
        require(f.alive("key_25_failed") && f.inState(25, "SYS_QSTFSM", "'failed'"), "fallback of stateWithFallback() didn't switch state");
        require(capture.has("Quest state raised: uid 25"), "error caught for stateWithFallback() is not logged");

        const auto oldKey = f.key("key_25_failed");
        f.drive("_RSVD_NAME_restoreQuestState(25, SYS_QSTFSM, 'n', nil)");
        require(!f.runner.hasKey(oldKey) && f.key("key_25_failed") != oldKey && f.alive("key_25_failed"), "fallback of stateWithFallback() doesn't work after restore");
    }

    void testFallbackNested()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=26, state='n', args='raise', fallback=function(uid, args, err) TEST.fbErr_26 = err setQuestState{uid=uid, state='failed'} end}");
        require(f.strHas("fbErr_26", "fallback n failed") && f.str("trace_26") == "func,fallback,", "fallback of setQuestState() doesn't catch a raise in fallback of stateWithFallback()");
        require(!f.alive("key_26_n") && f.alive("key_26_failed") && f.inState(26, "SYS_QSTFSM", "'failed'"), "fallback of setQuestState() didn't switch state");

        f.drive("setQuestState{uid=27, state='n', args='stay', fallback=function() TEST.fb_27 = true end, exitfunc=function() TEST.exit_27 = true end}");
        require(f.isNil("fb_27") && f.isTrue("exit_27"), "fallback of stateWithFallback() returning doesn't count as the state function returning");
        require(!f.alive("key_27_n") && f.inState(27, "SYS_QSTFSM", "'n'"), "state runner is not done after fallback of stateWithFallback() returned");

        f.drive("setQuestState{uid=28, state='n', fallback=function() TEST.fb_28 = true end}");
        require(f.isNil("fb_28") && f.alive("key_28_failed") && f.isNil("after_28_n"), "fallback of setQuestState() is called after fallback of stateWithFallback() switched state");
    }

    void testFallbackCatchesRemoteError()
    {
        QuestFixture f;
        CoutCapture capture;

        // the remote code raised, _RSVD_NAME_remoteCall passes its error on as SYS_EXECERROR
        require(f.runner.execRawString("_G['_RSVD_NAME_remoteCall' .. SYS_COOP] = function(uid, code, args, onDone) onDone(SYS_EXECERROR, 'remote side raised') end").valid(), "failed to stub remote call");
        f.drive("setQuestState{uid=30, state='r', args=" + std::to_string(uidf::getQuestUID(2)) + "}");

        require(f.strHas("fallbackR_30", "Remote call to QST_2 failed: remote side raised"), "fallback doesn't get the error raised by the remote code");
        require(!f.alive("key_30_r") && f.isNil("after_30_r"), "state runner goes on after its remote call raised");
        require(f.alive("key_30_failed") && f.inState(30, "SYS_QSTFSM", "'failed'"), "fallback doesn't switch state after a remote error");
    }

    void testQuestRuntimeVar()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=31, state='b'} setQuestRuntimeVar(31, 'timer', 7) setQuestRuntimeVar(31, 'mapUID', 8) setQuestRuntimeVar(31, 'mapUID', nil) setQuestRuntimeVar(32, 'timer', 9)");

        require(f.runner.execRawString("TEST.rtSet = getQuestRuntimeVar(31, 'timer') == 7 and getQuestRuntimeVar(31, 'mapUID') == nil and getQuestRuntimeVar(32, 'timer') == 9").valid(), "failed to read runtime vars");
        require(f.isTrue("rtSet"), "runtime var doesn't keep its value, or setting nil doesn't remove it");

        require(f.runner.execRawString("TEST.rtNotSaved = TEST.db[31] ~= nil and TEST.db[31].fld_vars == nil").valid(), "failed to read quest db");
        require(f.isTrue("rtNotSaved"), "runtime var is saved in the quest db");

        f.drive("setQuestState{uid=31, state=SYS_DONE}");
        require(f.runner.execRawString("TEST.rtDone = getQuestRuntimeVar(31, 'timer') == nil and getQuestRuntimeVar(32, 'timer') == 9").valid(), "failed to read runtime vars");
        require(f.isTrue("rtDone"), "quest done doesn't drop the runtime vars of its uid only");
    }

    void testSetStateFrom()
    {
        QuestFixture f;
        const auto isFalse = [&f](const std::string &name){ return f.get(name).is<bool>() && !f.isTrue(name); };

        f.drive("TEST.plain = setQuestState{uid=40, state='b'}");
        require(f.isTrue("plain"), "switch by another thread doesn't return true");
        const auto keyB = f.key("key_40_b");

        f.drive("TEST.fromOther = setQuestState{uid=40, from='c', state='c'}");
        require(isFalse("fromOther"), "switch from another state doesn't return false");
        require(f.runner.hasKey(keyB) && f.isNil("key_40_c") && f.inState(40, "SYS_QSTFSM", "'b'"), "switch from another state changes something");

        f.drive("TEST.fromList = setQuestState{uid=40, from={'a', 'b'}, state='c'}");
        require(f.isTrue("fromList"), "switch from one of the given states doesn't return true");
        require(!f.runner.hasKey(keyB) && f.alive("key_40_c") && f.inState(40, "SYS_QSTFSM", "'c'"), "switch from one of the given states doesn't switch");

        f.drive("TEST.fromDone = setQuestState{uid=40, from='b', state=SYS_DONE}");
        require(isFalse("fromDone") && f.alive("key_40_c") && f.inState(40, "SYS_QSTFSM", "'c'"), "quest done from another state changes something");

        f.drive("TEST.fromNoState = setQuestState{uid=41, from='b', state='c'}");
        require(isFalse("fromNoState") && f.isNil("key_41_c") && f.inState(41, "SYS_QSTFSM", "nil"), "switch of a quest not started changes something");

        f.drive("setQuestState{uid=42, state='q'}");
        require(isFalse("fromOther_42") && f.isTrue("after_42_q"), "state runner doesn't go on after its switch from another state returned false");
        require(!f.alive("key_42_q") && f.isNil("afterSwitch_42") && f.alive("key_42_b") && f.inState(42, "SYS_QSTFSM", "'b'"), "state runner switching from its own state doesn't end");
    }

    void testOldStateClosedFirst()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=50, state='t1'}");

        require(f.str("trace_50") == "exit t1,enter t2,", "new state function runs before the <close> handlers of the state runner switching its own state");
        require(!f.alive("key_50_t1") && f.isTrue("closed_50_t1") && f.alive("key_50_t2") && f.inState(50, "SYS_QSTFSM", "'t2'"), "state runner switching its own state doesn't end in the new state");

        f.drive("setQuestState{uid=50, state='c'}");
        require(!f.alive("key_50_t2") && f.isTrue("closed_50_t2") && f.alive("key_50_c"), "state runner started after the old one closed is not registered");

        f.drive("setQuestState{uid=51, state='t3'}");
        f.drive("setQuestState{uid=51, state='t2'}");
        require(f.str("trace_51") == "exit t3,enter t2,", "new state function runs before the <close> handlers of the state runner closed by other thread");
    }

    void testSwitchCycleStops()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=52, state='cyc'}");

        // the driver thread is the first of the 64 threads on the C stack
        const auto cycCount = f.get("cycCount_52");
        require(cycCount.is<int>() && cycCount.as<int>() == 63, "state switches in a cycle with no yield don't stop at 64 threads on the C stack");
        require(capture.has("threads run on top of each other on the C stack"), "state switch over the limit doesn't raise");
        require(f.inState(52, "SYS_QSTFSM", "'cyc'"), "state switch over the limit changes the quest state");

        f.drive("setQuestState{uid=52, state='c'}");
        require(f.alive("key_52_c") && f.inState(52, "SYS_QSTFSM", "'c'"), "state change after a cycle of state switches stopped doesn't work");
    }

    void testNoStateSwitchDuringQuestDone()
    {
        QuestFixture f;
        CoutCapture capture;

        f.drive("setQuestState{uid=54, state='b'} TEST.db[54].fld_gridtriggers = {{'slowMap'}}");
        const auto keyB = f.key("key_54_b");

        const auto kp = f.runner.spawn(f.driverKey++, std::string("setQuestState{uid=54, state=SYS_DONE} TEST.done_54 = true"));
        require(f.runner.hasKeyPair(kp) && f.isNil("done_54"), "quest done doesn't wait in its remote call");

        f.drive("TEST.switchOK_54, TEST.switchErr_54 = pcall(setQuestState, {uid=54, state='c'})");
        require(f.get("switchOK_54").is<bool>() && !f.isTrue("switchOK_54") && f.strHas("switchErr_54", "setQuestState() is not allowed while quest done of uid 54 runs"), "state switch while quest done runs doesn't raise");

        f.drive("_RSVD_NAME_restoreQuestState(54, SYS_QSTFSM, 'b', nil)");
        require(f.runner.hasKey(keyB) && f.key("key_54_b") == keyB, "restore while quest done runs closes or starts a state runner");
        require(capture.has("Quest done of uid 54 runs, fsm "), "restore skipped while quest done runs is not logged");

        require(f.runner.execRawString("TEST.mapLoaded = true").valid(), "failed to load the map");
        f.runner.resume(kp);
        require(!f.runner.hasKeyPair(kp) && f.isTrue("done_54"), "quest done doesn't finish");
        require(!f.runner.hasKey(keyB) && f.isTrue("closed_54_b") && f.isNil("key_54_c") && f.inState(54, "SYS_QSTFSM", "SYS_DONE"), "quest done is undone, or doesn't close the state runner");

        // a quest done whose caller is closed while it runs drops its mark too
        require(f.runner.execRawString("TEST.mapLoaded = nil").valid(), "failed to make the map slow again");
        f.drive("setQuestState{uid=55, state='b'} TEST.db[55].fld_gridtriggers = {{'slowMap'}}");

        const auto kpClosed = f.runner.spawn(f.driverKey++, std::string("setQuestState{uid=55, state=SYS_DONE}"));
        f.runner.close(kpClosed);

        f.drive("setQuestState{uid=55, state='c'}");
        require(f.alive("key_55_c") && f.inState(55, "SYS_QSTFSM", "'c'"), "quest done closed while it runs keeps refusing state switches");
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
        testFallbackOnRaise();
        testFallbackNotCalled();
        testFallbackNoSwitch();
        testFallbackCatchesBadSwitch();
        testFallbackNeedsStateFunction();
        testCloseHandlerSwitchBeforeFallback();
        testStateWithFallback();
        testFallbackNested();
        testFallbackCatchesRemoteError();
        testQuestRuntimeVar();
        testSetStateFrom();
        testOldStateClosedFirst();
        testSwitchCycleStops();
        testNoStateSwitchDuringQuestDone();
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
        std::printf("Quest state runner passed: go to next state, state changed by other thread, set state of other uid, synchronous chain, quest done closes all FSMs, sub FSM sets quest done, restore, finished state, runner closed by a thread it starts, no state switch while closing, self close checked before any change, fallback of setQuestState() and stateWithFallback(), fallback of a remote error, runtime vars, switch from a given state, old state closed before the new one starts, state switches in a cycle with no yield stop, and no state switch or restore while quest done runs.\n");

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
