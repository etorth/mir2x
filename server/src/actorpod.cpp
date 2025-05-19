#include <cstdio>
#include <atomic>
#include <cinttypes>

#include "protocoldef.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "netdriver.hpp"
#include "serverobject.hpp"
#include "server.hpp"
#include "serverargparser.hpp"

extern Server *g_server;
extern ActorPool *g_actorPool;
extern ServerArgParser *g_serverArgParser;

ActorPod::ActorPod(uint64_t uid, ServerObject *serverObject, double updateFreq)
    : m_UID([uid]() -> uint64_t
      {
          fflassert(uid);
          fflassert(uidf::getUIDType(uid) != UID_RCV  , uid, uidf::getUIDString(uid));
          fflassert(uidf::getUIDType(uid) >= UID_BEGIN, uid, uidf::getUIDString(uid));
          fflassert(uidf::getUIDType(uid) <  UID_END  , uid, uidf::getUIDString(uid));
          return uid;
      }())
    , m_SO(serverObject)
    , m_updateFreq(regMetronomeFreq(updateFreq))
{
    registerOp(AM_ACTIVATE, [thisptr = this]([[maybe_unused]] this auto self, const ActorMsgPack &) -> corof::awaitable<>
    {
        return thisptr->m_SO->onActivate();
    });
}

ActorPod::~ActorPod()
{
    // don't call destructor in running actor thread
    // could be its actor thread or the stealing actor thread

    // we can't check it here...
    // in ActorPod it shouldn't be aware who is calling itself

    try{
        g_actorPool->detach(this, nullptr);
    }
    catch(...){
        g_server->propagateException();
    }
}

