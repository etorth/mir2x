#include <memory>
#include <iterator>
#include <tuple>
#include "stdf.hpp"
#include "luaf.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "serdesmsg.hpp"
#include "server.hpp"
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"

extern Server *g_server;

LuaCoopResumer::LuaCoopResumer(ServerLuaCoroutineRunner *luaRunner, void *currRunner, sol::function callback, sol::this_state s)
    : m_luaRunner(luaRunner)
    , m_currRunner(currRunner)
    , m_luaThread([s]
      {
          lua_State * const L = s;
          lua_pushthread(L);
          sol::main_reference threadRef(L, -1);

          lua_pop(L, 1);
          return threadRef;
      }())
    , m_callback(std::move(callback))
{
    fflassert(m_luaRunner);
    fflassert(m_currRunner);
}

LuaCoopResumer::LuaCoopResumer(const LuaCoopResumer & resumer)
    : m_luaRunner(resumer.m_luaRunner)
    , m_currRunner(resumer.m_currRunner)
    , m_luaThread(resumer.m_luaThread)
    , m_callback(resumer.m_callback)
{}

LuaCoopResumer::LuaCoopResumer(LuaCoopResumer && resumer)
    : m_luaRunner(resumer.m_luaRunner)
    , m_currRunner(resumer.m_currRunner)
    , m_luaThread(std::move(resumer.m_luaThread))
    , m_callback(std::move(resumer.m_callback))
{}

void LuaCoopResumer::pushOnClose(std::function<void()> fnOnClose) const
{
    static_cast<ServerLuaCoroutineRunner::LuaThreadHandle *>(m_currRunner)->onClose.push(std::move(fnOnClose));
}

void LuaCoopResumer::popOnClose() const
{
    static_cast<ServerLuaCoroutineRunner::LuaThreadHandle *>(m_currRunner)->onClose.pop();
}

void LuaCoopResumer::resumeYieldedRunner(ServerLuaCoroutineRunner *luaRunner, void *handle)
{
    if(auto currRunner = static_cast<ServerLuaCoroutineRunner::LuaThreadHandle *>(handle); currRunner->needResume){
        luaRunner->resumeRunner(static_cast<ServerLuaCoroutineRunner::LuaThreadHandle *>(currRunner));
    }
}

