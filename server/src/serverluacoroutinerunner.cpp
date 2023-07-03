#include <memory>
#include <iterator>
#include "luaf.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "serdesmsg.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"

extern MonoServer *g_monoServer;

struct LuaThreadHandle
{
    // for this class
    // the terms coroutine, runner, thread means same

    // scenario why adding seqID:
    // 1. received an event which triggers processNPCEvent(event)
    // 2. inside processNPCEvent(event) the script emits query to other actor
    // 3. when waiting for the response of the query, user clicked the close button or click init button to end up the current call stack
    // 4. receives the query response, we should ignore it
    //
    // to fix this we have to give every call stack an uniq seqID
    // and the query response needs to match the seqID

    const uint64_t key;
    const uint64_t seqID;

    // consume coroutine result
    // forward pfr to issuer as a special case
    std::function<void(const sol::protected_function_result &)> onDone;

    // thread can be closed when
    //
    //     1. it yields in C layer
    //     2. its control has been dropped and wait some callback to resume
    //
    // then if the thread is closed without calling the registered callback
    // we need some clear-functionality

    // thread can call back and forth in C/lua
    //
    // C -> lua -> C -> lua -> C -> lua
    //                         ^
    //                         |
    //                         +-- code of this layer is:
    //
    //                         bindYielding("_RSVD_NAME_pauseYielding", [](uint64_t time, uint64_t threadKey)
    //                         {
    //                             addDelay(time, [threadKey]()
    //                             {
    //                                 resume(threadKey);
    //                             });
    //                         });
    //
    // then even all C layer before this layer are not yield-able, still we know this chain may eventually get yielded
    // and each C layer may require to register a callback if closed before done, which requires a stack as
    //
    // C -> lua -> C -> lua -> C -> lua
    //                         ^
    //                         |
    //                         +-- code of this layer is:
    //
    //                         bindYielding("_RSVD_NAME_pauseYielding", [](uint64_t time, uint64_t threadKey)
    //                         {
    //                             const auto key = m_delayQueue.addDelay(time, [threadKey]()
    //                             {
    //                                 resume(threadKey);
    //                                 m_delayQueue.pop(); // no need to trigger if delayed command gets executed
    //                             });
    //
    //                             onClose.push([key]()
    //                             {
    //                                 m_delayQueue.erase(key);
    //                             })
    //                         });

    std::stack<std::function<void()>> onClose;

    sol::thread runner;
    sol::coroutine callback;

    bool needNotify = false;
    std::deque<luaf::luaVar> notifyList;

