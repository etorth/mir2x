/*
 * =====================================================================================
 *
 *       Filename: activeobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 08/10/2017 23:07:36
 *
 *    Description: server object with active state
 *                      1. it's active via actor pod
 *                      2. it's stateful
 *
 *                 actually stateless object can be implemented as a static object
 *                 after activated this kind of object can only be modified via messages
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
 *                 access control protocol for ActiveObject
 *                 for active object A and B, A can access B through b->func() iif
 *                    1. B outlives A
 *                    2. B::func() is constant qualified
 *                 otherwise any time if A need to access B, should use GetUIDRecord() and
 *                 forward a message to B for response
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
#include <atomic>
#include <Theron/Theron.h>

#include "actorpod.hpp"
#include "statehook.hpp"
#include "delaycmd.hpp"
#include "messagepack.hpp"
#include "serverobject.hpp"

enum ObjectType: uint8_t
{
    TYPE_NONE,
    TYPE_INFO,

    TYPE_CHAR,
    TYPE_EVENT,
    TYPE_UTILITY,
    
    // char information
    TYPE_NPC,
    TYPE_PLAYER,
    TYPE_MONSTER,

    // event information
    TYPE_MAGIC,
    TYPE_XXXXX,

    // utility information
    TYPE_SERVERMAP,
    TYPE_SERVICECORE,

    // creature information
    TYPE_CREATURE,

    TYPE_HUMAN,
    TYPE_UNDEAD,
    TYPE_ANIMAL,
};

enum ObjectState: uint8_t
{
    // three states of an active object
    STATE_NONE = 0,

    STATE_LIFECYCLE,
    STATE_EMBRYO,
    STATE_INCARNATED,
    STATE_PHANTOM,

    STATE_MOTION,
    STATE_MOVING,
    STATE_DEAD,
    STATE_GHOST,

    STATE_MODE,
    STATE_NEVERDIE,
    STATE_ATTACKALL,
    STATE_PEACE,
    STATE_CANMOVE,
    STATE_WAITMOVE,

    STATE_ATTACKMODE,
    STATE_ATTACKMODE_NORMAL,
    STATE_ATTACKMODE_DOGZ,
    STATE_ATTACKMODE_ATTACKALL,

    STATE_ONHORSE,
};

class ActiveObject: public ServerObject
{
    protected:
        std::array< uint8_t, 255> m_StateV;
        std::array<uint32_t, 255> m_StateTimeV;

    protected:
        ActorPod *m_ActorPod;

    protected:
        StateHook m_StateHook;

        // keep an incremental counter for DelayCmd
        // we have to maintain this count to make DelayCmdQ stable for sort
        uint32_t m_DelayCmdCount;
        std::priority_queue<DelayCmd> m_DelayCmdQ;

    public:
        ActiveObject();
       ~ActiveObject();

    protected:
        uint8_t GetState(uint8_t);
        uint32_t StateTime(uint8_t);

    protected:
        void SetState(uint8_t, uint8_t);

    public:
        Theron::Address Activate();

    protected:
        void Deactivate();

    public:
        bool ActorPodValid() const
        {
            return GetAddress() != Theron::Address::Null();
        }

        Theron::Address GetAddress() const
        {
            return m_ActorPod ? m_ActorPod->GetAddress() : Theron::Address::Null();
        }

    public:
        virtual void OperateAM(const MessagePack &, const Theron::Address &) = 0;

    public:
        void Delay(uint32_t, const std::function<void()> &);
};