ServerLuaCoroutineRunner::ServerLuaCoroutineRunner(ActorPod *podPtr)
    : ServerLuaModule()
    , m_actorPod([podPtr]()
      {
          fflassert(podPtr); return podPtr;
      }())
{
    m_actorPod->registerOp(AM_SENDNOTIFY, [thisptr = this](this auto, const ActorMsgPack &mpk) -> corof::awaitable<>
    {
        auto sdSN = mpk.deserialize<SDSendNotify>();
        if(mpk.seqID() && !sdSN.waitConsume){
            thisptr->m_actorPod->post(mpk.fromAddr(), AM_OK);
        }

        thisptr->addNotify(sdSN.key, sdSN.seqID, std::move(sdSN.varList));

        if(mpk.seqID() && sdSN.waitConsume){
            thisptr->m_actorPod->post(mpk.fromAddr(), AM_OK);
        }
        return {};
    });

    bindFunction("getUID", [this]() -> uint64_t
    {
        return m_actorPod->UID();
    });

    bindFunction("getThreadKey", [this]() -> uint64_t
    {
        return m_currRunner->key;
    });

    bindFunction("getThreadSeqID", [this]() -> uint64_t
    {
        return m_currRunner->seqID;
    });

    bindFunction("getKeyPair", [this]() -> std::pair<uint64_t, uint64_t>
    {
        return m_currRunner->keyPair();
    });

    // don't return std::array<uint64_t, 3> here
    // sol2 doesn't create an array, but an object with metatable, can not be serialized
    bindFunction("getThreadAddress", [this]()
    {
        return sol::as_table<std::array<uint64_t, 3>>({m_actorPod->UID(), m_currRunner->key, m_currRunner->seqID});
    });

    // backend of closeThread() in serverluacoroutinerunner.lua, seqID 0 closes every thread under key
    // selfClose: the calling thread is one of them, it only gets closeRequested, the lua wrapper yields to end it
    bindFunction("_RSVD_NAME_closeThread", [this](uint64_t key, uint64_t seqID, sol::this_state s) -> std::tuple<bool, bool>
    {
        const bool selfClose = m_currRunner && m_currRunner->key == key && (seqID == 0 || m_currRunner->seqID == seqID);
        if(selfClose){
            if(const auto reason = selfCloseError(s.lua_state())){
                throw fflpanic("thread {}:{} closing itself {}", to_llu(m_currRunner->key), to_llu(m_currRunner->seqID), reason);
            }
        }

        const bool found = hasKey(key, seqID);
        if(found){
            close(key, seqID);
        }
        return {found, selfClose};
    });

    // backend of runThread() in serverluacoroutinerunner.lua
    // closed: the new thread asked to close the calling thread, i.e. switched its quest state, the lua wrapper yields to end it
    bindFunction("_RSVD_NAME_runThread", [this](uint64_t key, sol::function func, sol::this_state s) -> std::tuple<uint64_t, bool>
    {
        checkThreadDepth();
        const auto [newKey, newSeqID] = runThread(key, func);

        // closing: the caller is a <close> handler of the calling thread, the ongoing close ends it
        if(m_currRunner && m_currRunner->closeRequested && !m_currRunner->closing){
            if(const auto reason = selfCloseError(s.lua_state())){
                throw fflpanic("thread {}:{} closed during runThread() of thread {}:{}, but it can't end there, runThread() is called {}", to_llu(m_currRunner->key), to_llu(m_currRunner->seqID), to_llu(newKey), to_llu(newSeqID), reason);
            }
            return {newSeqID, true};
        }
        return {newSeqID, false};
    });

    // backend of closeThreadThenRun() in serverluacoroutinerunner.lua
    // the calling thread only gets closeRequested here, resumeRunner() closes it at its yield, then starts func
    bindFunction("_RSVD_NAME_closeThreadThenRun", [this](uint64_t key, sol::main_function func, sol::this_state s)
    {
        if(!m_currRunner){
            throw fflpanic("closeThreadThenRun() called outside any thread");
        }

        if(const auto reason = selfCloseError(s.lua_state())){
            throw fflpanic("thread {}:{} closing itself {}", to_llu(m_currRunner->key), to_llu(m_currRunner->seqID), reason);
        }

        // func starts in the resumeRunner() of the calling thread, on top of the same threads as a runThread() here
        checkThreadDepth();

        fflassert(!m_currRunner->afterClose, m_currRunner->keyPair());
        m_currRunner->afterClose = std::make_pair(key, std::move(func));
        closeRunner(m_currRunner);
    });

    // lets setQuestState() check, before it changes anything, that it can start the new state runner
    bindFunction("_RSVD_NAME_checkThreadDepth", [this]()
    {
        checkThreadDepth();
    });

    // true while lua_closethread() runs <close> handlers, setQuestState() refuses to run in them
    bindFunction("_RSVD_NAME_hasClosingThread", [this]() -> bool
    {
        return m_closingRunner != nullptr;
    });

    // lets setQuestState() check, before it changes anything, that the calling state runner can close itself at the end
    bindFunction("_RSVD_NAME_selfCloseError", [this](sol::this_state s) -> sol::object
    {
        const sol::state_view sv(s);
        if(!m_currRunner){
            return sol::make_object(sv, "outside any thread");
        }

        if(const auto reason = selfCloseError(s.lua_state())){
            return sol::make_object(sv, reason);
        }
        return sol::make_object(sv, sol::lua_nil);
    });

    bindCoop("_RSVD_NAME_remoteCall", [thisptr = this](this auto, LuaCoopResumer onDone, LuaCoopState s, uint64_t uid, std::string code, sol::object args) -> corof::awaitable<>
    {
        fflassert(uid != thisptr->m_actorPod->UID());

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto mpk = co_await thisptr->m_actorPod->send(uid, {AM_REMOTECALL, cerealf::serialize(SDRemoteCall
        {
            .code = code,
            .args = luaf::buildLuaVar(args),
        })});

        if(closed){
            // even thread is closed, we still exam the remote call result to detect error
            // but will not resume the thread anymore since it's already closed
        }
        else{
            onDone.popOnClose();
        }

        switch(mpk.type()){
            case AM_SDBUFFER:
                {
                    auto sdRCR = mpk.template deserialize<SDRemoteCallResult>();
                    if(sdRCR.error.empty()){
                        if(!closed){
                            std::vector<sol::object> resList;
                            for(auto & var: sdRCR.varList){
                                resList.emplace_back(luaf::buildLuaObj(s.getView(), std::move(var)));
                            }
                            onDone(SYS_EXECDONE, sol::as_args(resList));
                        }
                    }
                    else{
                        // the remote code raised, the error goes back to the calling lua code as a lua error, see uidRemoteCall()
                        // a throw here would leave the actor thread and stop the whole server
                        //
                        // logged here as well, the remote side doesn't log it, and the calling code may catch it
                        fflassert(sdRCR.varList.empty(), sdRCR.error, sdRCR.varList);
                        g_server->addLog(LOGTYPE_WARNING, "Error detected in remote call: %s", concatCode(code).c_str());
                        for(const auto &line: sdRCR.error){
                            g_server->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                        }

                        if(!closed){
                            onDone(SYS_EXECERROR, str_join(sdRCR.error, "\n"));
                        }
                    }
                    break;
                }
            case AM_BADACTORPOD:
                {
                    if(!closed){
                        onDone(SYS_EXECBADUID);
                    }
                    break;
                }
            default:
                {
                    throw fflpanic("lua call failed in {}: {}", to_cstr(uidf::getUIDString(uid)), mpk.str());
                }
        }
    });

    bindCoop("_RSVD_NAME_setMonsterDropOnDie", [thisptr = this](this auto, LuaCoopResumer onDone, uint64_t monsterUID, uint64_t playerUID, bool allowDefaultDrop, sol::table itemCfgList) -> corof::awaitable<>
    {
        fflassert(uidf::isMonster(monsterUID), monsterUID);
        if(playerUID){
            fflassert(uidf::isPlayer(playerUID), playerUID);
        }

        if(!itemCfgList.empty()){
            fflassert(luaf::isArray(itemCfgList), itemCfgList);
        }

        const auto fnGetItem = [](const sol::object &obj) -> SDItem
        {
            if(obj.is<lua_Integer>()){
                return SDItem::buildItemList(check_cast<uint32_t>(obj.as<lua_Integer>()), 1).front();
            }

            if(obj.is<std::string>()){
                return SDItem::buildItemList(DBCOM_ITEMID(obj.as<std::string>().c_str()), 1).front();
            }

            return SDItem::fromLuaVar(luaf::buildLuaVar(obj));
        };

        SDDropOnDie sdDropOnDie;
        sdDropOnDie.playerUID = playerUID;
        sdDropOnDie.allowDefaultDrop = allowDefaultDrop;

        for(size_t i = 1; i <= itemCfgList.size(); ++i){
            const sol::object entry = itemCfgList[i];
            fflassert(entry.is<sol::table>(), i);

            const sol::table cfg = entry.as<sol::table>();
            sdDropOnDie.itemList.push_back(SDDropItemOdds
            {
                .item = fnGetItem(cfg["item"]),
                .odds = cfg["odds"].get_or<size_t>(1),
            });
        }

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto mpk = co_await thisptr->m_actorPod->send(monsterUID, {AM_SETDROPONDIE, cerealf::serialize(sdDropOnDie)});
        if(closed){
            co_return;
        }

        onDone.popOnClose();
        switch(mpk.type()){
            case AM_TRUE:
                {
                    onDone(true);
                    break;
                }
            case AM_FALSE:
            case AM_BADACTORPOD:
                {
                    onDone(false);
                    break;
                }
            default:
                {
                    throw fflvalue(uidf::getUIDString(monsterUID), itemCfgList, mpk.str());
                }
        }
    });

    bindFunction("_RSVD_NAME_postNotify", [this](uint64_t dstUID, uint64_t dstThreadKey, uint64_t dstThreadSeqID, sol::object args) -> bool
    {
        fflassert(uidf::validUID(dstUID));
        fflassert(dstThreadKey > 0);

        return m_actorPod->post(dstUID, {AM_SENDNOTIFY, cerealf::serialize(SDSendNotify
        {
            .key = dstThreadKey,
            .seqID = dstThreadSeqID,
            .varList = luaf::buildLuaVar(args),
            .waitConsume = false,
        })});
    });

    bindCoop("_RSVD_NAME_sendNotify", [thisptr = this](this auto, LuaCoopResumer onDone, uint64_t dstUID, uint64_t dstThreadKey, uint64_t dstThreadSeqID, sol::object args) -> corof::awaitable<>
    {
        fflassert(uidf::validUID(dstUID));
        fflassert(dstThreadKey > 0);

        bool closed = false;
        onDone.pushOnClose([&closed](){ closed = true; });

        const auto mpk = co_await thisptr->m_actorPod->send(dstUID, {AM_SENDNOTIFY, cerealf::serialize(SDSendNotify
        {
            .key = dstThreadKey,
            .seqID = dstThreadSeqID,
            .varList = luaf::buildLuaVar(args),
            .waitConsume = false,
        })});

        if(closed){
            co_return;
        }

        onDone.popOnClose();
        switch(mpk.type()){
            case AM_OK:
                {
                    onDone(SYS_EXECDONE);
                    break;
                }
            default:
                {
                    onDone();
                    break;
                }
        }
    });

    bindFunction("_RSVD_NAME_pickNotify", [this](uint64_t maxCount, sol::this_state s)
    {
        if(maxCount == 0 || maxCount >= m_currRunner->notifyList.size()){
            return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::move(m_currRunner->notifyList)));
        }
        else{
            auto begin = m_currRunner->notifyList.begin();
            auto end   = m_currRunner->notifyList.begin() + maxCount;

            std::deque<luaf::luaVar> vers(std::make_move_iterator(begin), std::make_move_iterator(end));
            m_currRunner->notifyList.erase(begin, end);

            return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::move(vers)));
        }
    });

    bindFunction("_RSVD_NAME_waitNotify", [this](uint64_t timeout, sol::this_state s) -> sol::object
    {
        if(!m_currRunner->notifyList.empty()){
            auto firstVar = std::move(m_currRunner->notifyList.front());
            m_currRunner->notifyList.pop_front();

            m_currRunner->needNotify = false;
            return luaf::buildLuaObj(sol::state_view(s), std::move(firstVar));
        }

        m_currRunner->needNotify = true;
        if(timeout > 0){
            const auto kp = m_currRunner->keyPair();
            const auto timer = std::make_shared<std::pair<uint64_t, uint64_t>>();

            // the timer can fire after a notify has ended the wait, its call can even come after the thread waits again
            *timer = addTimer(timeout, [kp, timer, this](bool fired)
            {
                if(fired){
                    if(auto runnerPtr = hasKeyPair(kp); runnerPtr && runnerPtr->notifyTimer == *timer){
                        resumeNotifyWaiter(runnerPtr);
                    }
                }
            });

            m_currRunner->notifyTimer = *timer;
            m_currRunner->onClose.push([timer = *timer, this]()
            {
                cancelTimer(timer);
            });
        }
        return sol::make_object(sol::state_view(s), sol::lua_nil);
    });

    bindFunction("clearNotify", [this]()
    {
        m_currRunner->notifyList.clear();
    });

    bindYielding("_RSVD_NAME_pauseYielding", [this](uint64_t msec)
    {
        const auto kp = m_currRunner->keyPair();
        const auto timer = addTimer(msec, [kp, this](bool timeout)
        {
            if(timeout){
                if(auto runnerPtr = hasKeyPair(kp)){
                    // first pop the onclose function
                    // otherwise the resume() will call it if the resume reaches end of code
                    fflassert(!runnerPtr->onClose.empty());
                    runnerPtr->onClose.pop();
                    resumeRunner(runnerPtr);
                }
            }
        });

        m_currRunner->onClose.push([timer, this]()
        {
            cancelTimer(timer);
        });
    });

    constexpr static unsigned char luaScript []
    {
        #embed "serverluacoroutinerunner.lua" suffix(,)
        '\0'
    };
    pfrCheck(execRawString(to_rawcstr(luaScript)));
}