    LuaThreadHandle(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
        : key(argKey)
        , seqID(argSeqID)
        , onDone(std::move(argOnDone))
        , runner(sol::thread::create(argLuaModule.getLuaState().lua_state()))
        , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_main"])
    {
        fflassert(key);
        fflassert(seqID);

        if(argOnClose){
            onClose.push(std::move(argOnClose));
        }
    }
};

void LuaCoopResumer::pushOnClose(std::function<void()> fnOnClose) const
{
    m_luaRunner->pushOnClose(m_threadKey, m_threadSeqID, std::move(fnOnClose));
}

void LuaCoopResumer::popOnClose() const
{
    m_luaRunner->popOnClose(m_threadKey, m_threadSeqID);
}

void LuaCoopResumer::resumeRunner(ServerLuaCoroutineRunner *luaRunner, uint64_t threadKey, uint64_t threadSeqID)
{
    luaRunner->resume(threadKey, threadSeqID);
}

ServerLuaCoroutineRunner::ServerLuaCoroutineRunner(ActorPod *podPtr)
    : ServerLuaModule()
    , m_actorPod([podPtr]()
      {
          fflassert(podPtr); return podPtr;
      }())
{
    m_actorPod->registerOp(AM_SENDNOTIFY, [this](const ActorMsgPack &mpk)
    {
        auto sdSN = mpk.deserialize<SDSendNotify>();
        auto runnerPtr = hasKey(sdSN.key, sdSN.seqID);

        if(runnerPtr){
            // notify is from variadic_args, which may contain tailing nil
            // create a table that micmics format returned by table.pack() to preserve tailing nil

            auto tblVar = luaf::buildLuaVar(std::move(sdSN.varList));
            auto tblPtr = std::get_if<luaf::luaTable>(&tblVar);

            tblPtr->emplace(luaf::luaVarWrapper("n"), lua_Integer(tblPtr->size()));
            runnerPtr->notifyList.push_back(std::move(tblVar));
        }

        if(mpk.seqID()){
            if(sdSN.waitConsume){
                if(runnerPtr && runnerPtr->needNotify){
                    runnerPtr->needNotify = false;
                    resumeRunner(runnerPtr);
                }
                m_actorPod->forward(mpk.fromAddr(), AM_OK);
            }
            else{
                m_actorPod->forward(mpk.fromAddr(), AM_OK);
                if(runnerPtr && runnerPtr->needNotify){
                    runnerPtr->needNotify = false;
                    resumeRunner(runnerPtr);
                }
            }
        }
        else{
            if(runnerPtr && runnerPtr->needNotify){
                runnerPtr->needNotify = false;
                resumeRunner(runnerPtr);
            }
        }
    });

    bindFunction("getUID", [this]() -> uint64_t
    {
        return m_actorPod->UID();
    });

    bindFunction("getUIDString", [this]() -> std::string
    {
        return uidf::getUIDString(m_actorPod->UID());
    });

    bindFunctionCoop("_RSVD_NAME_uidExecute", [this](LuaCoopResumer onDone, LuaCoopState s, uint64_t uid, std::string code)
    {
        if(uid == m_actorPod->UID()){
            // run code locally in sandbox
            //
            //     setQuestHandler(questName,
            //     {
            //         [SYS_ENTER] = function(uid, value)
            //             ...
            //         end,
            //     })
            //
            //     uidExecute( getUID(), [[ runEventHandler(uid, {SYS_EPQST, questName}, SYS_ENTER) ]])
            //
            // this introduces dependecy between threads
            // this guarantees any gloval changes in qust handler [SYS_ENTER] won't affect runEventHandler(), because it's running in sandbox

            auto closed = std::make_shared<bool>(true);
            spawn(uid, code, [closed, onDone, this](const sol::protected_function_result &pfr)
            {
                *closed = false;
                std::vector<std::string> error;

                if(pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
                    std::vector<sol::object> resList;
                    resList.reserve(pfr.return_count());

                    for(int i = 0; i < pfr.return_count(); ++i){
                        resList.push_back(pfr[i]);
                    }

                    onDone(SYS_EXECDONE, sol::as_args(resList));
                }
                else{
                    if(error.empty()){
                        error.push_back("unknown error");
                    }

                    for(const auto &line: error){
                        g_monoServer->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                    }
                    throw fflerror("lua sandbox call failed in %s", to_cstr(uidf::getUIDString(m_actorPod->UID())));
                }
            },

            [closed, onDone]()
            {
                if(*closed){
                    onDone(SYS_EXECDONE, SYS_EXECCLOSE);
                }
            });
        }

        else{
            auto closed = std::make_shared<bool>(false);
            onDone.pushOnClose([closed]()
            {
                *closed = true;
            });

            m_actorPod->forward(uid, {AM_REMOTECALL, cerealf::serialize(SDRemoteCall
            {
                .code = code,
            })},

            [closed, s, uid, onDone](const ActorMsgPack &mpk)
            {
                // even thread is closed, we still exam the remote call result to detect error
                // but will not resume the thread anymore since it's already closed

                // pretty differnt than other close ops
                // normally onClose does real clean work, but here onClose only setup flags

                if(!(*closed)){
                    onDone.popOnClose();
                }

                switch(mpk.type()){
                    case AM_SDBUFFER:
                        {
                            // TODO shall we check if s still valid ?
                            // coroutine can be closed when the remote call is still in progress, tried looks still fine to access s

                            const auto sdRCR = mpk.deserialize<SDRemoteCallResult>();
                            if(sdRCR.error.empty()){
                                std::vector<sol::object> resList;
                                for(auto && var: sdRCR.varList){
                                    resList.emplace_back(luaf::buildLuaObj(s.getView(), std::move(var)));
                                }

                                if(!(*closed)){
                                    onDone(SYS_EXECDONE, sol::as_args(resList));
                                }
                            }
                            else{
                                // don't need to handle remote call error, peer side has reported the error
                                // _RSVD_NAME_uidExecute always returns valid result from remote peer to lua layer if not throw
                                fflassert(sdRCR.varList.empty(), sdRCR.error, sdRCR.varList);
                                for(const auto &line: sdRCR.error){
                                    g_monoServer->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
                                }
                                throw fflerror("lua call failed in %s", to_cstr(uidf::getUIDString(uid)));
                            }
                            break;
                        }
                    case AM_BADACTORPOD:
                        {
                            if(!(*closed)){
                                onDone(SYS_EXECBADUID);
                            }
                            break;
                        }
                    default:
                        {
                            throw fflerror("lua call failed in %s: %s", to_cstr(uidf::getUIDString(uid)), mpkName(mpk.type()));
                        }
                }
            });
        }
    });

    bindFunction("postNotify", [this](uint64_t uid, uint64_t threadKey, uint64_t threadSeqID, sol::variadic_args args)
    {
        std::vector<luaf::luaVar> argList;
        argList.reserve(args.size());

        for(const auto &arg: args){
            argList.emplace_back(luaf::buildLuaVar(sol::object(arg)));
        }

        m_actorPod->forward(uid, {AM_SENDNOTIFY, cerealf::serialize(SDSendNotify
        {
            .key = threadKey,
            .seqID = threadSeqID,
            .varList = std::move(argList),
            .waitConsume = false,
        })});
    });

    bindFunction(std::string("_RSVD_NAME_sendNotify") + SYS_COOP, [this](uint64_t questUID, uint64_t dstThreadKey, uint64_t dstThreadSeqID, sol::variadic_args args)
    {
        fflassert(uidf::isQuest(questUID), uidf::getUIDString(questUID));
        fflassert(dstThreadKey > 0);
        fflassert(args.size() >= 3, args.size());

        fflassert(args.begin()[args.size() - 3].is<sol::function>());
        fflassert(args.begin()[args.size() - 2].is<lua_Integer>());
        fflassert(args.begin()[args.size() - 1].is<lua_Integer>());

        const auto onDone      = args.begin()[args.size() - 3].as<sol::function>();
        const auto threadKey   = args.begin()[args.size() - 2].as<uint64_t>();
        const auto threadSeqID = args.begin()[args.size() - 1].as<uint64_t>();

        fflassert(onDone);
        fflassert(threadKey > 0);
        fflassert(threadSeqID > 0);

        auto closed = std::make_shared<bool>(false);
        pushOnClose(threadKey, threadSeqID, [closed]()
        {
            *closed = true;
        });

        const LuaCoopCallDoneFlag callDoneFlag;
        m_actorPod->forward(questUID, {AM_SENDNOTIFY, cerealf::serialize(SDSendNotify
        {
            .key = dstThreadKey,
            .seqID = dstThreadSeqID,
            .varList = luaf::vargBuildLuaVarList(args, 0, args.size() - 3),
            .waitConsume = false,
        })},

        [closed, callDoneFlag, onDone, threadKey, threadSeqID, this](const ActorMsgPack &mpk)
        {
            if(*closed){
                return;
            }
            else{
                popOnClose(threadKey, threadSeqID);
            }

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

            if(callDoneFlag){
                resume(threadKey, threadSeqID);
            }
        });
    });

    bindFunction("_RSVD_NAME_pickNotify", [this](uint64_t maxCount, uint64_t threadKey, uint64_t threadSeqID, sol::this_state s)
    {
        fflassert(threadKey);
        fflassert(threadSeqID);
        fflassert(hasKey(threadKey, threadSeqID), threadKey, threadSeqID);

        auto runnerPtr = m_runnerList.find(threadKey)->second.get();
        if(maxCount == 0 || maxCount >= runnerPtr->notifyList.size()){
            return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::move(runnerPtr->notifyList)));
        }
        else{
            auto begin = runnerPtr->notifyList.begin();
            auto end   = runnerPtr->notifyList.begin() + maxCount;

            std::deque<luaf::luaVar> vers(std::make_move_iterator(begin), std::make_move_iterator(end));
            runnerPtr->notifyList.erase(begin, end);

            return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::move(vers)));
        }
    });

    bindFunction("_RSVD_NAME_waitNotify", [this](uint64_t timeout, uint64_t threadKey, uint64_t threadSeqID, sol::this_state s) -> sol::object
    {
        fflassert(threadKey);
        fflassert(threadSeqID);
        fflassert(hasKey(threadKey, threadSeqID), threadKey, threadSeqID);

        auto runnerPtr = m_runnerList.find(threadKey)->second.get();
        if(!runnerPtr->notifyList.empty()){
            auto firstVar = std::move(runnerPtr->notifyList.front());
            runnerPtr->notifyList.pop_front();

            runnerPtr->needNotify = false;
            return luaf::buildLuaObj(sol::state_view(s), std::move(firstVar));
        }

        runnerPtr->needNotify = true;
        if(timeout > 0){
            const auto delayKey = m_actorPod->getSO()->addDelay(timeout, [threadKey, threadSeqID, this]()
            {
                if(auto runnerPtr = hasKey(threadKey, threadSeqID)){
                    runnerPtr->onClose.pop();
                    runnerPtr->needNotify = false;
                    resumeRunner(runnerPtr);
                }
            });

            runnerPtr->onClose.push([delayKey, this]()
            {
                m_actorPod->getSO()->removeDelay(delayKey);
            });
        }
        return sol::make_object(sol::state_view(s), sol::nil);
    });

    bindFunction("_RSVD_NAME_clearNotify", [this](uint64_t threadKey, uint64_t threadSeqID)
    {
        fflassert(threadKey);
        fflassert(threadSeqID);
        fflassert(hasKey(threadKey, threadSeqID), threadKey, threadSeqID);

        m_runnerList.find(threadKey)->second->notifyList.clear();
    });

    bindYielding("_RSVD_NAME_pauseYielding", [this](uint64_t msec, uint64_t threadKey, uint64_t threadSeqID)
    {
        fflassert(threadKey > 0, threadKey);
        fflassert(threadSeqID > 0, threadSeqID);

        const auto delayKey = m_actorPod->getSO()->addDelay(msec, [threadKey, threadSeqID, this]()
        {
            if(auto runnerPtr = hasKey(threadKey, threadSeqID)){
                // first pop the onclose function
                // otherwise the resume() will call it if the resume reaches end of code
                fflassert(!runnerPtr->onClose.empty());
                runnerPtr->onClose.pop();
                resumeRunner(runnerPtr);
            }
        });

        pushOnClose(threadKey, threadSeqID, [delayKey, this]()
        {
            m_actorPod->getSO()->removeDelay(delayKey);
        });
    });

    bindFunction("_RSVD_NAME_switchExclusiveFunc", [this](uint64_t exclusiveKey, uint64_t threadKey, uint64_t threadSeqID)
    {
        if(threadKey){
            fflassert(threadSeqID > 0, threadKey, threadSeqID);
        }
        else{
            fflassert(threadSeqID == 0, threadKey, threadSeqID);
        }

        if(const auto p = m_exclusiveFuncList.find(exclusiveKey); p != m_exclusiveFuncList.end()){
            if(p->second != decltype(p->second)(threadKey, threadSeqID)){
                close(p->second.first, p->second.second);
            }

            if(threadKey){
                p->second = decltype(p->second)(threadKey, threadSeqID);
            }
            else{
                m_exclusiveFuncList.erase(p);
            }
        }
        else if(threadKey){
            m_exclusiveFuncList.emplace(exclusiveKey, std::pair<uint64_t, uint64_t>(threadKey, threadSeqID));
        }
    });

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "serverluacoroutinerunner.lua"
    END_LUAINC()));
}

