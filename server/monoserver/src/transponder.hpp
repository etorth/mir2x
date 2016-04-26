/*
 * =====================================================================================
 *
 *       Filename: transponder.hpp
 *        Created: 04/23/2016 10:51:19
 *  Last Modified: 04/25/2016 21:48:34
 *
 *    Description: base of actor model in mir2x, Theron::Actor acitvated at create
 *                 time so no way to control it, instead Transponder can 
 *                      1. react to message by callback
 *                      2. activate when needed
 *                      3. delete it to disable it
 *
 *                 it's not an object necessarily, in monoserver, an ``object" means
 *                 it has (UID(), AddTime()), but an transponder may not consist of
 *                 these attributes.
 *
 *                 Transponder : with an actor pod, override Operate() for operation
 *                 ReactObject : is an ``object" which has an acotr pod, and override
 *                               Operate() for operation
 *                 ActiveObject: is an ReactObject
 *
 *                 ReactObject is not an transponder, it's an ServerObject, because
 *                 I am trying to avoid MI.
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

#pragma once

#include "actorpod.hpp"
#include <Theron/Theron.h>

class Transponder
{
    protected:
        ActorPod   *m_ActorPod;

    public:
        Transponder()
            : m_ActorPod(nullptr)
        {
        }

        virtual ~Transponder()
        {
            delete m_ActorPod;
        }

    public:
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;

        virtual Theron::Address Activate()
        {
            extern Theron::Framework *g_Framework;
            m_ActorPod = new ActorPod(g_Framework,
                [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
                    Operate(rstMPK, stFromAddr);
                });
            return m_ActorPod->GetAddress();
        }
};