std::vector<uint64_t> ServerLuaCoroutineRunner::getSeqID(uint64_t key, std::vector<uint64_t> *seqIDListBuf) const
{
    if(auto eqr = m_runnerList.equal_range(key); eqr.first != eqr.second){
        std::vector<uint64_t> buf;
        std::vector<uint64_t> *result = seqIDListBuf ? seqIDListBuf : &buf;

        result->clear();
        result->reserve(std::distance(eqr.first, eqr.second));

        for(auto p = eqr.first; p != eqr.second; ++p){
            result->push_back(p->second.seqID);
        }
        return std::move(*result);
    }
    return {};
}

void ServerLuaCoroutineRunner::resume(uint64_t key, uint64_t seqID)
{
    if(auto p = hasKey(key, seqID)){
        // the caller of resume() doesn't know the state of the thread, only a suspended thread can be resumed
        //
        //     onStack: the thread is running, or it has ended and its resumeRunner() is still calling its onDone
        //              running : lua_resume() fails with "cannot resume non-suspended coroutine"
        //              raised  : lua_resume() fails with "cannot resume dead coroutine"
        //              finished: worse, its code runs again from the start
        //                        sol2 pushes the thread function before each lua_resume(), and lua_resume() takes a finished thread with a function on it as a new one
        //     closing: lua_closethread() is running its <close> handlers, it never continues
        //
        // in both cases the thread is not waiting for this resume, dropping it loses nothing
        // resumeRunner() asserts both flags are false, so they must be filtered here
        if(p->onStack || p->closing){
            return;
        }
        resumeRunner(p);
    }
    else{
        // won't throw here
        // if needs to confirm the coroutine exists, use hasKey() first
    }
}

