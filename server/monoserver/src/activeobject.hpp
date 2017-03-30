/*
 * =====================================================================================
 *
 *       Filename: activeobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 03/29/2017 18:27:05
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

    STATE_MODE,
    STATE_NEVERDIE,
    STATE_ATTACKALL,
    STATE_PEACE,
    STATE_CANMOVE,
    STATE_WAITMOVE,
};

class ActiveObject: public ServerObject
{
    protected:
        std::array< uint8_t, 255> m_TypeV;
        std::array< uint8_t, 255> m_StateV;
        std::array<uint32_t, 255> m_StateTimeV;

    protected:
        ActorPod *m_ActorPod;

    protected:
        StateHook m_StateHook;
        std::priority_queue<DelayCmd> m_DelayCmdQ;

    public:
        ActiveObject();
       ~ActiveObject();

    public:
        // static type information
        // can safely called outside of ActiveObject
        // always return values, nonthrow, holds two types of type_info
        //      1. Type(EYE_COLOR)  : EYE_RED
        //                            EYE_BLACK
        //                            EYE_BROWN
        //
        //      2. Type(TYPE_HUMAN) : x // yes
        //                            0 // no
        // for type_info with a parameter, it use uint8_t as 255 possibilities
        // for type_info with 0 / 1 result it return 0 / x as false / true
        uint8_t Type(uint8_t nType) const
        {
            return m_TypeV[nType];
        }

    protected:
        void ResetType(uint8_t nTypeLoc, uint8_t nTypeValue)
        {
            m_TypeV[nTypeLoc] = nTypeValue;
        }

    protected:
        uint8_t  State(uint8_t);
        uint32_t StateTime(uint8_t);
        void     ResetState(uint8_t, uint8_t);

    public:
        Theron::Address Activate();

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
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;

    public:
        void Delay(uint32_t, const std::function<void()> &);

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    protected:
        virtual const char *ClassName() = 0;
#endif
};