ServerLuaCoroutineRunner::~ServerLuaCoroutineRunner()
{}

uint64_t ServerLuaCoroutineRunner::getSeqID(uint64_t key) const
{
    if(auto p = m_runnerList.find(key); p != m_runnerList.end()){
        return p->second->seqID;
    }
    else{
        return 0;
    }
}

void ServerLuaCoroutineRunner::resume(uint64_t key, uint64_t seqID)
{
    if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
        resumeRunner(p->second.get());
    }
    else{
        // won't throw here
        // if needs to confirm the coroutine exists, use hasKey() first
    }
}

LuaThreadHandle *ServerLuaCoroutineRunner::hasKey(uint64_t key, uint64_t seqID) const
{
    const auto p = m_runnerList.find(key);
    return (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID) ? p->second.get() : nullptr;
}

void ServerLuaCoroutineRunner::pushOnClose(uint64_t key, uint64_t seqID, std::function<void()> onClose)
{
    fflassert(hasKey(key, seqID));
    fflassert(onClose);

    if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
        p->second->onClose.push(std::move(onClose));
    }
}

void ServerLuaCoroutineRunner::popOnClose(uint64_t key, uint64_t seqID)
{
    fflassert(hasKey(key, seqID));
    if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
        fflassert(!p->second->onClose.empty());
        p->second->onClose.pop();
    }
}