ServerLuaCoroutineRunner::LuaThreadHandle *ServerLuaCoroutineRunner::hasKey(uint64_t key, uint64_t seqID)
{
    if(auto eqr = m_runnerList.equal_range(key); eqr.first != eqr.second){
        for(auto p = eqr.first; p != eqr.second; ++p){
            if(seqID == 0 || seqID == p->second.seqID){
                return std::addressof(p->second);
            }
        }
    }
    return nullptr;
}

void ServerLuaCoroutineRunner::close(uint64_t key, uint64_t seqID)
{
    // closing runs lua and C++ code that can spawn, resume or close threads under the same key
    // so work on a snapshot of seqIDs and look each thread up again, never keep an iterator of m_runnerList across a close
    const auto seqIDList = seqID ? std::vector<uint64_t>{seqID} : getSeqID(key);
    for(const auto id: seqIDList){
        if(auto runnerPtr = hasKey(key, id)){
            closeRunner(runnerPtr);
        }
    }
}

void ServerLuaCoroutineRunner::addNotify(uint64_t key, uint64_t seqID, luaf::luaVar var)
{
    if(auto runnerPtr = hasKey(key, seqID)){
        runnerPtr->notifyList.push_back(std::move(var));
        if(runnerPtr->needNotify){
            resumeNotifyWaiter(runnerPtr);
        }
    }
}

