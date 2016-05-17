/*
 * =====================================================================================
 *
 *       Filename: reactobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 05/11/2016 16:03:15
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

// TODO & TBD
// when an actor is activated by more than one time, we can
// 1. delete previously allocated actor and create a new one
// 2. just return current address
//
// Now I use method-2, since the address could hanve been assigned to many other place
// for communication, delete it may cause problems
//
// And if we really want to change the address of current object, maybe we need to
// delte current object in total and create a new one instead
Theron::Address ReactObject::Activate()
{
    if(!m_ActorPod){
        extern Theron::Framework *g_Framework;
        m_ActorPod = new ActorPod(g_Framework,
            [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
                Operate(rstMPK, stFromAddr);
            });
    }

    return m_ActorPod->GetAddress();
}

bool ReactObject::AccessCheck()
{
    if(m_ActorPod){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "Direct access internal state of an actor after activation");
        g_MonoServer->Restart();

        return false;
    }
    return true;
}
