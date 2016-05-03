/*
 * =====================================================================================
 *
 *       Filename: reactobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 05/03/2016 14:26:46
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
#include "reactobject.hpp"

ReactObject::ReactObject(uint8_t nCategory, uint32_t nUID, uint32_t nAddTime)
    : ServerObject(nCategory, nUID, nAddTime)
    , m_ActorPod(nullptr)
{}

ReactObject::~ReactObject()
{
    delete m_ActorPod;
}

Theron::Address ReactObject::Activate()
{
    extern Theron::Framework *g_Framework;
    m_ActorPod = new ActorPod(g_Framework,
        [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
            Operate(rstMPK, stFromAddr);
        });

    return m_ActorPod->GetAddress();
}

// bool ReactObject::Send(const MessageBuf &rstMB, const Theron::Address &rstFromAddress,
//         const std::function<void(const MessagePack &, const Theron::Address &)> &fnRespondOp)
// {
//     return m_ActorPod->Send(rstMB, rstFromAddress, fnRespondOp);
// }