void ServerLuaCoroutineRunner::resumeNotifyWaiter(LuaThreadHandle *runnerPtr)
{
    fflassert(runnerPtr);
    fflassert(runnerPtr->needNotify, runnerPtr->keyPair());

    // the other one of the two must not resume the thread again
    // the onClose entry of the timer is on top while the thread waits, nothing else runs on the thread to push one
    runnerPtr->needNotify = false;
    if(const auto timer = std::exchange(runnerPtr->notifyTimer, std::nullopt)){
        cancelTimer(*timer);
        runnerPtr->onClose.pop();
    }
    resumeRunner(runnerPtr);
}

std::pair<uint64_t, uint64_t> ServerLuaCoroutineRunner::runThread(uint64_t key, const sol::function &func)
{
    return spawn(key, func, [key, this](const sol::protected_function_result &pfr)
    {
        std::vector<std::string> error;
        if(pfrCheck(pfr, [&error](const std::string &errLine){ error.push_back(errLine); })){
            if(pfr.return_count() > 0){
                // drop quest state function result
            }
        }
        else{
            if(error.empty()){
                error.push_back(str_printf("unknown error for runThread: key %llu", to_llu(key)));
            }

            for(const auto &line: error){
                g_server->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
            }
        }
    });
}

void ServerLuaCoroutineRunner::checkThreadDepth() const
{
    // a level takes about 8KB of C stack for runThread() in a debug build, 64 of them fit the 1MB stack of a windows thread with room to spare
    constexpr int maxThreadDepth = 64;
    if(m_threadDepth >= maxThreadDepth){
        throw fflpanic("{} threads run on top of each other on the C stack, can't start one more, i.e. state switches in a cycle with no yield", m_threadDepth);
    }
}

std::pair<uint64_t, uint64_t> ServerLuaCoroutineRunner::addTimer(uint64_t msec, std::function<void(bool)> fnOnTimer)
{
    return m_actorPod->getSO()->addDelay(msec, std::move(fnOnTimer));
}

void ServerLuaCoroutineRunner::cancelTimer(const std::pair<uint64_t, uint64_t> &timer)
{
    m_actorPod->getSO()->cancelDelay(timer);
}

int ServerLuaCoroutineRunner::closeLuaThread(LuaThreadHandle *runnerPtr)
{
    // returns LUA_OK, or the status of the final error, which lua_closethread() leaves at index 1 of the thread's stack
    fflassert(runnerPtr);
    runnerPtr->closing = true;

    const stdf::ValueKeeper keepCurrRunner(m_currRunner, runnerPtr);
    const stdf::ValueKeeper keepClosingRunner(m_closingRunner, runnerPtr);
    return lua_closethread(runnerPtr->runner.thread_state(), nullptr);
}

const char *ServerLuaCoroutineRunner::selfCloseError(lua_State *callerState) const
{
    fflassert(m_currRunner);

    // a yield there goes back to the code that resumed the coroutine, not to resumeRunner()
    if(callerState != m_currRunner->runner.thread_state()){
        return "from a coroutine created in it";
    }

    if(!lua_isyieldable(callerState)){
        return "where it can't yield";
    }
    return nullptr;
}

void ServerLuaCoroutineRunner::closeRunner(LuaThreadHandle *runnerPtr)
{
    fflassert(runnerPtr);
    if(runnerPtr->closing){
        return;
    }

    // a thread with frames on the C stack can't be reset now, resumeRunner() closes it at its next yield
    if(runnerPtr->onStack){
        runnerPtr->closeRequested = true;
        return;
    }

    const auto kp = runnerPtr->keyPair();
    if(const auto status = closeLuaThread(runnerPtr); status != LUA_OK){
        const sol::protected_function_result errPfr(runnerPtr->runner.thread_state(), 1, 1, 1, static_cast<sol::call_status>(status));
        g_server->addLog(LOGTYPE_WARNING, "Error in <close> handler while closing runner: key %llu, seqID %llu", to_llu(kp.first), to_llu(kp.second));
        pfrCheck(errPfr, [](const std::string &s)
        {
            g_server->addLog(LOGTYPE_WARNING, "%s", to_cstr(s));
        });
    }
    eraseRunner(kp);
}

void ServerLuaCoroutineRunner::eraseRunner(const std::pair<uint64_t, uint64_t> &kp)
{
    // extract() first, the dtor runs onClose callbacks, which can spawn, resume or close threads, m_runnerList must be consistent by then
    const auto eqr = m_runnerList.equal_range(kp.first);
    for(auto p = eqr.first; p != eqr.second; ++p){
        if(p->second.seqID == kp.second){
            const auto node = m_runnerList.extract(p);
            return;
        }
    }
}

