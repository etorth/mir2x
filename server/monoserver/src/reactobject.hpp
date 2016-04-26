/*
 * =====================================================================================
 *
 *       Filename: reactobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 04/25/2016 21:34:14
 *
 *    Description: object only react to message, with an object pod
 *                 atoms of an react object:
 *                      1. before Activate(), we can call its internal method by ``this"
 *                      2. after Activate(), we can only use ReactObject::Send()
 *
 *                 In other word, after Activate(), react object can only communicate
 *                 with react object or receiver
 *
 *                 This prevent me implement MonoServer as react object. For MonoServer
 *                 it needs to manager SessionHub. However SessionHub is not an actor, so
 *                 if MonoServer is an react object, we have to launch SessionHub before
 *                 calling of MonoServer::Activate(), but, before activation of MonoServer
 *                 we don't have the address of Mo MonoServer to pass to SessionHub!
 *
 *                 In my design, SessionHub create Session's with SID and pass it to the
 *                 MonoServer, then MonoServer check info of this connection from DB and
 *                 create player object, bind Session pointer to the player and send the
 *                 player to proper RegionMonitor via ServerMap object.
 *
 *                 Another thing is for g_MonoServer->AddLog(...), if make MonoServer as
 *                 a react object, we can't use it anymore
 *
 *                 So let's make MonoServer as a receriver instead.
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
#include "serverobject.hpp"

class ReactObject: public ServerObject
{
    protected:
        ActorPod *m_ActorPod;

    public:
        ReactObject(uint8_t nCategory, uint32_t nUID, uint32_t nAddTime)
            : ServerObject(nCategory, nUID, nAddTime)
            , m_ActorPod(nullptr)
        {}

        ~ReactObject()
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
