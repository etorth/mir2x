/*
 * =====================================================================================
 *
 *       Filename: transponder.cpp
 *        Created: 04/27/2016 00:05:15
 *  Last Modified: 05/04/2016 14:35:53
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
{
    auto fnDelayCmdQueue = [this](){
        if(!m_DelayCmdQ.empty){
            extern MonoServer *g_MonoServer;
            if(m_DelayCmdQ.top().Tick() >= g_MonoServer->GetTickCount()){
                try{
                    m_DelayCmdQ.top()();
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception for delay cmd");
                }
                m_DelayCmdQ.pop();
            }
        }
    };
    Install("DelayCmdQueue", fnDelayCmdQueue);
}

Transponder::~Transponder()
{
    delete m_ActorPod;
}

Theron::Address Transponder::Activate()
{
    extern Theron::Framework *g_Framework;
    m_ActorPod = new ActorPod(g_Framework, [this](){ InnTrigger(); },
        [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
            Operate(rstMPK, stFromAddr);
        });
    m_ThisAddress = m_ActorPod->GetAddress();
    return m_ThisAddress;
}

Transponder::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    extern MonoServer *g_MonoServer;
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTickCount(), fnCmd);
}

// bool Transponder::Send(const MessagePack &rstMSG, const Theron::Address &rstFromAddress,
//         const std::function<void(const MessagePack &, const Theron::Address &)> &fnRespondOp)
// {
//     return m_ActorPod->Send(rstMSG, rstFromAddress, fnRespondOp);
// }
