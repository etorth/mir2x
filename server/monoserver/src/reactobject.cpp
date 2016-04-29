/*
 * =====================================================================================
 *
 *       Filename: reactobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 04/28/2016 23:37:46
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

bool ReactObject::Send(const MessagePack &rstMSG,
        const Theron::Address &rstFromAddress, uint32_t *pRespond)
{
    return m_ActorPod->Send(rstMSG, rstFromAddress, pRespond);
}
