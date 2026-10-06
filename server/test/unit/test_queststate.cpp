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

        function getQuestScriptHash()
            return 'hash1'
        end

        local function copy(value)
            if type(value) ~= 'table' then
                return value
            end

            local result = {}
            for k, v in pairs(value) do
                result[k] = copy(v)
            end
            return result
        end

        -- a copy, as the real database returns a new table on every read, a table read before doesn't see later writes
        function dbGetQuestField(uid, field)
            return copy((TEST.db[uid] or {})[field])
        end

        function dbSetQuestField(uid, field, value)
            TEST.db[uid] = TEST.db[uid] or {}
            TEST.db[uid][field] = copy(value)
        end

        function _RSVD_NAME_setQuestDesp(uid, despTable, fsm, desp)
            dbSetQuestField(uid, 'fld_desp', despTable)
        end

        -- TEST.lastFields_<uid>: the fields of the last write of several fields
        function _RSVD_NAME_dbSetQuestFields(uid, fields, replace)
            if replace or (TEST.db[uid] == nil) then
                TEST.db[uid] = {}
            end

            TEST['lastFields_' .. uid] = {}
            for field, value in pairs(fields) do
                if value == SYS_LUANIL then
                    TEST.db[uid][field] = nil
                else
                    TEST.db[uid][field] = copy(value)
                end
                TEST['lastFields_' .. uid][field] = true
            end
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

            -- restored first at login, they switch the other fsm, or do quest done, before they yield
            rs = function(uid, args)
                local guard <close> = enter(uid, 'rs')
                setQuestState{uid=uid, fsm='sub', state='s1'}
                pause(SYS_POSINF)
            end,

            rd = function(uid, args)
                local guard <close> = enter(uid, 'rd')
                setQuestState{uid=uid, state=SYS_DONE}
            end,

            rsd = function(uid, args)
                local guard <close> = enter(uid, 'rsd')
                setQuestState{uid=uid, fsm='sub', state=SYS_DONE}
                pause(SYS_POSINF)
            end,

            -- its <close> handler switches the uid given as args
            swOther = function(uid, args)
                local guard <close> = enter(uid, 'swOther')
                local switcher <close> = setmetatable({}, {__close = function()
                    setQuestState{uid=args, state='b'}
                end})
                pause(SYS_POSINF)
            end,

            -- switches the uid given as args, as a team quest does
            cross = function(uid, args)
                local guard <close> = enter(uid, 'cross')
                setQuestState{uid=args, state='c'}
                after(uid, 'cross')
                pause(SYS_POSINF)
            end,

            closer = function(uid, args)
                local guard <close> = enter(uid, 'closer')
                local killer <close> = setmetatable({}, {__close = function()
                    closeThread(TEST['killKey_' .. uid])
                end})
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

            -- runs the code given as args on the state runner
            run = function(uid, args)
                load(args)(uid)
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
                TEST['s1Count_' .. uid] = (TEST['s1Count_' .. uid] or 0) + 1
                pause(SYS_POSINF)
            end,

            s3 = function(uid, args)
                local guard <close> = enter(uid, 's3')
                pause(SYS_POSINF)
            end,

            s2 = function(uid, args)
                local guard <close> = enter(uid, 's2')
                setQuestState{uid=uid, state=SYS_DONE}
                after(uid, 's2')
            end,

            run = function(uid, args)
                load(args)(uid)
                pause(SYS_POSINF)
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

        // NPC actors as stubs, getNPCharUID() finds TEST.npcs[map .. '/' .. npc]
        // a remote call takes effect in TEST.world[npcUID][playerUID] as it's sent, as the NPC handles its messages in order
        // it's recorded in TEST.calls with the key pair of its caller, its reply waits while TEST.holdReplies, see reply()
        // TEST.refuse makes the NPC side raise
        void stubNPCs()
        {
            require(runner.execRawString(R"###(
                TEST.npcs = {}
                TEST.calls = {}
                TEST.world = {}

                function getNPCharUID(mapName, npcName)
                    return TEST.npcs[mapName .. '/' .. npcName]
                end

                _G['_RSVD_NAME_remoteCall' .. SYS_COOP] = function(uid, code, args, onDone)
                    local call = {uid = uid, onDone = onDone, key = getThreadKey(), seq = getThreadSeqID()}
                    table.insert(TEST.calls, call)

                    if TEST.refuse then
                        call.result = {SYS_EXECERROR, 'refused by the npc'}
                    else
                        TEST.world[uid] = TEST.world[uid] or {}
                        if string.find(code, 'setUIDQuestHandler', 1, true) then
                            TEST.world[uid][args[1]] = args[3]
                        elseif string.find(code, 'deleteUIDQuestHandler', 1, true) then
                            TEST.world[uid][args[1]] = nil
                        end
                        call.result = {SYS_EXECDONE}
                    end

                    if not TEST.holdReplies then
                        onDone(table.unpack(call.result))
                    end
                end
            )###").valid(), "failed to stub NPCs");
        }

        // replies to the call-th remote call, and resumes its caller if it's still there
        void reply(int call)
        {
            const auto code = "local call = TEST.calls[" + std::to_string(call) + "] call.onDone(table.unpack(call.result)) TEST.replyKey, TEST.replySeq = call.key, call.seq";
            require(runner.execRawString(code.c_str()).valid(), "failed to reply");

            const std::pair<uint64_t, uint64_t> kp{get("replyKey").as<uint64_t>(), get("replySeq").as<uint64_t>()};
            if(runner.hasKeyPair(kp)){
                runner.resume(kp);
            }
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
        f.drive("setQuestState{uid=6, state='b'} setQuestState{uid=6, fsm='sub', state='s1'} dbSetQuestVar(6, 'k', 1)");
        require(f.alive("key_6_b") && f.alive("key_6_s1"), "state runners of two FSMs are not running together");

        f.drive("setQuestState{uid=6, state=SYS_DONE}");
        require(!f.alive("key_6_b") && f.isTrue("closed_6_b"), "quest done doesn't close state runner of main FSM");
        require(!f.alive("key_6_s1") && f.isTrue("closed_6_s1"), "quest done doesn't close state runner of sub FSM");
        require(f.inState(6, "SYS_QSTFSM", "SYS_DONE"), "quest is not done");

        f.drive("TEST.doneRow_6 = (TEST.db[6].fld_vars == nil) and (TEST.db[6].fld_states.sub == nil)");
        require(f.isTrue("doneRow_6"), "quest done keeps a field of the row other than the done state of the main FSM");
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
        require(capture.has("setQuestState() is not allowed while another switch of uid 11 runs"), "state switch in <close> handler doesn't raise");

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
        require(capture.has("setQuestState() is not allowed while another switch of uid 12 runs"), "state switch in <close> handler doesn't raise");

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
        require(capture.has("setQuestState() is not allowed while another switch of uid 13 runs"), "state switch in <close> handler doesn't raise");
    }

    void testNoStateSwitchInRestore()
    {
        QuestFixture f;
        CoutCapture capture;
        f.drive("setQuestState{uid=16, state='i', args='pause'}");
        const auto oldKey = f.key("key_16_i");

        // the restore at login closes the old state runner, its <close> handler tries to switch state
        f.drive("_RSVD_NAME_restoreQuestState(16, SYS_QSTFSM, 'i', 'pause')");
        require(!f.runner.hasKey(oldKey) && f.alive("key_16_i") && f.key("key_16_i") != oldKey, "restore doesn't start the state again");
        require(f.isNil("key_16_failed") && f.inState(16, "SYS_QSTFSM", "'i'"), "<close> handler of the old state runner switched state during a restore");
        require(capture.has("setQuestState() is not allowed while another switch of uid 16 runs"), "state switch in <close> handler during a restore doesn't raise");
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
        // no switch of uid 14 runs, the state runner that raised can't end itself in its <close> handler
        require(capture.has("switching its own state by setQuestState() where it can't yield: uid 14,"), "state switch in <close> handler doesn't raise");

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
        require(f.get("switchOK_54").is<bool>() && !f.isTrue("switchOK_54") && f.strHas("switchErr_54", "setQuestState() is not allowed while another switch of uid 54 runs"), "state switch while quest done runs doesn't raise");

        f.drive("_RSVD_NAME_restoreQuestState(54, SYS_QSTFSM, 'b', nil)");
        require(f.runner.hasKey(keyB) && f.key("key_54_b") == keyB, "restore while quest done runs closes or starts a state runner");
        require(capture.has("Another switch of uid 54 runs, i.e. its quest done, fsm "), "restore skipped while quest done runs is not logged");

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

    void testRestoreReadsEachFSM()
    {
        QuestFixture f;

        // the db as a former run left it
        f.drive("TEST.db[56] = {fld_states = {[SYS_QSTFSM] = {'b'}, sub = {'s1'}}} _RSVD_NAME_restoreQuestStates(56)");
        require(f.alive("key_56_b") && f.alive("key_56_s1"), "restore doesn't start every fsm");

        // main is restored first, its state function switches sub before it yields
        f.drive("TEST.db[57] = {fld_states = {[SYS_QSTFSM] = {'rs'}, sub = {'s3'}}} _RSVD_NAME_restoreQuestStates(57)");
        require(f.alive("key_57_rs") && f.alive("key_57_s1") && f.isNil("key_57_s3") && f.inState(57, "'sub'", "'s1'"), "restore overrides the switch of a state function restored before");

        const auto s1Count = f.get("s1Count_57");
        require(s1Count.is<int>() && s1Count.as<int>() == 1, "restore starts a fsm again that a state function restored before switched");

        f.drive("TEST.db[58] = {fld_states = {[SYS_QSTFSM] = {'rd'}, sub = {'s3'}}} _RSVD_NAME_restoreQuestStates(58)");
        require(f.isNil("key_58_s3") && f.inState(58, "SYS_QSTFSM", "SYS_DONE"), "restore goes on after a state function restored before did quest done");

        // a switch to SYS_DONE starts no state runner, only the state read again tells
        f.drive("TEST.db[59] = {fld_states = {[SYS_QSTFSM] = {'rsd'}, sub = {'s3'}}} _RSVD_NAME_restoreQuestStates(59)");
        require(f.alive("key_59_rsd") && f.isNil("key_59_s3") && f.inState(59, "'sub'", "SYS_DONE"), "restore overrides a fsm a state function restored before set to SYS_DONE");
    }

    void testClosedDuringSwitch()
    {
        QuestFixture f;
        f.drive("setQuestState{uid=53, state='closer'}");

        // the <close> handler of the old state runner closes the driver thread switching its state
        f.drive("TEST.killKey_53 = getThreadKey() TEST.switched_53 = setQuestState{uid=53, state=SYS_DONE} TEST.afterSwitch_53 = true");

        require(f.isNil("switched_53") && f.isNil("afterSwitch_53"), "thread closed during its state switch goes on after the switch");
        require(f.isTrue("closed_53_closer") && f.inState(53, "SYS_QSTFSM", "SYS_DONE"), "state switch that closed its caller isn't done");
    }

    void testCloseHandlerSwitchesOtherUID()
    {
        QuestFixture f;

        // the <close> handler of the old state runner of uid 60 switches uid 61, the switch of uid 60 doesn't refuse it
        f.drive("setQuestState{uid=60, state='swOther', args=61}");
        f.drive("setQuestState{uid=60, state='c'}");

        require(f.isTrue("closed_60_swOther") && f.alive("key_60_c") && f.inState(60, "SYS_QSTFSM", "'c'"), "state switch whose old state runner switches another uid in its <close> handler isn't done");
        require(f.alive("key_61_b") && f.inState(61, "SYS_QSTFSM", "'b'"), "<close> handler of an old state runner can't switch another uid");

        // the state runner of uid 63 switches uid 62, whose old state runner switches uid 63 in its <close> handler
        f.drive("setQuestState{uid=62, state='swOther', args=63}");
        f.drive("setQuestState{uid=63, state='cross', args=62}");

        require(f.isNil("after_63_cross") && !f.alive("key_63_cross") && f.isTrue("closed_63_cross"), "state runner switched by a <close> handler of a state runner it closes goes on after its switch");
        require(f.alive("key_63_b") && f.inState(63, "SYS_QSTFSM", "'b'"), "state switch by a <close> handler of a state runner of another uid isn't done");
        require(f.alive("key_62_c") && f.inState(62, "SYS_QSTFSM", "'c'"), "state switch whose old state runner switched the caller isn't done");
    }

    void testMapGridTriggerQuest()
    {
        QuestFixture f;

        // the remote call runs its code right here, against a stub of the map side
        require(f.runner.execRawString(R"###(
            function loadBaseMap(mapName)
                return 9001
            end

            _G['_RSVD_NAME_remoteCall' .. SYS_COOP] = function(uid, code, args, onDone)
                onDone(SYS_EXECDONE, load(code)(table.unpack(args, 1, args.n)))
            end

            function addQuestGridTrigger(questName, rectList, handler)
                TEST.gridQuest = questName
                TEST.gridRect = (rectList[1][1] == 3) and (rectList[1][2] == 4)
                TEST.gridHandler = type(handler) == 'function'
                return 77
            end
        )###").valid(), "failed to stub the map side");

        f.drive("TEST.gridID = setupMapGridTrigger('someMap', 3, 4, [[ return function(uid, x, y) return false end ]])");

        const auto gridQuest = f.get("gridQuest");
        require(gridQuest.is<std::string>() && gridQuest.as<std::string>() == "queststate_test", "setupMapGridTrigger() doesn't install the trigger as a trigger of its quest");
        require(f.isTrue("gridRect") && f.isTrue("gridHandler"), "setupMapGridTrigger() doesn't pass the rect and the handler");

        const auto gridID = f.get("gridID");
        require(gridID.is<int>() && gridID.as<int>() == 77, "setupMapGridTrigger() doesn't return the id of the trigger");
    }

    void testContextWriters()
    {
        QuestFixture f;

        // a quest not started: runtime only
        f.drive("_RSVD_NAME_questContext.install(80, 'npc/m/a', {code = 'none'})");
        f.drive("TEST.ctx80 = (TEST.db[80] == nil) and (_RSVD_NAME_questContext.get(80, 'npc/m/a').item.code == 'none')");
        require(f.isTrue("ctx80"), "an install for a quest not started isn't in runtime only");

        // the state runner: runtime and pending, the database gets it when its fsm commits
        f.drive(R"###(
            setQuestState{uid=81, state='run', args=[[
                local uid = ...
                TEST.v81 = _RSVD_NAME_questContext.install(uid, 'npc/m/a', {code = 'runner'})
            ]]}
        )###");
        f.drive("TEST.ctx81 = (TEST.db[81].fld_context == nil) and (_RSVD_NAME_questContext.get(81, 'npc/m/a').version == TEST.v81)");
        require(f.isTrue("ctx81"), "an install by the state runner is written at once, or isn't in runtime");

        f.drive("_RSVD_NAME_questContext.commit(81, SYS_QSTFSM, {}) TEST.ctx81 = TEST.db[81].fld_context['npc/m/a'].code == 'runner'");
        require(f.isTrue("ctx81"), "the commit of the fsm of the state runner doesn't write its install");

        // any other thread: runtime and committed at once
        f.drive("setQuestState{uid=82, state='b'} _RSVD_NAME_questContext.install(82, 'npc/m/a', {code = 'through'})");
        f.drive("TEST.ctx82 = (TEST.db[82].fld_context['npc/m/a'].code == 'through') and (_RSVD_NAME_questContext.get(82, 'npc/m/a') ~= nil)");
        require(f.isTrue("ctx82"), "an install by another thread isn't written at once");

        f.drive("_RSVD_NAME_questContext.remove(82, 'npc/m/a') TEST.ctx82 = (TEST.db[82].fld_context == nil) and (_RSVD_NAME_questContext.get(82, 'npc/m/a') == nil)");
        require(f.isTrue("ctx82"), "a remove by another thread isn't written at once");

        // a quest done: an install raises, a remove changes nothing
        f.drive(R"###(
            setQuestState{uid=83, state=SYS_DONE}
            local ok, err = pcall(_RSVD_NAME_questContext.install, 83, 'npc/m/a', {code = 'late'})
            TEST.ctx83 = (not ok) and (string.find(err, 'its quest is done', 1, true) ~= nil)
                and (select('#', _RSVD_NAME_questContext.remove(83, 'npc/m/a')) == 0)
                and (TEST.db[83].fld_context == nil) and (_RSVD_NAME_questContext.get(83, 'npc/m/a') == nil)
        )###");
        require(f.isTrue("ctx83"), "an install into a quest done doesn't raise, or a remove changes something");
    }

    void testContextOwner()
    {
        QuestFixture f;

        // the last writer owns a key: the runner of sub takes it from the pending keys of the main fsm, with a warning
        CoutCapture capture;
        f.drive(R"###(
            setQuestState{uid=84, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/k', {code = 'main'}) ]]}
            setQuestState{uid=84, fsm='sub', state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/k', {code = 'sub'}) ]]}

            _RSVD_NAME_questContext.commit(84, SYS_QSTFSM, {})
            TEST.mainOwns84 = TEST.db[84].fld_context ~= nil

            _RSVD_NAME_questContext.commit(84, 'sub', {})
            TEST.subOwns84 = TEST.db[84].fld_context['npc/m/k'].code == 'sub'
        )###");

        require(!f.isTrue("mainOwns84") && f.isTrue("subOwns84"), "a key written by the runner of another fsm stays pending in the first fsm");
        require(capture.has("Quest context npc/m/k of uid 84 is pending in fsm ") && capture.has(", written by the state runner of fsm sub"), "a key pending in one fsm and written by the runner of another isn't logged");

        // a write by another thread takes the key too
        f.drive(R"###(
            setQuestState{uid=85, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/k', {code = 'main'}) ]]}
            _RSVD_NAME_questContext.install(85, 'npc/m/k', {code = 'through'})
            _RSVD_NAME_questContext.commit(85, SYS_QSTFSM, {})
            TEST.ctx85 = TEST.db[85].fld_context['npc/m/k'].code == 'through'
        )###");
        require(f.isTrue("ctx85"), "the commit of an fsm overwrites a key another thread wrote after it");
    }

    void testContextCommit()
    {
        QuestFixture f;

        // installs overwrite, removes delete, the fields are written with the items
        f.drive(R"###(
            setQuestState{uid=86, state='b'}
            _RSVD_NAME_questContext.install(86, 'npc/m/k1', {code = 'old1'})
            _RSVD_NAME_questContext.install(86, 'npc/m/k2', {code = 'old2'})

            setQuestState{uid=86, state='run', args=[[
                local uid = ...
                _RSVD_NAME_questContext.install(uid, 'npc/m/k1', {code = 'new1'})
                _RSVD_NAME_questContext.remove(uid, 'npc/m/k2')
                _RSVD_NAME_questContext.install(uid, 'npc/m/k3', {code = 'new3'})
            ]]}

            _RSVD_NAME_questContext.commit(86, SYS_QSTFSM, {fld_states = {[SYS_QSTFSM] = {'c'}}})
            local committed = TEST.db[86].fld_context
            TEST.ctx86 = (committed['npc/m/k1'].code == 'new1') and (committed['npc/m/k2'] == nil) and (committed['npc/m/k3'].code == 'new3')
                and (TEST.db[86].fld_states[SYS_QSTFSM][1] == 'c')

            _RSVD_NAME_questContext.commit(86, SYS_QSTFSM, {fld_vars = {x = 1}})
            TEST.ctx86again = (TEST.db[86].fld_vars.x == 1) and (TEST.db[86].fld_context['npc/m/k1'].code == 'new1')
        )###");

        require(f.isTrue("ctx86"), "a commit doesn't apply the pending installs and removes, or doesn't write the fields with them");
        require(f.isTrue("ctx86again"), "a commit with nothing pending doesn't write its fields, or changes the committed items");
    }

    void testContextRollback()
    {
        QuestFixture f;

        // the pending keys go back to their committed items, or away
        f.drive(R"###(
            setQuestState{uid=87, state='b'}
            _RSVD_NAME_questContext.install(87, 'npc/m/k1', {code = 'old1'})
            _RSVD_NAME_questContext.install(87, 'npc/m/k2', {code = 'old2'})

            setQuestState{uid=87, state='run', args=[[
                local uid = ...
                _RSVD_NAME_questContext.install(uid, 'npc/m/k1', {code = 'new1'})
                _RSVD_NAME_questContext.remove(uid, 'npc/m/k2')
                _RSVD_NAME_questContext.install(uid, 'npc/m/k3', {code = 'new3'})
            ]]}

            local changes = _RSVD_NAME_questContext.rollback(87, SYS_QSTFSM)
            local k1 = _RSVD_NAME_questContext.get(87, 'npc/m/k1')

            TEST.ctx87 = (changes['npc/m/k1'].item.code == 'old1') and (changes['npc/m/k2'].item.code == 'old2') and (changes['npc/m/k3'].item == false)
                and (k1.item.code == 'old1') and (k1.version == changes['npc/m/k1'].version)
                and (_RSVD_NAME_questContext.get(87, 'npc/m/k2').item.code == 'old2') and (_RSVD_NAME_questContext.get(87, 'npc/m/k3') == nil)

            _RSVD_NAME_questContext.commit(87, SYS_QSTFSM, {})
            local committed = TEST.db[87].fld_context
            TEST.ctx87commit = (committed['npc/m/k1'].code == 'old1') and (committed['npc/m/k2'].code == 'old2') and (committed['npc/m/k3'] == nil)
                and (next(_RSVD_NAME_questContext.rollback(87, SYS_QSTFSM)) == nil)
        )###");

        require(f.isTrue("ctx87"), "a rollback doesn't put the pending keys back to their committed items, or doesn't return them");
        require(f.isTrue("ctx87commit"), "a rollback leaves pending keys for a commit or a second rollback");
    }

    void testContextUndo()
    {
        QuestFixture f;

        // an undo puts back what the write replaced, if no write of the key came after it
        f.drive(R"###(
            setQuestState{uid=88, state='b'}
            local v1 = _RSVD_NAME_questContext.install(88, 'npc/m/k', {code = 'one'})
            local v2, undo2 = _RSVD_NAME_questContext.install(88, 'npc/m/k', {code = 'two'})

            local record = nil
            TEST.undone88 = _RSVD_NAME_questContext.undo(88, 'npc/m/k', v2, undo2)
            record = _RSVD_NAME_questContext.get(88, 'npc/m/k')
            TEST.back88 = (record.item.code == 'one') and (record.version == v1) and (TEST.db[88].fld_context['npc/m/k'].code == 'one')

            local v3, undo3 = _RSVD_NAME_questContext.install(88, 'npc/m/k', {code = 'three'})
            _RSVD_NAME_questContext.install(88, 'npc/m/k', {code = 'four'})
            TEST.stale88 = (not _RSVD_NAME_questContext.undo(88, 'npc/m/k', v3, undo3)) and (_RSVD_NAME_questContext.get(88, 'npc/m/k').item.code == 'four')
        )###");

        require(f.isTrue("undone88") && f.isTrue("back88"), "an undo of a write by another thread doesn't put back runtime and committed");
        require(f.isTrue("stale88"), "an undo of a write followed by another one changes something");

        // an undo by the state runner puts its pending key back
        f.drive(R"###(
            setQuestState{uid=89, state='run', args=[[
                local uid = ...
                _RSVD_NAME_questContext.install(uid, 'npc/m/k', {code = 'first'})
                local v, undo = _RSVD_NAME_questContext.install(uid, 'npc/m/k', {code = 'second'})
                TEST.undone89 = _RSVD_NAME_questContext.undo(uid, 'npc/m/k', v, undo)
            ]]}

            _RSVD_NAME_questContext.commit(89, SYS_QSTFSM, {})
            TEST.back89 = TEST.db[89].fld_context['npc/m/k'].code == 'first'
        )###");
        require(f.isTrue("undone89") && f.isTrue("back89"), "an undo by the state runner doesn't put its pending key back");

        // a pending table committed since the write is gone, an undo doesn't put the key into the pending table of the next state
        f.drive(R"###(
            setQuestState{uid=90, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/k', {code = 'pending'}) ]]}
            local v, undo = _RSVD_NAME_questContext.install(90, 'npc/m/k', {code = 'through'})

            _RSVD_NAME_questContext.commit(90, SYS_QSTFSM, {})
            setQuestState{uid=90, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/other', {code = 'next'}) ]]}

            TEST.undone90 = _RSVD_NAME_questContext.undo(90, 'npc/m/k', v, undo)
            _RSVD_NAME_questContext.commit(90, SYS_QSTFSM, {})

            local committed = TEST.db[90].fld_context
            TEST.back90 = (committed['npc/m/k'] == nil) and (committed['npc/m/other'].code == 'next')
        )###");
        require(f.isTrue("undone90") && f.isTrue("back90"), "an undo puts its key into the pending table of a state that came after the write");
    }

    void testContextCommitWithSwitch()
    {
        QuestFixture f;

        // another thread switches: the state that ends commits its items with the new state, in one write
        f.drive(R"###(
            setQuestState{uid=91, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/k', {code = 'run'}) ]]}
            TEST.pendingBefore91 = TEST.db[91].fld_context == nil
            setQuestState{uid=91, state='b'}
            TEST.commit91 = (TEST.db[91].fld_context['npc/m/k'].code == 'run') and (TEST.lastFields_91.fld_states == true) and (TEST.lastFields_91.fld_context == true)
        )###");
        require(f.isTrue("pendingBefore91"), "an install by the state runner is written before its switch");
        require(f.isTrue("commit91") && f.inState(91, "SYS_QSTFSM", "'b'"), "a switch by another thread doesn't write the items of the state that ends with the new state");

        // the state runner switches itself
        f.drive(R"###(
            setQuestState{uid=92, state='run', args=[[
                local uid = ...
                _RSVD_NAME_questContext.install(uid, 'npc/m/k', {code = 'self'})
                setQuestState{uid=uid, state='b'}
            ]]}
            TEST.commit92 = TEST.db[92].fld_context['npc/m/k'].code == 'self'
        )###");
        require(f.isTrue("commit92") && f.inState(92, "SYS_QSTFSM", "'b'"), "a switch by the state runner doesn't write its items with the new state");

        // a switch of another fsm commits only its own items
        f.drive(R"###(
            setQuestState{uid=93, state='run', args=[[ _RSVD_NAME_questContext.install(..., 'npc/m/main', {code = 'main'}) ]]}
            setQuestState{uid=93, fsm='sub', state='s1'}
            TEST.sub93 = TEST.db[93].fld_context == nil
            setQuestState{uid=93, state='b'}
            TEST.main93 = TEST.db[93].fld_context['npc/m/main'].code == 'main'
        )###");
        require(f.isTrue("sub93") && f.isTrue("main93"), "a switch of one fsm commits the items of another");
    }

    void testQuestDoneWritesRowFirst()
    {
        QuestFixture f;

        // an item type whose removal records what it saw
        require(f.runner.execRawString(R"###(
            _RSVD_NAME_questContext.types.test = {remove = function(uid, key, record)
                TEST['removed_' .. uid] = (TEST['removed_' .. uid] or '') .. key .. '=' .. record.item.code .. ','
                TEST['doneAtRemove_' .. uid] = dbGetQuestState(uid, SYS_QSTFSM) == SYS_DONE
            end}
        )###").valid(), "failed to add an item type");

        f.drive(R"###(
            setQuestState{uid=94, state='b'}
            _RSVD_NAME_questContext.install(94, 'test/k', {type = 'test', code = 'a'})
            TEST.db[94].fld_gridtriggers = {{'slowMap'}}
        )###");

        // the old grid trigger record makes quest done wait in loadBaseMap(), after its row and the removals of its items
        const auto kp = f.runner.spawn(f.driverKey++, std::string("setQuestState{uid=94, state=SYS_DONE} TEST.done94 = true"));
        require(f.runner.hasKeyPair(kp) && f.isNil("done94"), "quest done doesn't wait in its remote call");
        require(f.inState(94, "SYS_QSTFSM", "SYS_DONE"), "quest done doesn't write its row before it removes items from the world");

        f.drive("TEST.removed94 = (TEST.removed_94 == 'test/k=a,') and TEST.doneAtRemove_94 and (_RSVD_NAME_questContext.get(94, 'test/k') == nil) and (TEST.db[94].fld_context == nil)");
        require(f.isTrue("removed94"), "quest done doesn't remove the items of the context after its row, or keeps them");

        f.drive("local ok, err = pcall(_RSVD_NAME_questContext.install, 94, 'test/k2', {type = 'test', code = 'b'}) TEST.lateRaised94 = (not ok) and (string.find(err, 'its quest is done', 1, true) ~= nil)");
        require(f.isTrue("lateRaised94"), "an install while quest done removes items doesn't raise");

        require(f.runner.execRawString("TEST.mapLoaded = true").valid(), "failed to load the map");
        f.runner.resume(kp);
        require(!f.runner.hasKeyPair(kp) && f.isTrue("done94"), "quest done doesn't finish");
    }

    void testNPCBehaviorContext()
    {
        QuestFixture f;
        f.stubNPCs();
        f.drive("TEST.npcs['m/n'] = 9100");

        // a quest not started: in the world and in runtime, never saved
        f.drive(R"###(
            setupNPCQuestBehavior('m', 'n', 100, 'code100')
            TEST.ok100 = (TEST.world[9100][100] == 'code100') and (TEST.db[100] == nil) and (_RSVD_NAME_questContext.get(100, 'npc/m/n').target == 9100)
        )###");
        require(f.isTrue("ok100"), "an NPC behavior for a quest not started isn't installed, or is saved");

        // the state runner: saved with its next switch, with what installs it again
        f.drive(R"###(
            setQuestState{uid=101, state='run', args=[[ setupNPCQuestBehavior('m', 'n', ..., 'return 7', 'code101') ]]}
            TEST.pending101 = (TEST.world[9100][101] == 'code101') and (TEST.db[101].fld_context == nil)

            setQuestState{uid=101, state='b'}
            local item = TEST.db[101].fld_context['npc/m/n']
            TEST.saved101 = (item.type == 'npc') and (item.map == 'm') and (item.npc == 'n') and (item.argstr == 'return 7') and (item.code == 'code101') and (item.hash == 'hash1')
        )###");
        require(f.isTrue("pending101") && f.isTrue("saved101"), "an NPC behavior of the state runner isn't installed, or isn't saved with its next switch");

        // another thread: saved at once, a clear removes it from the world and the database
        f.drive(R"###(
            setQuestState{uid=102, state='b'}
            setupNPCQuestBehavior('m', 'n', 102, 'code102')
            TEST.saved102 = (TEST.db[102].fld_context['npc/m/n'].code == 'code102') and (TEST.world[9100][102] == 'code102')

            clearNPCQuestBehavior('m', 'n', 102)
            TEST.cleared102 = (TEST.db[102].fld_context == nil) and (TEST.world[9100][102] == nil)
        )###");
        require(f.isTrue("saved102") && f.isTrue("cleared102"), "an NPC behavior of another thread isn't saved at once, or its clear doesn't remove it");

        // a quest done: an install raises before anything is sent, a clear sends nothing
        f.drive(R"###(
            setQuestState{uid=103, state=SYS_DONE}
            local calls = #TEST.calls
            local ok, err = pcall(setupNPCQuestBehavior, 'm', 'n', 103, 'code103')
            clearNPCQuestBehavior('m', 'n', 103)
            TEST.done103 = (not ok) and (string.find(err, 'its quest is done', 1, true) ~= nil) and (#TEST.calls == calls)
        )###");
        require(f.isTrue("done103"), "an NPC behavior for a quest done is sent, or doesn't raise");
    }

    void testNPCBehaviorRefused()
    {
        QuestFixture f;
        f.stubNPCs();

        // the NPC refuses the new behavior, its record is undone, the old one is what the NPC still has
        f.drive(R"###(
            TEST.npcs['m/n'] = 9100
            setQuestState{uid=104, state='b'}
            setupNPCQuestBehavior('m', 'n', 104, 'old')

            TEST.refuse = true
            local ok, err = pcall(setupNPCQuestBehavior, 'm', 'n', 104, 'new')
            TEST.refuse = false

            local record = _RSVD_NAME_questContext.get(104, 'npc/m/n')
            TEST.refused104 = (not ok) and (string.find(err, 'refused by the npc', 1, true) ~= nil)
                and (record.item.code == 'old') and (TEST.db[104].fld_context['npc/m/n'].code == 'old') and (TEST.world[9100][104] == 'old')
        )###");
        require(f.isTrue("refused104"), "an NPC behavior the NPC refuses doesn't raise, or its record isn't undone");
    }

    void testNPCBehaviorTimelines()
    {
        QuestFixture f;
        f.stubNPCs();
        f.drive("TEST.npcs['m/n'] = 9100");

        // T1: another thread switches while the state runner waits for the reply of its install, the switch saves the item
        f.drive(R"###(
            TEST.holdReplies = true
            setQuestState{uid=105, state='run', args=[[ setupNPCQuestBehavior('m', 'n', ..., 'code105') TEST.after105 = true ]]}

            TEST.holdReplies = false
            setQuestState{uid=105, state='b'}
            TEST.t1 = TEST.db[105].fld_context['npc/m/n'].code == 'code105'
        )###");
        require(f.isTrue("t1") && f.isNil("after105"), "T1: a switch while the state runner waits for its install doesn't save the item, or the runner goes on");

        // T3: quest done while an install by another thread waits for its reply, quest done removes the item
        f.drive("setQuestState{uid=106, state='b'} TEST.holdReplies = true");
        const auto kp = f.runner.spawn(f.driverKey++, std::string("setupNPCQuestBehavior('m', 'n', 106, 'code106') TEST.installed106 = true"));
        require(f.runner.hasKeyPair(kp), "an install doesn't wait for its reply");

        f.drive("TEST.installCall106 = #TEST.calls TEST.holdReplies = false setQuestState{uid=106, state=SYS_DONE}");
        f.reply(f.get("installCall106").as<int>());
        require(!f.runner.hasKeyPair(kp) && f.isTrue("installed106"), "an install doesn't finish after its reply");

        f.drive("TEST.t3 = (TEST.world[9100][106] == nil) and (TEST.db[106].fld_context == nil) and (_RSVD_NAME_questContext.get(106, 'npc/m/n') == nil)");
        require(f.isTrue("t3"), "T3: an item installed while quest done runs stays");

        // 比奇商会.lua: the NPC callback clears the dialog the sub fsm installed, then ends the sub fsm, the dialog doesn't come back
        f.drive(R"###(
            setQuestState{uid=107, state='b'}
            setQuestState{uid=107, fsm='sub', state='run', args=[[ setupNPCQuestBehavior('m', 'n', ..., 'questions') ]]}

            clearNPCQuestBehavior('m', 'n', 107)
            setQuestState{uid=107, fsm='sub', state=SYS_DONE}
            TEST.owner107 = (TEST.db[107].fld_context == nil) and (TEST.world[9100][107] == nil)
        )###");
        require(f.isTrue("owner107"), "an NPC behavior cleared by another thread comes back with the commit of the fsm that installed it");
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
        testNoStateSwitchInRestore();
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
        testRestoreReadsEachFSM();
        testClosedDuringSwitch();
        testCloseHandlerSwitchesOtherUID();
        testMapGridTriggerQuest();
        testContextWriters();
        testContextOwner();
        testContextCommit();
        testContextRollback();
        testContextUndo();
        testContextCommitWithSwitch();
        testQuestDoneWritesRowFirst();
        testNPCBehaviorContext();
        testNPCBehaviorRefused();
        testNPCBehaviorTimelines();
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
        std::printf("Quest state runner passed: go to next state, state changed by other thread, set state of other uid, synchronous chain, quest done closes all FSMs, sub FSM sets quest done, restore, finished state, runner closed by a thread it starts, no state switch while closing, self close checked before any change, fallback of setQuestState() and stateWithFallback(), fallback of a remote error, runtime vars, switch from a given state, old state closed before the new one starts, state switches in a cycle with no yield stop, no state switch or restore while quest done runs, restore reads each fsm again, a caller closed by its switch ends, a <close> handler switches another uid, setupMapGridTrigger() installs a trigger of its quest, the writers, owners, commit, rollback and undo of the quest context, its commit with a switch, quest done writing its row first, and NPC behaviors as context items, refused ones, and the timelines T1 and T3.\n");

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