bool ServerLuaCoroutineRunner::doSpawn(std::pair<uint64_t, uint64_t> kp, const std::string &code, luaf::luaVar args, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    fflassert(kp.first, kp);
    fflassert(str_haschar(code));

    const auto p = m_runnerList.emplace(std::piecewise_construct, std::forward_as_tuple(kp.first), std::forward_as_tuple(*this, kp.first, kp.second, std::move(onDone), std::move(onClose)));
    return resumeRunner(std::addressof(p->second), std::make_pair(str_printf(
        R"###( do                                                          )###""\n"
        R"###(     _RSVD_NAME_startTime = getNanoTstamp()                  )###""\n"
        R"###(     local _RSVD_NAME_autoClear<close> = autoClearTLSTable() )###""\n"
        R"###(     do                                                      )###""\n"
        R"###(        %s                                                   )###""\n"
        R"###(     end                                                     )###""\n"
        R"###( end                                                         )###""\n", code.c_str()), std::move(args)));

    // don't use p after resumeRunner()
    // because resumeRunner() may erase p from m_runnerList
}

bool ServerLuaCoroutineRunner::doSpawn(std::pair<uint64_t, uint64_t> kp, const sol::function &func, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    fflassert(kp.first, kp);
    fflassert(func);

    // give the plain function the same scoped tls cleanup the string overload builds into its chunk
    // the wrapper runs in the coroutine, so its <close> fires as soon as the coroutine returns or throws

    const sol::function wrapper = getState()["_RSVD_NAME_luaCoroutineRunner_funcMain"];
    fflassert(wrapper);

    const sol::function wrappedFunc = wrapper(func);
    fflassert(wrappedFunc);

    const auto p = m_runnerList.emplace(std::piecewise_construct, std::forward_as_tuple(kp.first), std::forward_as_tuple(*this, kp.first, kp.second, wrappedFunc, std::move(onDone), std::move(onClose)));
    return resumeRunner(std::addressof(p->second));

    // don't use p after resumeRunner()
    // because resumeRunner() may erase p from m_runnerList
}

template<typename... Args> corof::awaitable<std::vector<luaf::luaVar>> ServerLuaCoroutineRunner::evalImpl(uint64_t key, Args && ... args)
{
    std::vector<luaf::luaVar> result {};
    std::vector<std::string>  errors {};
    std::coroutine_handle<>   handle {};

    const auto fnOnThreadDone = [&result, &errors, &handle](std::vector<std::string> argError, std::vector<luaf::luaVar> argVarList)
    {
        if(!argError.empty()){
            fflassert(argVarList.empty(), argError, argVarList);
        }

        // never throw exceptions here
        // this callback may be invoked by resumeRunner() while resuming an unrelated actor/query-response async stack
        // throwing outside evalImpl's coroutine surfaces the error in the wrong stack, permanently leaking this coroutine and its awaiter
        //
        // instead, we stash the error and always resume the suspended awaiting coroutine
        // evalImpl will safely throw the luaf::evalError itself after co_await

        errors = std::move(argError);
        result = std::move(argVarList);

        // if lua code execution can finish without suspend, then handle will not be set
        // because doSpawn returns true -> LuaEvalAwaitable::await_ready(), so await_suspend() will not be called and nothing need to be resumed

        if(handle){
            handle.resume();
        }
    };

    const auto closed = std::make_shared<bool>(true);
    const auto done = doSpawn({key, m_seqID++}, std::forward<Args>(args)..., [&fnOnThreadDone, closed, key, this](const sol::protected_function_result &pfr)
    {
        *closed = false;
        std::vector<std::string> errors;

        if(pfrCheck(pfr, [&errors](const std::string &s){ errors.push_back(s); })){
            fnOnThreadDone(std::move(errors), luaf::pfrBuildLuaVarList(pfr));
        }
        else{
            if(errors.empty()){
                errors.push_back(std::format("unknown error for runner: key {}", key));
            }
            fnOnThreadDone(std::move(errors), {});
        }
    },

    [&fnOnThreadDone, closed, this]()
    {
        if(*closed){
            fnOnThreadDone({}, {luaf::luaVar(SYS_EXECCLOSE)});
        }
    });

    co_await LuaEvalAwaitable
    {
        // done is what doSpawn() returns, i.e. what resumeRunner() returns after the first resume of the thread
        //
        //     true : thread is gone without suspension: it finished, raised, or got closed at its first yield, see closeRequested
        //            fnOnThreadDone has been called already, by onDone, or by onClose with SYS_EXECCLOSE
        //            result/errors are set, don't suspend, handle stays empty, fnOnThreadDone had nothing to resume
        //
        //     false: thread has yielded and is still alive
        //            suspend, fnOnThreadDone resumes this coroutine by handle once the thread is gone
        .ready = done,
        .handle = std::addressof(handle),
    };

    if(!errors.empty()){
        throw luaf::evalError(std::move(errors));
    }
    co_return result;
}

