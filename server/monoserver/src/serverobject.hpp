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
#include "uidf.hpp"
#include "actorpod.hpp"
#include "messagepack.hpp"
#include "delaycommand.hpp"
#include "statetrigger.hpp"

class ServerObject
{
    private:
        const uint64_t m_UID;
        const std::string m_UIDName;

    protected:
        ActorPod *m_actorPod = nullptr;

    protected:
        StateTrigger m_stateTrigger;

    protected:
        uint32_t m_delayCmdIndex = 0;
        std::priority_queue<DelayCommand> m_delayCmdQ;

    public:
        ServerObject(uint64_t);

    public:
        virtual ~ServerObject();

    public:
        uint64_t UID() const
        {
            return checkActorPod() ? rawUID() : 0;
        }

        uint64_t rawUID() const
        {
            return m_UID;
        }

        const char *UIDName() const
        {
            return checkActorPod() ? m_UIDName.c_str() : "UID_INACTIVE";
        }

    public:
        uint64_t activate();

    protected:
        virtual void onActivate() {}

    protected:
        void deactivate();

    public:
        bool checkActorPod() const
        {
            return m_actorPod && m_actorPod->UID();
        }

        void checkActorPodEx() const
        {
            if(!checkActorPod()){
                throw fflerror("invalid ActorPod");
            }
        }

    public:
        virtual void operateAM(const MessagePack &) = 0;

    public:
        void addDelay(uint32_t, std::function<void()>);
};