void ServerLuaCoroutineRunner::close(uint64_t key, uint64_t seqID)
{
    if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
        while(!p->second->onClose.empty()){
            if(p->second->onClose.top()){
                p->second->onClose.top()(); // only do clean work, don't modify onClose stack inside
            }
            p->second->onClose.pop();
        }
        m_runnerList.erase(p);
    }
}

uint64_t ServerLuaCoroutineRunner::spawn(uint64_t key, std::pair<uint64_t, uint64_t> reqAddr, const std::string &code)
{
    fflassert(key);
    fflassert(str_haschar(code));

    fflassert(reqAddr.first , reqAddr);
    fflassert(reqAddr.second, reqAddr);

    return spawn(key, code, [key, reqAddr, this](const sol::protected_function_result &pfr)
    {
        const auto fnOnThreadDone = [reqAddr, this](std::vector<std::string> error, std::vector<luaf::luaVar> varList)
        {
            if(!error.empty()){
                fflassert(varList.empty(), error, varList);
            }

            m_actorPod->forward(reqAddr, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
            {
                .error = std::move(error),
                .varList = std::move(varList),
            })});
        };

        std::vector<std::string> error;
        if(pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
            // initial run succeeds and coroutine finished
            // simple cases like: uidExecute(uid, [[ return getName() ]])
            //
            // for this case there is no need to uses coroutine
            // but we can not predict script from NPC is synchronized call or not
            //
            // for cases like spaceMove() we can use quasi-function from NPC side
            // but it prevents NPC side to execute commands like:
            //
            //     uidExecute(uid, [[
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
            // alternatively call a light function directly takes ~30 ns, best try is every time let uidExecute() do as much as possible
            //
            // light cases, no yield in script
            // directly return the result with save the runner
            fnOnThreadDone(std::move(error), luaf::pfrBuildLuaVarList(pfr));
        }
        else{
            if(error.empty()){
                error.push_back(str_printf("unknown error for runner: key = %llu", to_llu(key)));
            }
            fnOnThreadDone(std::move(error), {});
        }
    },

    [reqAddr, this]()
    {
        m_actorPod->forward(reqAddr, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
        {
            .varList
            {
                luaf::luaVar(SYS_EXECCLOSE),
            },
        })});
    });
}

