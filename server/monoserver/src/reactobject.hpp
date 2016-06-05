/*
 * =====================================================================================
 *
 *       Filename: reactobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 06/05/2016 12:13:52
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
#include <queue>
#include <Theron/Theron.h>

#include "hook.hpp"
#include "delaycmd.hpp"
#include "messagepack.hpp"
#include "serverobject.hpp"

class ActorPod;
class ReactObject: public ServerObject
{
    protected:
        ActorPod *m_ActorPod;

        // the document of Theron says we can call GetAddress() outside of 
        // Actor, but is that safe?
        Theron::Address m_ThisAddress;

    protected:
        Hook m_Hook;
        std::priority_queue<DelayCmd> m_DelayCmdQ;

    public:
        ReactObject(uint8_t);
        ~ReactObject();

    protected:
        bool AccessCheck();
        bool ActorPodValid();

    public:
        virtual Theron::Address Activate();

        Theron::Address GetAddress()
        {
            return m_ThisAddress;
        }

    public:
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;

    public:
        void Delay(uint32_t, const std::function<void()> &);
};
