#include <cstdio>
#include <atomic>
#include <cinttypes>

#include "uidf.hpp"
#include "totype.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "monoserver.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ActorPod::ActorPod(uint64_t uid,
        std::function<void()> fnTrigger,
        std::function<void(const ActorMsgPack &)> fnOperation,
        double updateFreq,
        uint64_t expireTime)
    : m_UID([uid]() -> uint64_t
      {
          fflassert(uid);
          fflassert(uidf::getUIDType(uid) != UID_RCV);
          fflassert(uidf::getUIDType(uid) >= UID_BEGIN, uid, uidf::getUIDString(uid));
          fflassert(uidf::getUIDType(uid) <  UID_END  , uid, uidf::getUIDString(uid));
          return uid;
      }())
    , m_trigger(std::move(fnTrigger))
    , m_operation(std::move(fnOperation))
    , m_updateFreq(regMetronomeFreq(updateFreq))
    , m_expireTime(expireTime)
{}

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
        g_monoServer->propagateException();
    }
}

void ActorPod::innHandler(const ActorMsgPack &mpk)
{
    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s <- %s : (type: %s, seqID: %llu, respID: %llu)", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
    }

    if(m_expireTime){
        while(!m_respondCBList.empty()){
            if(hres_tstamp().to_nsec() >= m_respondCBList.begin()->second.expireTime){
                // everytime when we received the new MPK we check if there is handler the timeout
                // also this time get counted into the monitor entry
                m_podMonitor.amProcMonitorList[AM_TIMEOUT].recvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.amProcMonitorList[AM_TIMEOUT].procTick));
                    m_respondCBList.begin()->second.op(AM_TIMEOUT);
                }
                m_respondCBList.erase(m_respondCBList.begin());
                continue;
            }

            // std::map<ID, Handler> keeps order in ID number
            // ID number is the Resp() of the responding messages
            //
            // and we guarantee "ID1 < ID2" => "ExpireTime1 <= ExpireTime2"
            // so if we get first non-expired handler, means the rest are all not expired
            break;
        }
    }

    if(mpk.respID()){
        // try to find the response handler for current responding message
        // 1.     find it, good
        // 2. not find it: 1. didn't register for it, we must prevent this at sending
        //                 2. repsonse is too late ooops and the handler has already be deleted
        if(auto p = m_respondCBList.find(mpk.respID()); p != m_respondCBList.end()){
            if(p->second.op){
                m_podMonitor.amProcMonitorList[mpk.type()].recvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.amProcMonitorList[mpk.type()].procTick));
                    p->second.op(mpk);
                }
            }
            else{
                throw fflerror("%s <- %s : (type: %s, seqID: %llu, respID: %llu): Response handler not executable", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
            }
            m_respondCBList.erase(p);
        }
        else{
            // should only caused by deletion of timeout
            // do nothing for this case, don't take this as an error
        }
    }
    else{
        // this is not a responding message
        // use default message handling operation
        if(m_operation){
            m_podMonitor.amProcMonitorList[mpk.type()].recvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.amProcMonitorList[mpk.type()].procTick));
                m_operation(mpk);
            }
        }
        else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            throw fflerror("%s <- %s : (type: %s, seqID: %llu, respID: %llu): Message handler not executable", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(mpk.from())), mpkName(mpk.type()), to_llu(mpk.seqID()), to_llu(mpk.respID()));
        }
    }

    // trigger is for all types of messages
    // currently we don't take trigger time into consideration

    if(m_trigger){
        raii_timer stTimer(&(m_podMonitor.triggerMonitor.procTick));
        m_trigger();
    }
}

