/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/21/2016 23:02:31
 *    Description: server object with active state
 *                      1. it's active via actor pod
 *                      2. it's stateful
 *
 *                 actually stateless object can be implemented as a static object
 *                 after activated this kind of object can only be modified via messages
 *
 *                 access control protocol for ServerObject
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
#include "uidfunc.hpp"
#include "actorpod.hpp"
#include "statehook.hpp"
#include "delaycmd.hpp"
#include "messagepack.hpp"

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

class ServerObject
{
    private:
        const uint64_t m_UID;
        const std::string m_UIDName;

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
        ServerObject(uint64_t);

    public:
        virtual ~ServerObject();

    public:
        uint64_t UID() const
        {
            return ActorPodValid() ? m_UID : 0;
        }

        const char *UIDName() const
        {
            return ActorPodValid() ? m_UIDName.c_str() : "UID_INACTIVE";
        }

    protected:
        void SetState(uint8_t, uint8_t);

    protected:
        uint8_t  GetState    (uint8_t) const;
        uint32_t GetStateTime(uint8_t) const;

    public:
        virtual uint64_t Activate();

    protected:
        void Deactivate();

    public:
        bool ActorPodValid() const
        {
            return m_ActorPod && m_ActorPod->UID();
        }

    public:
        virtual void OperateAM(const MessagePack &) = 0;

    public:
        void Delay(uint32_t, const std::function<void()> &);
};
