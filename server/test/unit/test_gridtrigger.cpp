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

    class TestMapObject final: public ServerObject
    {
        public:
            TestMapObject()
                : ServerObject(uidf::getBaseMapUID(1, 0))
            {}

        public:
            corof::awaitable<> onActorMsg(const ActorMsgPack &) override
            {
                return {};
            }
    };

    constexpr static unsigned char mapScript []
    {
        #embed "../../src/servermap.lua" suffix(,)
        '\0'
    };

    // stands for the C++ bindings of ServerMap::LuaThreadRunner that the grid triggers use
    // TEST.grids['x,y'] holds the trigger ids of a grid in install order, as MapGrid::triggerList does
    constexpr const char *mapBindings = R"###(
        TEST = {grids = {}, rects = {}, ran = {}, switched = {}}

        local lastID = 0
        local function eachGrid(rectList, func)
            for _, rect in ipairs(rectList) do
                for dx = 0, rect.w - 1 do
                    for dy = 0, rect.h - 1 do
                        func((rect.x + dx) .. ',' .. (rect.y + dy))
                    end
                end
            end
        end

        function _RSVD_NAME_allocateGridTriggerId(rectList)
            lastID = lastID + 1
            TEST.rects[lastID] = rectList
            eachGrid(rectList, function(grid)
                TEST.grids[grid] = TEST.grids[grid] or {}
                table.insert(TEST.grids[grid], lastID)
            end)
            return lastID
        end

        function _RSVD_NAME_removeGridTriggerId(id)
            local rectList = TEST.rects[id]
            if not rectList then
                return
            end

            TEST.rects[id] = nil
            eachGrid(rectList, function(grid)
                for i, gridID in ipairs(TEST.grids[grid]) do
                    if gridID == id then
                        table.remove(TEST.grids[grid], i)
                        break
                    end
                end
            end)
        end

        function _RSVD_NAME_getGridTriggerIDList(x, y)
            local idList = {}
            for i, id in ipairs(TEST.grids[x .. ',' .. y] or {}) do
                idList[i] = id
            end
            return idList
        end

        function uidGridMapSwitch(uid, x, y)
            table.insert(TEST.switched, uid)
            return true
        end

        -- a handler that records its name when it runs
        function TEST.handler(name, result)
            return function(uid, x, y)
                table.insert(TEST.ran, name)
                return result
            end
        end

        function TEST.same(a, b)
            if #a ~= #b then
                return false
            end

            for i = 1, #a do
                if a[i] ~= b[i] then
                    return false
                end
            end
            return true
        end
    )###";

    struct MapFixture
    {
        TestMapObject so;

        // the pod is never attached to an actor pool, and ~ActorPod() detaches from g_actorPool, which this test doesn't have
        // so the pod is left alive on purpose
        ServerLuaCoroutineRunner runner{new ActorPod(&so)};

        MapFixture()
        {
            require(runner.execRawString(mapBindings).valid(), "failed to setup map bindings");
            require(runner.execRawString(to_rawcstr(mapScript)).valid(), "failed to load servermap.lua");
        }

        void run(const std::string &code)
        {
            require(runner.execRawString(code.c_str()).valid(), "failed to run lua code");
        }

        bool check(const std::string &expr)
        {
            run("TEST.result = " + expr);
            const sol::object obj = runner.getState()["TEST"]["result"];
            return obj.is<bool>() && obj.as<bool>();
        }
    };

    void testRegistration()
    {
        MapFixture f;
        f.run(R"###(
            TEST.map   = addGridTrigger(10, 10, TEST.handler('map', false))
            TEST.quest = addQuestGridTrigger('questA', {{x = 9, y = 10, w = 2, h = 1}}, TEST.handler('questA', false))
            TEST.uid   = addUIDGridTrigger(7, 'questA', 10, 10, TEST.handler('uid7', false))
        )###");

        require(f.check("TEST.same(getGridTriggerIDList(10, 10), {TEST.map, TEST.quest, TEST.uid}) and TEST.same(getGridTriggerIDList(9, 10), {TEST.quest})"), "triggers don't cover their grids in install order");

        require(f.check("getGridTriggerInfo(TEST.map).type == SYS_EPDEF and getGridTriggerInfo(TEST.map).uid == nil and getGridTriggerInfo(TEST.map).quest == nil"), "trigger of the map script doesn't record its type, or records a player or a quest");
        require(f.check("getGridTriggerInfo(TEST.quest).type == SYS_EPDEF and getGridTriggerInfo(TEST.quest).uid == nil and getGridTriggerInfo(TEST.quest).quest == 'questA'"), "all-player trigger of a quest doesn't record its quest");
        require(f.check("getGridTriggerInfo(TEST.uid).type == SYS_EPUID and getGridTriggerInfo(TEST.uid).uid == 7 and getGridTriggerInfo(TEST.uid).quest == 'questA'"), "per-player trigger doesn't record its player and quest");
        require(f.check("getGridTriggerInfo(12345) == nil"), "a trigger that doesn't exist has info");
    }

    void testDelete()
    {
        MapFixture f;
        f.run(R"###(
            TEST.map  = addGridTrigger(10, 10, TEST.handler('map', false))
            TEST.uid1 = addUIDGridTrigger(7, 'questA', 10, 10, TEST.handler('uid7a', false))
            TEST.uid2 = addUIDGridTrigger(7, 'questA', 11, 10, TEST.handler('uid7b', false))

            deleteGridTrigger(TEST.map)
            deleteGridTrigger(TEST.uid1)
            deleteGridTrigger(TEST.uid1)
            deleteGridTrigger(12345)
        )###");

        require(f.check("getGridTriggerInfo(TEST.map) == nil and getGridTriggerInfo(TEST.uid1) == nil"), "deleted trigger keeps its record");
        require(f.check("TEST.same(getGridTriggerIDList(10, 10), {})"), "deleted trigger stays on its grid");
        require(f.check("getGridTriggerInfo(TEST.uid2) ~= nil and TEST.same(getGridTriggerIDList(11, 10), {TEST.uid2})"), "delete removes another trigger of the same player and quest");

        f.run("_RSVD_NAME_clearQuestUIDGridTrigger(7, 'questA')");
        require(f.check("getGridTriggerInfo(TEST.uid2) == nil and TEST.same(getGridTriggerIDList(11, 10), {})"), "per-quest clear after a delete misses a trigger");
    }

    void testQuestClear()
    {
        MapFixture f;
        f.run(R"###(
            TEST.questA = addQuestGridTrigger('questA', 10, 10, TEST.handler('questA', false))
            TEST.uid7A  = addUIDGridTrigger(7, 'questA', 10, 10, TEST.handler('uid7A', false))
            TEST.uid7A2 = addUIDGridTrigger(7, 'questA', 12, 10, TEST.handler('uid7A2', false))
            TEST.uid7B  = addUIDGridTrigger(7, 'questB', 10, 10, TEST.handler('uid7B', false))
            TEST.uid8A  = addUIDGridTrigger(8, 'questA', 10, 10, TEST.handler('uid8A', false))

            _RSVD_NAME_clearQuestUIDGridTrigger(7, 'questA')
        )###");

        require(f.check("getGridTriggerInfo(TEST.uid7A) == nil and getGridTriggerInfo(TEST.uid7A2) == nil and TEST.same(getGridTriggerIDList(12, 10), {})"), "per-quest clear doesn't remove every per-player trigger of the player in the quest");
        require(f.check("TEST.same(getGridTriggerIDList(10, 10), {TEST.questA, TEST.uid7B, TEST.uid8A})"), "per-quest clear removes a trigger of another quest, of another player, or an all-player one");

        f.run("_RSVD_NAME_clearQuestUIDGridTrigger(7, 'questA')");
        require(f.check("TEST.same(getGridTriggerIDList(10, 10), {TEST.questA, TEST.uid7B, TEST.uid8A})"), "second per-quest clear changes something");
    }

    void testDispatch()
    {
        // the first per-player trigger of the player, of any quest, else the first all-player one, else let through
        MapFixture f;
        f.run(R"###(
            addQuestGridTrigger('questA', 10, 10, TEST.handler('questA', true))
            addGridTrigger(10, 10, TEST.handler('map', false))
            addUIDGridTrigger(7, 'questB', 10, 10, TEST.handler('uid7B', false))
            addUIDGridTrigger(7, 'questA', 10, 10, TEST.handler('uid7A', true))
            addUIDGridTrigger(9, 'questC', 11, 11, TEST.handler('uid9C', false))

            _RSVD_NAME_runGridTrigger(7, 10, 10)
            TEST.ran7, TEST.switched7, TEST.ran, TEST.switched = TEST.ran, TEST.switched, {}, {}

            _RSVD_NAME_runGridTrigger(8, 10, 10)
            TEST.ran8, TEST.switched8, TEST.ran, TEST.switched = TEST.ran, TEST.switched, {}, {}

            _RSVD_NAME_runGridTrigger(7, 11, 11)
        )###");

        require(f.check("TEST.same(TEST.ran7, {'uid7B'}) and TEST.same(TEST.switched7, {})"), "dispatch doesn't run the first per-player trigger of the player alone");
        require(f.check("TEST.same(TEST.ran8, {'questA'}) and TEST.same(TEST.switched8, {8})"), "dispatch doesn't run the first all-player trigger alone, or doesn't switch when it returns true");
        require(f.check("TEST.same(TEST.ran, {}) and TEST.same(TEST.switched, {7})"), "dispatch doesn't let a player through a grid with only another player's trigger");
    }

    void runTests()
    {
        testRegistration();
        testDelete();
        testQuestClear();
        testDispatch();
    }
}

int main()
{
    try{
        char arg0[] = "gridtrigger_test";
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
        Log log("mir2x-gridtrigger-test");
        std::cout.rdbuf(savedCoutBuf);
        g_mir2xLog = &log;

        Server server;
        g_server = &server;

        runTests();
        std::printf("Grid trigger passed: registration with type, player and quest, delete of any kind, per-quest clear, and dispatch.\n");

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