uint64_t ActorPod::rollSeqID()
{
    // NOTE we have to use increasing seqID for one pod to support timeout
    // previously we reset m_nextSeqID when m_respondCBList is empty, as following:
    //
    //     if(m_respondCBList.empty){
    //         m_nextSeqID = 1;
    //     }
    //     else{
    //         m_nextSeqID++;
    //     }
    //     return m_nextSeqID;
    //
    // This implementation has bug when we support timeout:
    // when we sent a message and wait its response but timed out, we remove the respond callback from m_respondCBList
    // then m_respondCBList can be empty if we reset m_nextSeqID, however the responding actor message can come after the m_nextSeqID reset

    if(g_serverArgParser->enableUniqueActorMessageID){
        static std::atomic<uint64_t> s_nextSeqID {1}; // shared by all ActorPod
        return s_nextSeqID.fetch_add(1);
    }
    else{
        return m_nextSeqID++;
    }
}

bool ActorPod::forward(uint64_t uid, const ActorMsgBuf &mbuf, uint64_t respID)
{
    if(!uid){
        throw fflerror("%s -> NONE: (type: %s, seqID: 0, respID: %llu): Try to send message to an empty address", to_cstr(uidf::getUIDString(UID())), mpkName(mbuf.type()), to_llu(respID));
    }

    if(uid == UID()){
        throw fflerror("%s -> %s: (type: %s, seqID: 0, respID: %llu): Try to send message to itself", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(respID));
    }

    if(!mbuf){
        throw fflerror("%s -> %s: (type: AM_NONE, seqID: 0, respID: %llu): Try to send an empty message", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), to_llu(respID));
    }

    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (type: %s, seqID: 0, respID: %llu)", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(respID));
    }

    m_podMonitor.amProcMonitorList[mbuf.type()].sendCount++;
    return g_actorPool->postMessage(uid, {mbuf, UID(), 0, respID});
}

bool ActorPod::forward(uint64_t uid, const ActorMsgBuf &mbuf, uint64_t respID, std::function<void(const ActorMsgPack &)> opr)
{
    if(!uid){
        throw fflerror("%s -> NONE: (type: %s, seqID: 0, respID: %llu): Try to send message to an empty address", to_cstr(uidf::getUIDString(UID())), mpkName(mbuf.type()), to_llu(respID));
    }

    if(uid == UID()){
        throw fflerror("%s -> %s: (type: %s, seqID: 0, respID: %llu): Try to send message to itself", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(respID));
    }

    if(!mbuf){
        throw fflerror("%s -> %s: (type: AM_NONE, seqID: 0, respID: %llu): Try to send an empty message", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), to_llu(respID));
    }

    if(!opr){
        throw fflerror("%s -> %s: (type: %s, seqID: NA, respID: %llu): Response handler not executable", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(respID));
    }

    const auto seqID = rollSeqID();
    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (type: %s, seqID: %llu, respID: %llu)", to_cstr(uidf::getUIDString(UID())), to_cstr(uidf::getUIDString(uid)), mpkName(mbuf.type()), to_llu(seqID), to_llu(respID));
    }

    m_podMonitor.amProcMonitorList[mbuf.type()].sendCount++;
    if(g_actorPool->postMessage(uid, {mbuf, UID(), seqID, respID})){
        if(m_respondCBList.try_emplace(seqID, hres_tstamp().to_nsec() + m_expireTime, std::move(opr)).second){
            return true;
        }
        throw fflerror("failed to register response handler for posted message: %s", mpkName(mbuf.type()));
    }
    else{
        // respond the response handler here
        // if post failed, it can only be the UID is detached
        if(opr){
            m_podMonitor.amProcMonitorList[AM_BADACTORPOD].recvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.amProcMonitorList[AM_BADACTORPOD].procTick));
                opr(AM_BADACTORPOD);
            }
        }
        return false;
    }
}

void ActorPod::attach(std::function<void()> fnAtStart)
{
    g_actorPool->attach(this, std::move(fnAtStart));
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
            g_monoServer->addLog(LOGTYPE_DEBUG, "UID: %s %s: procTick %llu ms, sendCount %llu, recvCount %llu", uidf::getUIDString(UID()).c_str(), mpkName(nIndex), to_llu(nProcTick), to_llu(nSendCount), to_llu(nRecvCount));
        }
    }
}
