#include <memory>
#include "luaf.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "serdesmsg.hpp"
#include "monoserver.hpp"
#include "serverluacoroutinerunner.hpp"

extern MonoServer *g_monoServer;
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
    bindFunctionCoop("_RSVD_NAME_uidExecute", [this](LuaCoopResumer onDone, LuaCoopState s, uint64_t uid, std::string code)
    {
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
                            for(auto && var: cerealf::deserialize<std::vector<luaf::luaVar>>(sdRCR.serVarList)){
                                resList.emplace_back(luaf::buildLuaObj(s.getView(), std::move(var)));
                            }

                            if(!(*closed)){
                                onDone(SYS_EXECDONE, sol::as_args(resList));
                            }
                        }
                        else{
                            // don't need to handle remote call error, peer side has reported the error
                            // _RSVD_NAME_uidExecute always returns valid result from remote peer to lua layer if not throw
                            fflassert(sdRCR.serVarList.empty(), sdRCR.error, sdRCR.serVarList);
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
    });

    bindFunction("postNotify", [this](uint64_t uid, uint64_t threadKey, uint64_t threadSeqID, sol::variadic_args args)
    {
        std::vector<luaf::luaVar> argList;
        argList.reserve(args.size());

        for(const auto &arg: args){
            argList.emplace_back(luaf::buildLuaVar(sol::object(arg)));
        }

        m_actorPod->forward(uid, {AM_QUESTNOTIFY, cerealf::serialize(SDQuestNotify
        {
            .key = threadKey,
            .seqID = threadSeqID,
            .varList = std::move(argList),
            .waitConsume = false,
        })});
    });

    bindFunctionCoop("_RSVD_NAME_sendNotify", [this](LuaCoopResumer onDone, uint64_t uid, uint64_t threadKey, uint64_t threadSeqID, sol::object notifyArg) //TODO support sol::variadic_args
    {
        std::vector<luaf::luaVar> argList;
        argList.reserve(1);

        argList.emplace_back(luaf::buildLuaVar(notifyArg));

        m_actorPod->forward(uid, {AM_QUESTNOTIFY, cerealf::serialize(SDQuestNotify
        {
            .key = threadKey,
            .seqID = threadSeqID,
            .varList = std::move(argList),
            .waitConsume = false,
        })},

        [onDone](const ActorMsgPack &mpk)
        {
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
    });

    bindFunction("_RSVD_NAME_waitNotify", [this](uint64_t threadKey, uint64_t threadSeqID, sol::this_state s)
    {
        fflassert(threadKey);
        fflassert(threadSeqID);
        fflassert(hasKey(threadKey, threadSeqID), threadKey, threadSeqID);

        auto runnerPtr = m_runnerList.find(threadKey)->second.get();
        return luaf::buildLuaObj(sol::state_view(s), luaf::buildLuaVar(std::move(runnerPtr->notifyList)));
    });

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "serverluacoroutinerunner.lua"
    END_LUAINC()));
}

uint64_t ServerLuaCoroutineRunner::spawn(uint64_t key, std::pair<uint64_t, uint64_t> reqAddr, const std::string &code)
{
    fflassert(key);
    fflassert(str_haschar(code));

    fflassert(reqAddr.first , reqAddr);
    fflassert(reqAddr.second, reqAddr);

    return spawn(key, code, [key, reqAddr, this](const sol::protected_function_result &pfr)
    {
        const auto fnOnThreadDone = [reqAddr, this](std::vector<std::string> error, std::string serVarList)
        {
            if(!error.empty()){
                fflassert(serVarList.empty(), error, serVarList.size());
            }

            m_actorPod->forward(reqAddr, {AM_SDBUFFER, cerealf::serialize(SDRemoteCallResult
            {
                .error = std::move(error),
                .serVarList = std::move(serVarList),
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
            fnOnThreadDone(std::move(error), cerealf::serialize(luaf::pfrBuildLuaVarList(pfr), false));
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
            .serVarList = cerealf::serialize(std::vector<luaf::luaVar>
            {
                luaf::luaVar(SYS_EXECCLOSE),
            }),
        })});
    });
}

uint64_t ServerLuaCoroutineRunner::spawn(uint64_t key, const std::string &code, std::function<void(const sol::protected_function_result &)> onDone, std::function<void()> onClose)
{
    fflassert(key);
    fflassert(str_haschar(code));

    const auto [p, added] = m_runnerList.insert_or_assign(key, std::make_unique<_CoroutineRunner>(*this, key, m_seqID++, std::move(onDone), std::move(onClose)));
    const auto currSeqID = p->second->seqID;

    resumeRunner(p->second.get(), str_printf(
        R"###( getTLSTable().threadKey = %llu                                  )###""\n"
        R"###( getTLSTable().threadSeqID = %llu                                )###""\n"
        R"###( getTLSTable().startTime = getNanoTstamp()                       )###""\n"
        R"###( local _RSVD_NAME_autoTLSTableClear<close> = autoClearTLSTable() )###""\n"
        R"###( do                                                              )###""\n"
        R"###(    %s                                                           )###""\n"
        R"###( end                                                             )###""\n", to_llu(key), to_llu(currSeqID), code.c_str()));

    return currSeqID; // don't use p resumeRunner() can invalidate p
}

void ServerLuaCoroutineRunner::resumeRunner(ServerLuaCoroutineRunner::_CoroutineRunner *runnerPtr, std::optional<std::string> codeOpt)
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

    const auto pfr = codeOpt.has_value() ? runnerPtr->callback(codeOpt.value()) : runnerPtr->callback();

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