uint64_t ServerLuaCoroutineRunner::spawn(uint64_t key, const std::string &code, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    fflassert(key);
    fflassert(str_haschar(code));

    const auto [p, added] = m_runnerList.insert_or_assign(key, std::make_unique<LuaThreadHandle>(*this, key, m_seqID++, std::move(onDone), std::move(onClose)));
    const auto currSeqID = p->second->seqID;

    resumeRunner(p->second.get(), str_printf(
        R"###( do                                                          )###""\n"
        R"###(     getTLSTable().threadKey   = %llu                        )###""\n"
        R"###(     getTLSTable().threadSeqID = %llu                        )###""\n"
        R"###(     getTLSTable().startTime   = getNanoTstamp()             )###""\n"
        R"###(                                                             )###""\n"
        R"###(     local _RSVD_NAME_autoClear<close> = autoClearTLSTable() )###""\n"
        R"###(     do                                                      )###""\n"
        R"###(        %s                                                   )###""\n"
        R"###(     end                                                     )###""\n"
        R"###( end                                                         )###""\n", to_llu(key), to_llu(currSeqID), code.c_str()));

    return currSeqID; // don't use p resumeRunner() can invalidate p
}

void ServerLuaCoroutineRunner::resumeRunner(LuaThreadHandle *runnerPtr, std::optional<std::string> codeOpt)
{
    // resume current runnerPtr
    // this function can invalidate runnerPtr if it's done

    fflassert(runnerPtr);
    fflassert(runnerPtr->callback);

    // here sol2 can tell if coroutine return nothing vs return nil
    //
    //    uidExecute(uid, [[        func_return_nil() ]]) -- pfr in c++ is {   }, empty
    //    uidExecute(uid, [[ return func_return_nil() ]]) -- pfr in c++ is {nil}, size-1-list
    //
    // but if in script we don't check
    //
    //    local result = uidExecute(uid, [[        func_return_nil() ]]) -- result in lua is nil
    //    local result = uidExecute(uid, [[ return func_return_nil() ]]) -- result in lua is nil, too
    //
    // this difference has been propogated to remote caller side by pfr serialization
    // this difference should be handled by caller side

    struct KeepRunner final
    {
        LuaThreadHandle * & refRunner;
        LuaThreadHandle *   oldRunner;

        KeepRunner(LuaThreadHandle * &runner, LuaThreadHandle *newRunner)
            : refRunner(runner)
            , oldRunner(runner)
        {
            refRunner = newRunner;
        }

        ~KeepRunner()
        {
            refRunner = oldRunner;
        }
    };

    const auto pfr = [&]()
    {
        const KeepRunner keep(m_currRunner, runnerPtr);
        return codeOpt.has_value() ? runnerPtr->callback(codeOpt.value()) : runnerPtr->callback();
    }();

    if(runnerPtr->callback){
        return;
    }

    // backup key and comletion handler
    // runnerPtr->onDone can invalidate runnerPtr, althrough this is no encouraged

    const auto threadKey = runnerPtr->key;
    const auto threadSeqID = runnerPtr->seqID;
    const auto onDoneFunc = std::move(runnerPtr->onDone);

    if(onDoneFunc){
        onDoneFunc(pfr);
    }
    else{
        std::vector<std::string> error;
        if(pfrCheck(pfr, [&error](const std::string &s){ error.push_back(s); })){
            if(pfr.return_count() > 0){
                g_monoServer->addLog(LOGTYPE_WARNING, "Dropped result: %s", to_cstr(str_any(luaf::pfrBuildLuaVarList(pfr))));
            }
        }
        else{
            if(error.empty()){
                error.push_back(str_printf("unknown error for runner: key = %llu", to_llu(runnerPtr->key)));
            }

            for(const auto &line: error){
                g_monoServer->addLog(LOGTYPE_WARNING, "%s", to_cstr(line));
            }
        }
    }

    close(threadKey, threadSeqID);
}
