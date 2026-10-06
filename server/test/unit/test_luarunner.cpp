#include <coroutine>
#include <cstdio>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <sol/sol.hpp>
#include "argf.hpp"
#include "corof.hpp"
#include "log.hpp"
#include "luaf.hpp"
#include "server.hpp"
#include "serverargparser.hpp"
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"
#include "sysconst.hpp"
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

    // runs the timers of pause() and waitNotify() by hand, this test has no actor pool to run them
    class ManualTimerRunner final: public ServerLuaCoroutineRunner
    {
        private:
            uint64_t m_lastTimer = 0;

        public:
            std::map<uint64_t, std::function<void(bool)>> timers; // waiting to fire, by id
            std::vector<std::function<void(bool)>> cancelled;    // cancelled, their call with false is still to come

        public:
            using ServerLuaCoroutineRunner::ServerLuaCoroutineRunner;
            using ServerLuaCoroutineRunner::addNotify;

        protected:
            std::pair<uint64_t, uint64_t> addTimer(uint64_t, std::function<void(bool)> fnOnTimer) override
            {
                timers.emplace(++m_lastTimer, std::move(fnOnTimer));
                return {m_lastTimer, m_lastTimer};
            }

            // as the delay driver does: a cancelled timer is called later with false, a fired one can't be cancelled anymore
            void cancelTimer(const std::pair<uint64_t, uint64_t> &timer) override
            {
                if(auto p = timers.find(timer.first); p != timers.end()){
                    cancelled.push_back(std::move(p->second));
                    timers.erase(p);
                }
            }

        public:
            // takes the timer out as firing does, its call with true comes when the caller makes it
            std::function<void(bool)> fire(uint64_t id)
            {
                const auto p = timers.find(id);
                require(p != timers.end(), "timer is not waiting to fire");

                auto fnOnTimer = std::move(p->second);
                timers.erase(p);
                return fnOnTimer;
            }
    };

    struct RunnerFixture
    {
        TestServerObject so;

        // the pod is never attached to an actor pool, and ~ActorPod() detaches from g_actorPool, which this test doesn't have
        // so the pod is left alive on purpose
        ManualTimerRunner runner{new ActorPod(&so)};

        RunnerFixture()
        {
            require(runner.execRawString("TEST = {}").valid(), "failed to create lua table TEST");
        }

        sol::object get(const char *name)
        {
            return runner.getState()["TEST"][name];
        }

        bool isNil(const char *name)
        {
            return get(name).get_type() == sol::type::lua_nil;
        }

        bool isTrue(const char *name)
        {
            const auto obj = get(name);
            return obj.is<bool>() && obj.as<bool>();
        }

        bool isString(const char *name, const std::string &value)
        {
            const auto obj = get(name);
            return obj.is<std::string>() && obj.as<std::string>() == value;
        }

        bool isInteger(const char *name, lua_Integer value)
        {
            const auto obj = get(name);
            return obj.is<lua_Integer>() && obj.as<lua_Integer>() == value;
        }

        bool hasPrefix(const char *name, const std::string &prefix)
        {
            const auto obj = get(name);
            return obj.is<std::string>() && obj.as<std::string>().starts_with(prefix);
        }
    };

    // Server::addLog() prints to std::cout in slave mode
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

    std::string errorString(const sol::protected_function_result &pfr)
    {
        const sol::error err = pfr;
        return err.what();
    }

    bool isExecClose(const std::vector<luaf::luaVar> &result)
    {
        if(result.size() != 1){
            return false;
        }

        const auto p = std::get_if<std::string>(&result[0]);
        return p && *p == SYS_EXECCLOSE;
    }

    corof::awaitable<> runEval(ServerLuaCoroutineRunner &runner, uint64_t key, std::string code, std::vector<luaf::luaVar> &result, bool &done)
    {
        result = co_await runner.eval(key, code, luaf::luaVar{});
        done = true;
    }

    void testYieldResumeFinish()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;
        std::string doneString;
        lua_Integer doneInteger = 0;

        const auto kp = f.runner.spawn(101, std::string(R"###(
            TEST.step = 1
            coroutine.yield()
            TEST.step = 2
            return 'finished', 42
        )###"), {}, [&](const sol::protected_function_result &pfr)
        {
            doneCount++;
            if(pfr.valid() && pfr.return_count() == 2){
                doneString = pfr.get<std::string>(0);
                doneInteger = pfr.get<lua_Integer>(1);
            }
        },
        [&closeCount]()
        {
            closeCount++;
        });

        require(f.runner.hasKeyPair(kp), "yielded runner is gone");
        require(f.isInteger("step", 1), "runner didn't run to its first yield");
        require(doneCount == 0 && closeCount == 0, "yielded runner reported as done");

        f.runner.resume(kp);
        require(!f.runner.hasKeyPair(kp), "finished runner is not erased");
        require(f.isInteger("step", 2), "runner didn't continue after resume");
        require(doneCount == 1 && doneString == "finished" && doneInteger == 42, "finished runner lost its results");
        require(closeCount == 1, "onClose of a finished runner is not called once");
    }

    void testCloseSuspended()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;
        bool handlerRanBeforeOnClose = false;

        const auto kp = f.runner.spawn(102, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.errType = type(err)
                TEST.key = getThreadKey()
                TEST.seqID = getThreadSeqID()
                TEST.tls = tlsValue
            end})

            tlsValue = 'tls'
            pause(SYS_POSINF)
            TEST.resumedAfterClose = true
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        [&]()
        {
            closeCount++;
            handlerRanBeforeOnClose = f.isString("errType", "nil");
        });

        require(f.runner.hasKeyPair(kp), "suspended runner is gone");
        f.runner.close(kp);

        require(!f.runner.hasKeyPair(kp), "closed runner is not erased");
        require(f.isString("errType", "nil"), "<close> handler of a suspended runner didn't get nil error");
        require(f.isInteger("key", 102) && f.isInteger("seqID", static_cast<lua_Integer>(kp.second)), "<close> handler doesn't run as the closed runner");
        require(f.isString("tls", "tls"), "<close> handler can't see thread local storage");
        require(f.isNil("resumedAfterClose"), "closed runner continued");
        require(closeCount == 1 && handlerRanBeforeOnClose, "onClose is not called once after the <close> handler");
        require(doneCount == 0, "onDone called for a closed runner");

        f.runner.resume(kp);
        f.runner.close(kp);
        require(closeCount == 1, "closed runner is closed again");
    }

    void testErrorClosesBeforeOnDone()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;
        std::string doneError;
        bool handlerRanBeforeOnDone = false;
        bool handleAliveInOnDone = false;

        f.runner.spawn(103, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.err = err
                TEST.key = getThreadKey()
            end})

            error('boom', 0)
        )###"), {}, [&](const sol::protected_function_result &pfr)
        {
            doneCount++;
            if(!pfr.valid()){
                doneError = errorString(pfr);
            }

            handlerRanBeforeOnDone = !doneError.empty() && f.isString("err", doneError);
            handleAliveInOnDone = f.runner.hasKey(103) != nullptr;
        },
        [&closeCount]()
        {
            closeCount++;
        });

        // sol2's default message handler adds the traceback before the thread unwinds, as xpcall(f, debug.traceback) does
        // so the <close> handler gets the same error as the owner
        require(!f.runner.hasKey(103), "raised runner is not erased");
        require(doneCount == 1 && doneError.starts_with("boom\nstack traceback:"), "onDone didn't get the runner error");
        require(handlerRanBeforeOnDone, "<close> handler of a raised runner didn't get the runner error before onDone");
        require(handleAliveInOnDone, "raised runner is erased before onDone");
        require(f.isInteger("key", 103), "<close> handler of a raised runner doesn't run as the runner");
        require(closeCount == 1, "onClose of a raised runner is not called once");
    }

    void testHandlerReplacesError()
    {
        RunnerFixture f;
        int doneCount = 0;
        std::string doneError;

        CoutCapture capture;
        f.runner.spawn(104, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.err = err
                error('second', 0)
            end})

            error('first', 0)
        )###"), {}, [&](const sol::protected_function_result &pfr)
        {
            doneCount++;
            if(!pfr.valid()){
                doneError = errorString(pfr);
            }
        },
        nullptr);

        require(!f.runner.hasKey(104), "raised runner is not erased");
        require(f.hasPrefix("err", "first\nstack traceback:"), "<close> handler didn't get the runner error");
        require(doneCount == 1 && doneError == "second", "onDone didn't get the error raised by the <close> handler");
        require(capture.has("Runner error replaced by error in <close> handler") && capture.has("original error:\nfirst\n"), "replaced runner error is not logged");
    }

    void testHandlerErrorWhileClosing()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;

        const auto kp = f.runner.spawn(105, std::string(R"###(
            local outer <close> = setmetatable({}, {__close = function(_, err)
                TEST.outerErr = err
            end})

            local inner <close> = setmetatable({}, {__close = function()
                error('closefail', 0)
            end})

            pause(SYS_POSINF)
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        [&closeCount]()
        {
            closeCount++;
        });

        CoutCapture capture;
        f.runner.close(kp);

        require(!f.runner.hasKeyPair(kp), "runner with a raising <close> handler is not erased");
        require(f.isString("outerErr", "closefail"), "outer <close> handler didn't get the error of the inner one");
        require(capture.has("Error in <close> handler while closing runner: key 105") && capture.has("closefail\n"), "error in <close> handler is not logged");
        require(closeCount == 1 && doneCount == 0, "closing a runner with a raising <close> handler notifies wrongly");
    }

    void testDeferredClose()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;

        const auto kp = f.runner.spawn(106, std::string(R"###(
            local myKey, mySeqID = getThreadKey(), getThreadSeqID()
            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.closeCount = (TEST.closeCount or 0) + 1
                TEST.errType = type(err)
            end})

            runThread(1061, function()
                TEST.closeRet = closeThread(myKey, mySeqID)
            end)

            TEST.afterRunThread = true
            pause(SYS_POSINF)
            TEST.afterPause = true
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        [&closeCount]()
        {
            closeCount++;
        });

        require(f.isTrue("closeRet"), "closeThread() didn't find the running thread");
        require(f.isNil("afterRunThread"), "runner continued after runThread() of a thread closing it");
        require(f.isNil("afterPause"), "runner continued after its deferred close");
        require(f.isInteger("closeCount", 1) && f.isString("errType", "nil"), "<close> handler didn't run once on deferred close");
        require(!f.runner.hasKeyPair(kp) && !f.runner.hasKey(1061), "runners are not erased");
        require(closeCount == 1 && doneCount == 0, "deferred close notifies wrongly");

        f.runner.spawn(1063, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function()
                closeThread(TEST.finishKey, TEST.finishSeqID)
            end})
            pause(SYS_POSINF)
        )###"), {}, nullptr, nullptr);

        std::string doneString;
        f.runner.spawn(1062, std::string(R"###(
            TEST.finishKey, TEST.finishSeqID = getThreadKey(), getThreadSeqID()
            local guard <close> = setmetatable({}, {__close = function()
                TEST.finishCloseCount = (TEST.finishCloseCount or 0) + 1
            end})

            TEST.finishCloseRet = closeThread(1063)
            return 'finished anyway'
        )###"), {}, [&doneString](const sol::protected_function_result &pfr)
        {
            if(pfr.valid() && pfr.return_count() == 1){
                doneString = pfr.get<std::string>(0);
            }
        },
        nullptr);

        require(f.isTrue("finishCloseRet"), "closeThread() didn't find the suspended thread");
        require(doneString == "finished anyway", "runner with a pending close request lost its result");
        require(f.isInteger("finishCloseCount", 1), "<close> handler didn't run once");
        require(!f.runner.hasKey(1062) && !f.runner.hasKey(1063), "runners are not erased");

        int stuckCloseCount = 0;
        const auto kpStuck = f.runner.spawn(1064, std::string(R"###(
            local myKey, mySeqID = getThreadKey(), getThreadSeqID()

            local okSort, errSort = pcall(table.sort, {2, 1}, function(a, b)
                runThread(1065, function()
                    closeThread(myKey, mySeqID)
                end)
                TEST.afterSortRunThread = true
                return a < b
            end)

            local okWrap, errWrap = pcall(coroutine.wrap(function()
                runThread(1066, function()
                    closeThread(myKey, mySeqID)
                end)
                TEST.afterWrapRunThread = true
            end))

            TEST.sortRaised = not okSort and string.find(errSort, "closed during runThread() of thread 1065:", 1, true) ~= nil and string.find(errSort, "runThread() is called where it can't yield", 1, true) ~= nil
            TEST.wrapRaised = not okWrap and string.find(errWrap, "closed during runThread() of thread 1066:", 1, true) ~= nil and string.find(errWrap, "runThread() is called from a coroutine created in it", 1, true) ~= nil

            coroutine.yield()
            TEST.afterStuckYield = true
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        [&stuckCloseCount]()
        {
            stuckCloseCount++;
        });

        require(f.isNil("afterSortRunThread") && f.isNil("afterWrapRunThread"), "code after runThread() ran in a closed runner that can't end");
        require(f.isTrue("sortRaised"), "runThread() in a table.sort() comparator of a closed runner doesn't raise");
        require(f.isTrue("wrapRaised"), "runThread() in a coroutine created in a closed runner doesn't raise");
        require(f.isNil("afterStuckYield"), "closed runner continued after its next yield");
        require(!f.runner.hasKeyPair(kpStuck) && !f.runner.hasKey(1065) && !f.runner.hasKey(1066), "runners are not erased");
        require(stuckCloseCount == 1 && doneCount == 0, "closed runner that can't end at runThread() notifies wrongly");
    }

    void testCloseInOnDone()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;

        const auto fnOnDone = [&](uint64_t key)
        {
            return [&f, &doneCount, key](const sol::protected_function_result &)
            {
                doneCount++;
                f.runner.resume(key);
                f.runner.close(key);
            };
        };

        const auto fnOnClose = [&closeCount]()
        {
            closeCount++;
        };

        f.runner.spawn(107, std::string("return 1"), {}, fnOnDone(107), fnOnClose);
        require(!f.runner.hasKey(107), "finished runner closed in its onDone is not erased");
        require(doneCount == 1 && closeCount == 1, "finished runner closed in its onDone notifies wrongly");

        f.runner.spawn(1071, std::string("error('closed in onDone', 0)"), {}, fnOnDone(1071), fnOnClose);
        require(!f.runner.hasKey(1071), "raised runner closed in its onDone is not erased");
        require(doneCount == 2 && closeCount == 2, "raised runner closed in its onDone notifies wrongly");
    }

    void testEval()
    {
        RunnerFixture f;

        // closed during its first run, eval() must not wait for it
        {
            bool done = false;
            std::vector<luaf::luaVar> result;

            runEval(f.runner, 108, R"###(
                local myKey, mySeqID = getThreadKey(), getThreadSeqID()
                runThread(1081, function()
                    closeThread(myKey, mySeqID)
                end)

                coroutine.yield()
                return 'unreachable'
            )###", result, done).resume();

            require(done, "eval() hangs on a runner closed during its first run");
            require(isExecClose(result), "eval() of a closed runner doesn't return SYS_EXECCLOSE");
            require(!f.runner.hasKey(108) && !f.runner.hasKey(1081), "eval runners are not erased");
        }

        // suspended and then resumed
        {
            bool done = false;
            std::vector<luaf::luaVar> result;

            runEval(f.runner, 1082, R"###(
                coroutine.yield()
                return 7
            )###", result, done).resume();

            require(!done, "eval() returns before its runner finishes");
            f.runner.resume(1082);

            const auto p = result.size() == 1 ? std::get_if<lua_Integer>(&result[0]) : nullptr;
            require(done && p && *p == 7, "eval() lost the result of a resumed runner");
        }

        // suspended and then closed
        {
            bool done = false;
            std::vector<luaf::luaVar> result;

            runEval(f.runner, 1083, R"###(
                pause(SYS_POSINF)
            )###", result, done).resume();

            require(!done, "eval() returns before its runner finishes");
            f.runner.close(1083);
            require(done && isExecClose(result), "eval() of a closed runner doesn't return SYS_EXECCLOSE");
        }
    }

    void testCloseByKey()
    {
        RunnerFixture f;

        const auto kpB = f.runner.spawn(109, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function()
                TEST.closeCountB = (TEST.closeCountB or 0) + 1
            end})
            pause(SYS_POSINF)
        )###"), {}, nullptr, nullptr);

        f.runner.getState()["TEST"]["seqIDB"] = kpB.second;

        // closes B and starts a new thread under the same key while close(109) is going through the key
        const auto kpA = f.runner.spawn(109, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function()
                TEST.closeCountA = (TEST.closeCountA or 0) + 1
                closeThread(109, TEST.seqIDB)
                runThread(109, function()
                    TEST.seqIDC = getThreadSeqID()
                    pause(SYS_POSINF)
                end)
            end})
            pause(SYS_POSINF)
        )###"), {}, nullptr, nullptr);

        f.runner.close(109);

        require(!f.runner.hasKeyPair(kpA) && !f.runner.hasKeyPair(kpB), "close(key) leaves runners");
        require(f.isInteger("closeCountA", 1) && f.isInteger("closeCountB", 1), "close(key) doesn't run each <close> handler once");

        const auto seqIDC = f.get("seqIDC");
        require(seqIDC.is<lua_Integer>() && f.runner.hasKey(109, static_cast<uint64_t>(seqIDC.as<lua_Integer>())), "runner started while closing is lost");

        f.runner.close(109);
        require(!f.runner.hasKey(109), "close(key) leaves runners");
    }

    void testSelfClose()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;

        const auto fnOnDone = [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        };

        const auto fnOnClose = [&closeCount]()
        {
            closeCount++;
        };

        // never returns, neither to the code after it nor to its callers
        const auto kp = f.runner.spawn(111, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.closeCount = (TEST.closeCount or 0) + 1
                TEST.errType = type(err)
            end})

            local function exitThread()
                closeThread(getThreadKey(), getThreadSeqID())
                TEST.afterClose = true
            end

            exitThread()
            TEST.afterCaller = true
        )###"), {}, fnOnDone, fnOnClose);

        require(f.isNil("afterClose") && f.isNil("afterCaller"), "closeThread() on itself returned");
        require(f.isInteger("closeCount", 1) && f.isString("errType", "nil"), "<close> handler didn't run once on self close");
        require(!f.runner.hasKeyPair(kp), "self closed runner is not erased");
        require(doneCount == 0 && closeCount == 1, "self close notifies wrongly");

        // inside pcall(), the close unwinds through it
        f.runner.spawn(1111, std::string(R"###(
            pcall(function()
                local inner <close> = setmetatable({}, {__close = function()
                    TEST.pcallCloseCount = (TEST.pcallCloseCount or 0) + 1
                end})
                closeThread(getThreadKey())
            end)
            TEST.afterPcall = true
        )###"), {}, fnOnDone, fnOnClose);

        require(f.isInteger("pcallCloseCount", 1) && f.isNil("afterPcall"), "self close inside pcall() didn't end the runner");
        require(!f.runner.hasKey(1111) && doneCount == 0 && closeCount == 2, "runner closing itself inside pcall() is not closed");

        // by its key, other threads under the key are closed on the spot, the caller at its yield
        f.runner.spawn(1112, std::string(R"###(
            local guard <close> = setmetatable({}, {__close = function()
                TEST.otherCloseCount = (TEST.otherCloseCount or 0) + 1
            end})
            pause(SYS_POSINF)
        )###"), {}, fnOnDone, fnOnClose);

        f.runner.spawn(1112, std::string(R"###(
            closeThread(getThreadKey())
            TEST.afterKeyClose = true
        )###"), {}, fnOnDone, fnOnClose);

        require(f.isInteger("otherCloseCount", 1) && f.isNil("afterKeyClose"), "closeThread() on its own key didn't close all threads under it");
        require(!f.runner.hasKey(1112) && doneCount == 0 && closeCount == 4, "threads under the key of the caller are not closed");

        // started by another thread, only the started one ends, the starting one goes on as itself
        f.runner.spawn(1113, std::string(R"###(
            runThread(1114, function()
                closeThread(getThreadKey(), getThreadSeqID())
                TEST.afterNestedClose = true
            end)
            TEST.outerKey = getThreadKey()
        )###"), {}, fnOnDone, fnOnClose);

        require(f.isNil("afterNestedClose") && f.isInteger("outerKey", 1113), "self close of a started runner affects the runner starting it");
        require(!f.runner.hasKey(1113) && !f.runner.hasKey(1114) && doneCount == 1 && closeCount == 5, "started runner closing itself is not closed");

        // in a <close> handler run by a return, the handler can yield, the return values are dropped
        f.runner.spawn(1115, std::string(R"###(
            local outer <close> = setmetatable({}, {__close = function(_, err)
                TEST.outerErrType = type(err)
            end})

            local inner <close> = setmetatable({}, {__close = function()
                closeThread(getThreadKey(), getThreadSeqID())
                TEST.afterHandlerClose = true
            end})

            return 'dropped'
        )###"), {}, fnOnDone, fnOnClose);

        require(f.isString("outerErrType", "nil") && f.isNil("afterHandlerClose"), "self close in a <close> handler of a returning runner didn't end it");
        require(!f.runner.hasKey(1115) && doneCount == 1 && closeCount == 6, "runner closing itself in a <close> handler is not closed");

        // eval() of a runner closing itself
        {
            bool done = false;
            std::vector<luaf::luaVar> result;

            runEval(f.runner, 1116, R"###(
                closeThread(getThreadKey(), getThreadSeqID())
                return 'unreachable'
            )###", result, done).resume();

            require(done && isExecClose(result), "eval() of a runner closing itself doesn't return SYS_EXECCLOSE");
            require(!f.runner.hasKey(1116), "eval runner closing itself is not erased");
        }

        // called outside any thread, it closes the thread and returns
        const auto kpMain = f.runner.spawn(1117, std::string("pause(SYS_POSINF)"), {}, nullptr, nullptr);
        require(f.runner.execRawString("TEST.mainRet = closeThread(1117)").valid() && f.isTrue("mainRet"), "closeThread() outside any thread fails");
        require(!f.runner.hasKeyPair(kpMain), "closeThread() outside any thread doesn't close the thread");
    }

    void testSelfCloseWhereCanNotYield()
    {
        RunnerFixture f;
        int doneCount = 0;
        int closeCount = 0;

        const auto fnOnClose = [&closeCount]()
        {
            closeCount++;
        };

        // raises before anything is closed, the thread and the other thread under its key go on
        const auto kpOther = f.runner.spawn(112, std::string("pause(SYS_POSINF)"), {}, nullptr, nullptr);
        const auto kp = f.runner.spawn(112, std::string(R"###(
            local myKey, mySeqID = getThreadKey(), getThreadSeqID()

            local okSort, errSort = pcall(table.sort, {2, 1}, function(a, b)
                closeThread(myKey)
                return a < b
            end)

            local okWrap, errWrap = pcall(coroutine.wrap(function()
                closeThread(myKey, mySeqID)
            end))

            TEST.sortRaised = not okSort and string.find(errSort, "closing itself where it can't yield", 1, true) ~= nil
            TEST.wrapRaised = not okWrap and string.find(errWrap, "closing itself from a coroutine created in it", 1, true) ~= nil

            coroutine.yield()
            TEST.resumed = true
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        fnOnClose);

        require(f.isTrue("sortRaised"), "self close in a table.sort() comparator doesn't raise");
        require(f.isTrue("wrapRaised"), "self close in a coroutine created in the thread doesn't raise");
        require(f.runner.hasKeyPair(kp) && f.runner.hasKeyPair(kpOther), "raising self close has closed threads");

        f.runner.resume(kp);
        require(f.isTrue("resumed") && !f.runner.hasKeyPair(kp), "runner doesn't go on after a raising self close");
        require(doneCount == 1 && closeCount == 1, "runner notifies wrongly after a raising self close");

        f.runner.close(kpOther);
        require(!f.runner.hasKey(112), "close(key) leaves runners");

        // in a <close> handler run while the thread is closed
        f.runner.spawn(1121, std::string(R"###(
            local myKey, mySeqID = getThreadKey(), getThreadSeqID()
            local guard <close> = setmetatable({}, {__close = function()
                local ok, err = pcall(closeThread, myKey, mySeqID)
                TEST.handlerRaised = not ok and string.find(err, "closing itself where it can't yield", 1, true) ~= nil
            end})
            pause(SYS_POSINF)
        )###"), {}, nullptr, fnOnClose);

        f.runner.close(1121);
        require(f.isTrue("handlerRaised"), "self close in a <close> handler of a closed runner doesn't raise");
        require(!f.runner.hasKey(1121) && closeCount == 2, "runner raising self close in its <close> handler is not closed");
    }

    // a coop awaits this to stay pending, the test resumes it by hand, like a reply arriving from another actor
    struct PendingCoop
    {
        std::coroutine_handle<> handle;

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> h) noexcept
        {
            handle = h;
        }

        void await_resume() const noexcept {}

        void reply()
        {
            require(static_cast<bool>(handle), "no coop is pending");
            std::exchange(handle, nullptr).resume();
        }
    };

    // runs two full GC cycles, then tells if the thread registered in TEST[weakName] still exists
    bool threadAlive(RunnerFixture &f, const char *weakName)
    {
        const auto code = std::string("collectgarbage('collect') collectgarbage('collect') TEST.alive = next(TEST.") + weakName + ") ~= nil";
        require(f.runner.execRawString(code.c_str()).valid(), "failed to run full GC");
        return f.isTrue("alive");
    }

    void testCloseWhileCoopPending()
    {
        RunnerFixture f;
        PendingCoop pending;
        bool closedSeen = false;
        int doneCount = 0;
        int closeCount = 0;

        f.runner.bindCoop("_RSVD_NAME_testCoop", [&pending, &closedSeen](this auto, LuaCoopResumer onDone, sol::object value) -> corof::awaitable<>
        {
            bool closed = false;
            onDone.pushOnClose([&closed](){ closed = true; });

            co_await pending;

            if(closed){
                closedSeen = true;
                co_return;
            }

            onDone.popOnClose();
            onDone(value);
        });

        // control: a closed thread with no coop pending has nothing left to keep it, the GC frees it
        f.runner.spawn(1130, std::string(R"###(
            TEST.weakControl = setmetatable({}, {__mode = 'k'})
            TEST.weakControl[coroutine.running()] = true
            pause(SYS_POSINF)
        )###"));

        f.runner.close(1130);
        require(!threadAlive(f, "weakControl"), "closed thread is never collected, the test can't tell if a pending coop keeps its thread");

        const auto kp = f.runner.spawn(1131, std::string(R"###(
            TEST.weak = setmetatable({}, {__mode = 'k'})
            TEST.weak[coroutine.running()] = true

            TEST.result = _RSVD_NAME_callFuncCoop('testCoop', 'first')

            local guard <close> = setmetatable({}, {__close = function(_, err)
                TEST.errType = type(err)
            end})

            _RSVD_NAME_callFuncCoop('testCoop', 'second')
            TEST.resumedAfterClose = true
        )###"), {}, [&doneCount](const sol::protected_function_result &)
        {
            doneCount++;
        },
        [&closeCount]()
        {
            closeCount++;
        });

        require(f.runner.hasKeyPair(kp) && f.isNil("result"), "runner doesn't wait for its coop");

        pending.reply();
        require(f.isString("result", "first"), "coop reply doesn't get back to the runner");
        require(f.runner.hasKeyPair(kp) && static_cast<bool>(pending.handle), "runner doesn't wait for its second coop");

        f.runner.close(kp);
        require(!f.runner.hasKeyPair(kp), "closed runner is not erased");
        require(f.isString("errType", "nil") && closeCount == 1 && doneCount == 0, "runner closed while its coop is pending is not closed as usual");
        require(threadAlive(f, "weak"), "closed thread is collected while its coop is pending");

        pending.reply();
        require(closedSeen, "pending coop doesn't see its thread closed");
        require(f.isNil("resumedAfterClose"), "closed runner continued after its coop finished");
        require(!threadAlive(f, "weak"), "closed thread is never collected after its coop finished");
    }

    void testRemoteCallError()
    {
        RunnerFixture f;
        PendingCoop pending;
        const std::string remoteError = "remote side raised\nstack traceback:\n\t[C]: in function 'error'";

        // stands for _RSVD_NAME_remoteCall getting the reply of a remote call whose code raised
        f.runner.bindCoop("_RSVD_NAME_remoteCall", [&pending, &remoteError](this auto, LuaCoopResumer onDone, uint64_t, std::string, sol::object) -> corof::awaitable<>
        {
            bool closed = false;
            onDone.pushOnClose([&closed](){ closed = true; });

            co_await pending;
            if(closed){
                co_return;
            }

            onDone.popOnClose();
            onDone(SYS_EXECERROR, remoteError);
        });

        const auto remoteUID = std::to_string(uidf::getQuestUID(2));
        f.runner.spawn(1140, std::string(R"###(
            local ok, err = pcall(uidRemoteCall, )###") + remoteUID + R"###(, [[ error('remote side raised') ]])
            TEST.caughtOK = ok
            TEST.caughtErr = err
        )###");

        require(f.runner.hasKey(1140), "runner doesn't wait for its remote call");
        pending.reply();
        require(!f.runner.hasKey(1140), "runner doesn't finish after catching a remote error");
        require(f.get("caughtOK").is<bool>() && !f.isTrue("caughtOK"), "remote error doesn't raise in the calling runner");

        const auto caughtErr = f.get("caughtErr");
        require(caughtErr.is<std::string>() && caughtErr.as<std::string>().find("Remote call to QST_2 failed: remote side raised\nstack traceback:") != std::string::npos, "remote error doesn't carry the error of the remote code");

        int doneCount = 0;
        std::string doneError;
        f.runner.spawn(1141, std::string(R"###(
            uidRemoteCall()###") + remoteUID + R"###(, [[ error('remote side raised') ]])
            TEST.afterRemoteCall = true
        )###", {}, [&](const sol::protected_function_result &pfr)
        {
            doneCount++;
            if(!pfr.valid()){
                doneError = errorString(pfr);
            }
        });

        pending.reply();
        require(!f.runner.hasKey(1141) && f.isNil("afterRemoteCall"), "runner goes on after an uncaught remote error");
        require(doneCount == 1 && doneError.find("Remote call to QST_2 failed: remote side raised") != std::string::npos, "uncaught remote error doesn't reach onDone as the runner error");
    }

    void testWaitNotifyTimeout()
    {
        RunnerFixture f;

        // a failed check leaves threads waiting on timers, close them while the runner can still cancel the timers by hand
        // the base runner's teardown would cancel them by its own cancelTimer(), which needs an actor pool
        const auto closeLeft = stdf::guard([&f]()
        {
            for(const auto key: {1150, 1151, 1152, 1153}){
                f.runner.close(key);
            }
        });

        require(f.runner.execRawString("TEST.msg1 = table.pack('first') TEST.msg2 = table.pack('second')").valid(), "failed to create notify messages");
        const auto msg = [&f](const char *name){ return luaf::buildLuaVar(f.get(name)); };

        f.runner.spawn(1150, std::string(R"###(
            TEST.got = waitNotify(1000)
            pause(500)
            TEST.afterPause = true
        )###"));

        require(f.runner.timers.size() == 1, "waitNotify() with a timeout doesn't start a timer");
        f.runner.addNotify(1150, 0, msg("msg1"));
        require(f.isString("got", "first"), "notify doesn't end the wait");
        require(f.runner.timers.size() == 1 && f.runner.cancelled.size() == 1, "notify doesn't cancel the timer of the wait");

        for(const auto &fnOnTimer: std::exchange(f.runner.cancelled, {})){
            fnOnTimer(false);
        }
        require(f.runner.hasKey(1150) && f.isNil("afterPause"), "cancelled timer of a wait ends the pause after it");

        f.runner.fire(f.runner.timers.begin()->first)(true);
        require(!f.runner.hasKey(1150) && f.isTrue("afterPause"), "pause doesn't end at its timer");

        // the timer fires while a notify is on its way, its call comes after the notify, when the thread waits again
        f.runner.spawn(1151, std::string(R"###(
            TEST.first = waitNotify(1000)
            TEST.second = waitNotify(1000)
            TEST.afterSecond = true
        )###"));

        const auto stale = f.runner.fire(f.runner.timers.begin()->first);
        f.runner.addNotify(1151, 0, msg("msg1"));
        require(f.isString("first", "first") && f.runner.timers.size() == 1, "notify doesn't end the wait, or the thread doesn't wait again");

        stale(true);
        require(f.runner.hasKey(1151) && f.isNil("afterSecond"), "timer that fired before a notify ends the next wait");

        f.runner.addNotify(1151, 0, msg("msg2"));
        require(!f.runner.hasKey(1151) && f.isString("second", "second"), "notify doesn't end the next wait");

        f.runner.spawn(1152, std::string("TEST.timedOut = select('#', waitNotify(1000)) == 0"));
        f.runner.fire(f.runner.timers.begin()->first)(true);
        require(!f.runner.hasKey(1152) && f.isTrue("timedOut"), "waitNotify() doesn't end at its timeout");

        f.runner.spawn(1153, std::string("waitNotify(1000)"));
        f.runner.close(1153);
        require(!f.runner.hasKey(1153) && f.runner.timers.empty(), "closing a thread in waitNotify() doesn't cancel its timer");

        for(const auto &fnOnTimer: std::exchange(f.runner.cancelled, {})){
            fnOnTimer(false);
        }
    }

    void testTeardown()
    {
        bool handlerRan = false;
        int closeCount = 0;
        {
            RunnerFixture f;
            f.runner.getState().set_function("markHandlerRan", [&handlerRan]()
            {
                handlerRan = true;
            });

            f.runner.spawn(110, std::string(R"###(
                local guard <close> = setmetatable({}, {__close = function()
                    markHandlerRan()
                end})
                pause(SYS_POSINF)
            )###"), {}, nullptr, [&closeCount]()
            {
                closeCount++;
            });
        }

        require(!handlerRan, "runner teardown runs lua <close> handlers");
        require(closeCount == 1, "runner teardown doesn't call onClose once");
    }

    void runTests()
    {
        testYieldResumeFinish();
        testCloseSuspended();
        testErrorClosesBeforeOnDone();
        testHandlerReplacesError();
        testHandlerErrorWhileClosing();
        testDeferredClose();
        testCloseInOnDone();
        testEval();
        testCloseByKey();
        testSelfClose();
        testSelfCloseWhereCanNotYield();
        testCloseWhileCoopPending();
        testRemoteCallError();
        testWaitNotifyTimeout();
        testTeardown();
    }
}

int main()
{
    try{
        char arg0[] = "luarunner_test";
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
        Log log("mir2x-luarunner-test");
        std::cout.rdbuf(savedCoutBuf);
        g_mir2xLog = &log;

        Server server;
        g_server = &server;

        runTests();
        std::printf("Lua runner close passed: yield and resume, close while suspended, close on error, replaced error, raising close handler, deferred close, close in onDone, eval, close by key, self close, self close where it can't yield, close while a coop is pending, remote call error, notify and timeout of waitNotify(), and teardown.\n");

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
