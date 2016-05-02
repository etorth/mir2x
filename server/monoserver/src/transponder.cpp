/*
 * =====================================================================================
 *
 *       Filename: transponder.cpp
 *        Created: 04/27/2016 00:05:15
 *  Last Modified: 05/02/2016 01:03:02
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

#include "transponder.hpp"
#include "actorpod.hpp"

Transponder::Transponder()
    : m_ActorPod(nullptr)
{
}

Transponder::~Transponder()
{
    delete m_ActorPod;
}

Theron::Address Transponder::Activate()
{
    extern Theron::Framework *g_Framework;
    m_ActorPod = new ActorPod(g_Framework,
        [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
            Operate(rstMPK, stFromAddr);
        });
    m_ThisAddress = m_ActorPod->GetAddress();
    return m_ThisAddress;
}

bool Transponder::Send(const MessagePack &rstMSG, const Theron::Address &rstFromAddress,
        const std::function<void(const MessagePack &, const Theron::Address &)> &fnRespondOp)
{
    return m_ActorPod->Send(rstMSG, rstFromAddress, fnRespondOp);
}