std::pair<uint64_t, uint64_t> ServerLuaCoroutineRunner::spawn(uint64_t key, std::pair<uint64_t, uint64_t> reqAddr, const std::string &code, luaf::luaVar args)
{
    fflassert(key);
    fflassert(str_haschar(code));

    fflassert(reqAddr.first , reqAddr);
    fflassert(reqAddr.second, reqAddr);

    auto closed = std::make_shared<bool>(true);
    return spawn(key, code, std::move(args), [closed, key, reqAddr, this](const sol::protected_function_result &pfr)
    {
        const auto fnOnThreadDone = [reqAddr, this](std::vector<std::string> error, std::vector<luaf::luaVar> varList)
        {
            if(!error.empty()){
                fflassert(varList.empty(), error, varList);
            }

            m_actorPod->post(reqAddr, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
            {
                .error = std::move(error),
                .varList = std::move(varList),
            })});
        };

        *closed = false;
        std::vector<std::string> error;
        if(pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
            // initial run succeeds and coroutine finished
            // simple cases like: uidRemoteCall(uid, [[ return getName() ]])
            //
            // for this case there is no need to uses coroutine
            // but we can not predict script from NPC is synchronized call or not
            //
            // for cases like spaceMove() we can use quasi-function from NPC side
            // but it prevents NPC side to execute commands like:
            //
            //     uidRemoteCall(uid, [[
            //         spaceMove(12, 22, 12) -- impossible if using quasi-function
            //         return getLevel()
            //     ]])
            //
            // for the quasi-function solution player side has no spaceMove() function avaiable
            // player side can only support like:
            //
            //     'SPACEMOVE 12, 22, 12'
            //
            // this is because spaveMove is a async operation, it needs callbacks
            // this limits the script for NPC side, put everything into lua coroutine is a solution for it, but with cost
            //
            // for how quasi-function was implemented
            // check commit: 30981cc539a05b41309330eaa04fbf3042c9d826
            //
            // starting/running an coroutine with a light function in Lua costs ~280 ns
            // alternatively call a light function directly takes ~30 ns, best try is every time let uidRemoteCall() do as much as possible
            //
            // light cases, no yield in script
            // directly return the result with save the runner
            fnOnThreadDone(std::move(error), luaf::pfrBuildLuaVarList(pfr));
        }
        else{
            if(error.empty()){
                error.push_back(str_printf("unknown error for runner: key %llu", to_llu(key)));
            }
            fnOnThreadDone(std::move(error), {});
        }
    },

    [closed, reqAddr, this]()
    {
        if(*closed){
            m_actorPod->post(reqAddr, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
            {
                .varList
                {
                    luaf::luaVar(SYS_EXECCLOSE),
                },
            })});
        }
    });
}

std::pair<uint64_t, uint64_t> ServerLuaCoroutineRunner::spawn(uint64_t key, const std::string &code, luaf::luaVar args, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    const auto currSeqID = m_seqID++;
    doSpawn({key, currSeqID}, code, std::move(args), std::move(onDone), std::move(onClose));
    return {key, currSeqID};
}

std::pair<uint64_t, uint64_t> ServerLuaCoroutineRunner::spawn(uint64_t key, const sol::function &func, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    const auto currSeqID = m_seqID++;
    doSpawn({key, currSeqID}, func, std::move(onDone), std::move(onClose));
    return {key, currSeqID};
}

corof::awaitable<std::vector<luaf::luaVar>> ServerLuaCoroutineRunner::eval(uint64_t key, const std::string &code, luaf::luaVar args)
{
    return evalImpl(key, code, std::move(args));
}

corof::awaitable<std::vector<luaf::luaVar>> ServerLuaCoroutineRunner::eval(uint64_t key, const sol::function &func)
{
    return evalImpl(key, func);
}

