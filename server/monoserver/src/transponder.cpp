/*
 * =====================================================================================
 *
 *       Filename: transponder.cpp
 *        Created: 04/27/2016 00:05:15
 *  Last Modified: 06/09/2016 17:18:20
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "actorpod.hpp"
#include "monoserver.hpp"
#include "transponder.hpp"

Transponder::Transponder()
    : m_ActorPod(nullptr)
    , m_ThisAddress(Theron::Address::Null())
{
    auto fnDelayCmdQueue = [this](){
        if(!m_DelayCmdQ.empty()){
            extern MonoServer *g_MonoServer;
            if(m_DelayCmdQ.top().Tick() >= g_MonoServer->GetTimeTick()){
                try{
                    m_DelayCmdQ.top()();
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception for delay cmd");
                }
                m_DelayCmdQ.pop();
            }
        }

        return false;
    };
    m_StateHook.Install("DelayCmdQueue", fnDelayCmdQueue);
}

Transponder::~Transponder()
{
    delete m_ActorPod;
}

Theron::Address Transponder::Activate()
{
    if(!m_ActorPod){
        extern Theron::Framework *g_Framework;
        m_ActorPod = new ActorPod(g_Framework, [this](){ m_StateHook.Execute(); },
                [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
                this->Operate(rstMPK, stFromAddr);
                });
#ifdef MIR2X_DEBUG
        // transponder won't have (UID, AddTime), it has name though
        // define ClassName() rather than Name() since for class Player it has a Name() function
        m_ActorPod->BindPod(0, 0, ClassName());
#endif
        m_ThisAddress = m_ActorPod->GetAddress();
    }

    // TODO & TBD no idea of whether this is needed
    return m_ThisAddress;
}

void Transponder::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    extern MonoServer *g_MonoServer;
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTimeTick(), fnCmd);
}

bool Transponder::ActorPodValid()
{
    return m_ActorPod && m_ActorPod->GetAddress();
}

// bool Transponder::Send(const MessagePack &rstMSG, const Theron::Address &rstFromAddress,
//         const std::function<void(const MessagePack &, const Theron::Address &)> &fnRespondOp)
// {
//     return m_ActorPod->Send(rstMSG, rstFromAddress, fnRespondOp);
// }