void ActorPod::innHandler(const ActorMsgPack &mpk)
{
    if(g_serverArgParser->sharedConfig().traceActorMessage){
        g_server->addLog(LOGTYPE_DEBUG, "%s <- %s : (type: %s, seqID: %llu, respID: %llu)", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
    }

    if(mpk.respID()){
        if(auto p = m_respondCBList.find(mpk.respID()); p != m_respondCBList.end()){
            std::visit(VarDispatcher
            {
                [&mpk, this](std::coroutine_handle<corof::awaitable<ActorMsgPack>::promise_type> &handle)
                {
                    if(handle){
                        m_podMonitor.amProcMonitorList[mpk.type()].recvCount++;
                        {
                            raii_timer stTimer(&(m_podMonitor.amProcMonitorList[mpk.type()].procTick));
                            handle.promise().return_value(mpk);
                            handle.resume();
                        }
                    }
                    else{
                        throw fflerror("%s <- %s : (type: %s, seqID: %llu, respID: %llu): coroutine not executable", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
                    }
                },

                [&mpk, this](std::function<void(const ActorMsgPack &)> &op)
                {
                    if(op){
                        op(mpk);
                    }
                    else{
                        throw fflerror("%s <- %s : (type: %s, seqID: %llu, respID: %llu): coroutine not executable", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
                    }
                },
            },

            p->second);
            m_respondCBList.erase(p);
        }
        else{
            throw fflerror("%s <- %s : (type: %s, seqID: %llu, respID: %llu): no corresponding coroutine", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
        }
    }
    else{
        m_podMonitor.amProcMonitorList[mpk.type()].recvCount++;
        {
            raii_timer stTimer(&(m_podMonitor.amProcMonitorList[mpk.type()].procTick));
            if(const auto &mpkFunc = m_msgOpList.at(mpk.type())){
                mpkFunc(mpk).resume();
            }
            else{
                m_SO->onActorMsg(mpk).resume();
            }
        }
    }

    // trigger is for all types of messages
    // currently we don't take trigger time into consideration
    {
        raii_timer stTimer(&(m_podMonitor.triggerMonitor.procTick));
        m_SO->afterActorMsg();
    }
}

uint64_t ActorPod::rollSeqID()
{
    if(g_serverArgParser->sharedConfig().enableUniqueActorMessageID){
        static std::atomic<uint64_t> s_nextSeqID {1}; // shared by all ActorPod
        return s_nextSeqID.fetch_add(1);
    }
    else{
        return m_nextSeqID++;
    }
}

std::pair<uint64_t, uint64_t> ActorPod::createWaitToken(uint64_t tick, std::function<void(const ActorMsgPack &)> op)
{
    const auto seqID = rollSeqID();
    const auto empOK = m_respondCBList.try_emplace(seqID, std::move(op)); // prepare cb before request timeout

    fflassert(empOK.second);
    return
    {
        g_actorPool->requestTimeout({UID(), seqID}, tick),
        seqID,
    };
}

bool ActorPod::cancelWaitToken(const std::pair<uint64_t, uint64_t> &token)
{
    return g_actorPool->cancelTimeout(token.first);
}

std::optional<uint64_t> ActorPod::doPost(const std::pair<uint64_t, uint64_t> &addr, ActorMsgBuf mbuf, bool waitResp)
{
    const auto uid = addr.first;
    const auto respID = addr.second;

    if(!uid){
        throw fflerror("%s -> NONE: (type: %s, seqID: 0, respID: %llu): Try to send message to an empty address", to_cstr(uidf::getUIDString(UID())), mpkName(mbuf.type()), to_llu(respID));
    }

    if(uid == UID()){
        throw fflerror("%s -> %s: (type: %s, seqID: 0, respID: %llu): Try to send message to itself", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(respID));
    }

    if(!mbuf){
        throw fflerror("%s -> %s: (type: AM_NONE, seqID: 0, respID: %llu): Try to send an empty message", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), to_llu(respID));
    }

    const auto seqID = waitResp ? rollSeqID() : UINT64_C(0);
    if(g_serverArgParser->sharedConfig().traceActorMessage){
        g_server->addLog(LOGTYPE_DEBUG, "%s -> %s: (type: %s, seqID: %llu, respID: %llu)", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(seqID), to_llu(respID));
    }

    m_podMonitor.amProcMonitorList[mbuf.type()].sendCount++;
    if(g_actorPool->postMessage(uid, {mbuf, UID(), seqID, respID})){
        return seqID;
    }
    else{
        return std::nullopt;
    }
}

void ActorPod::attach()
{
    g_actorPool->attach(this);
}

void ActorPod::detach(std::function<void()> fnAtExit) const
{
    // we can call detach in its message handler
    // remember the message handler can be executed by worker thread or stealing worker thread
    // we can't guarantee that after detach no thread can access the actor
    //
    //  void MessageHandler()
    //  {
    //      if(bNeedDetach){
    //          detach();
    //          OtherRelease(); // here still accessing the actor
    //          return;         // after this line we can guarantee actor is not running
    //      }
    //  }
    //
    // it's worse to call it out of its message handler
    // when detach returns we can't guarantee if the actor is till handling message
    //
    g_actorPool->detach(this, std::move(fnAtExit));
}

bool ActorPod::checkUIDValid(uint64_t uid)
{
    if(!uid){
        throw fflerror("invalid zero UID");
    }
    return g_actorPool->checkUIDValid(uid);
}

void ActorPod::PrintMonitor() const
{
    for(size_t nIndex = 0; nIndex < m_podMonitor.amProcMonitorList.size(); ++nIndex){
        const uint64_t nProcTick  = m_podMonitor.amProcMonitorList[nIndex].procTick / 1000000;
        const uint64_t nSendCount = m_podMonitor.amProcMonitorList[nIndex].sendCount;
        const uint64_t nRecvCount = m_podMonitor.amProcMonitorList[nIndex].recvCount;
        if(nSendCount || nRecvCount){
            g_server->addLog(LOGTYPE_DEBUG, "UID: %s %s: procTick %llu ms, sendCount %llu, recvCount %llu", uidf::getUIDString(UID()).c_str(), mpkName(nIndex), to_llu(nProcTick), to_llu(nSendCount), to_llu(nRecvCount));
        }
    }
}

void ActorPod::postNet(uint32_t channID, uint8_t headCode, const void *buf, size_t bufSize, uint64_t respID)
{
    fflassert(channID);
    g_actorPool->m_netDriver->post(channID, headCode, buf, bufSize, respID);
}

void ActorPod::postNet(uint8_t headCode, const void *buf, size_t bufSize, uint64_t respID)
{
    fflassert(m_channID);
    g_actorPool->m_netDriver->post(m_channID, headCode, buf, bufSize, respID);
}

void ActorPod::bindNet(uint32_t channID)
{
    fflassert(!m_channID, m_channID);

    m_channID = channID;
    g_actorPool->m_netDriver->bindPlayer(channID, UID());
}

void ActorPod::closeNet()
{
    if(m_channID){
        g_actorPool->m_netDriver->close(m_channID);
        m_channID = 0;
    }
}