bool ServerLuaCoroutineRunner::resumeRunner(LuaThreadHandle *runnerPtr, std::optional<std::pair<std::string, luaf::luaVar>> codeOpt)
{
    // resumes the thread once
    // returns true if the thread is gone: it finished, raised, or got closed at its yield, runnerPtr is invalid then

    fflassert(runnerPtr);

    // a sol::coroutine converts to true only if it's new or its last call yielded, sol2 doesn't ask lua, see resume()
    fflassert(runnerPtr->callback);

    // resume() filters both out, other callers only resume a suspended thread
    fflassert(!runnerPtr->onStack, runnerPtr->keyPair());
    fflassert(!runnerPtr->closing, runnerPtr->keyPair());

    // the whole call, a thread started after the close below runs on top of this one too
    const stdf::ValueKeeper keepThreadDepth(m_threadDepth, m_threadDepth + 1);

    // here sol2 can tell if coroutine return nothing vs return nil
    //
    //    uidRemoteCall(uid, [[        func_return_nil() ]]) -- pfr in c++ is {   }, empty
    //    uidRemoteCall(uid, [[ return func_return_nil() ]]) -- pfr in c++ is {nil}, size-1-list
    //
    // but if in script we don't check
    //
    //    local result = uidRemoteCall(uid, [[        func_return_nil() ]]) -- result in lua is nil
    //    local result = uidRemoteCall(uid, [[ return func_return_nil() ]]) -- result in lua is nil, too
    //
    // this difference has been propogated to remote caller side by pfr serialization
    // this difference should be handled by caller side

    // onDone is not expected to throw
    // but if it does, the handle must still be erased to avoid leaking the LuaThreadHandle entry, then the exception is rethrown

    const auto fnNotifyDone = [runnerPtr, this](const sol::protected_function_result &pfr) -> std::exception_ptr
    {
        try{
            if(const auto onDoneFunc = std::move(runnerPtr->onDone)){
                // runnerPtr stays valid, the thread is onStack, closing it in onDone only sets closeRequested
                onDoneFunc(pfr);
            }
            else{
                std::vector<std::string> error;
                if(pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
                    if(pfr.return_count() > 0){
                        g_server->addLog(LOGTYPE_WARNING, "Dropped result: %s", to_cstr(str_any(luaf::pfrBuildLuaVarList(pfr))));
                    }
                }
                else{
                    if(error.empty()){
                        error.push_back(str_printf("unknown error for runner: key = %llu", to_llu(runnerPtr->key)));
                    }

                    for(const auto &line: error){
                        g_server->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                    }
                }
            }
        }
        catch(...){
            return std::current_exception();
        }
        return nullptr;
    };

    const auto kp = runnerPtr->keyPair();

    bool yielded = false;
    std::exception_ptr onDoneExcept;
    {
        // onStack covers the resume and the onDone call
        // declared first in this scope so it's reset last, after pfr and errPfr have popped their values from the thread's stack
        const stdf::ValueKeeper keepOnStack(runnerPtr->onStack, true);

        auto pfr = [&]()
        {
            const stdf::ValueKeeper keepCurrRunner(m_currRunner, runnerPtr);
            if(codeOpt.has_value()){
                return runnerPtr->callback(codeOpt.value().first, luaf::buildLuaObj(sol::state_view(runnerPtr->runner.state()), codeOpt.value().second));
            }
            else{
                return runnerPtr->callback();
            }
        }();

        if(runnerPtr->callback){
            yielded = true;
        }
        else if(pfr.valid()){
            onDoneExcept = fnNotifyDone(pfr);
        }
        else{
            // thread raised, its <close> handlers haven't run yet, run them before the owner gets the error
            // sol2 has added the traceback to the error, so the handlers get the same error as the owner
            lua_State * const co = runnerPtr->runner.thread_state();
            const auto errStatus = pfr.status();

            std::vector<std::string> errLines;
            pfrCheck(pfr, [&errLines](const std::string &s){ errLines.push_back(s); });

            // lua_closethread() resets the stack pfr refers to, without abandon() the pfr dtor would pop slots that no longer exist
            pfr.abandon();

            auto closeStatus = closeLuaThread(runnerPtr);
            if(closeStatus == LUA_OK){
                // lua_closethread() sees no error only if lua_resume() refused to run the thread, i.e. "C stack overflow"
                // can't happen here, kept as a guard so the owner still gets the error
                std::string errStr;
                for(const auto &line: errLines){
                    if(!errStr.empty()){
                        errStr += '\n';
                    }
                    errStr += line;
                }

                lua_pushlstring(co, errStr.data(), errStr.size());
                closeStatus = static_cast<int>(errStatus);
            }

            // the final error at index 1, a <close> handler that raised replaces the original one, same as coroutine.close()
            const sol::protected_function_result errPfr(co, 1, 1, 1, static_cast<sol::call_status>(closeStatus));

            std::vector<std::string> finalErrLines;
            pfrCheck(errPfr, [&finalErrLines](const std::string &s){ finalErrLines.push_back(s); });

            if(finalErrLines != errLines){
                g_server->addLog(LOGTYPE_WARNING, "Runner error replaced by error in <close> handler: key %llu, seqID %llu, original error:", to_llu(kp.first), to_llu(kp.second));
                for(const auto &line: errLines){
                    g_server->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                }
            }

            onDoneExcept = fnNotifyDone(errPfr);
        }
    }

    if(yielded){
        // asked to close while it ran, i.e. it closed itself, close it now that it's suspended
        if(runnerPtr->closeRequested){
            auto afterClose = std::exchange(runnerPtr->afterClose, std::nullopt);
            closeRunner(runnerPtr);

            // see closeThreadThenRun(), started after the <close> handlers of the closed thread, and not on top of its C stack
            if(afterClose){
                runThread(afterClose->first, sol::function(getState().lua_state(), sol::ref_index(afterClose->second.registry_index())));
            }
            return true;
        }
        return false;
    }

    eraseRunner(kp);
    if(onDoneExcept){
        std::rethrow_exception(onDoneExcept);
    }
    return true;
}
