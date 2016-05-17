/*
 * =====================================================================================
 *
 *       Filename: reactobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 05/11/2016 16:01:19
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
#include <Theron/Theron.h>

#include "messagepack.hpp"
#include "serverobject.hpp"

class ActorPod;
class ReactObject: public ServerObject
{
    protected:
        ActorPod *m_ActorPod;

    protected:
        bool AccessCheck();

    public:
        ReactObject(uint8_t, uint32_t, uint32_t);
        ~ReactObject();

    public:
        virtual Theron::Address Activate();
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;
};
