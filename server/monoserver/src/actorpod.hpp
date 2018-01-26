/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *    Description: why I made actor as a plug, because I want it to be a one to zero/one
 *                 mapping as ServerObject -> Actor
 *
 *                 Then a server object can plug to the actor system slot for concurrent
 *                 operation, but also some server objects don't need this functionality
 *
 *                 And what's more, if an object is both ServerObject and Actor, we need
 *                 MI, but I really don't want to use MI
 *
 *                 every time when class state updated, we call the trigger to check what
 *                 should be done, for actor the only way to update state is to handle
 *                 message, then:
 *
 *                 if(response message){
 *                     check response pool;
 *                 }else{
 *                     call operation handler;
 *                 }
 *
 *                 if(trigger registered){
 *                     call trigger;
 *                 }
 *
 *                 the best place to put trigger is inside class ActorPod, this means for
 *                 ActiveObject, we can't use trigger before activate it, define ctor of
 *                 ActorPod as:
 *
 *                 ActorPod(Trigger, Operation);
 *
 *                 to provide the trigger, this trigger can handle delay commands, so we
 *                 define class DelayCmd and take the trigger as:
 *
 *                      auto fnTrigger = [this](){ m_StateHook.Execute(); }
 *
 *                 and
 *
 *                      m_StateHook.Install("ClearQueue", fnClearQueue);
 *                      m_StateHook.Uninstall("ClearQueue");
 *
 *                 then every time when new actor messages handled, we can check it
 *                 put the trigger here. Then for Transponder and ReactObject, we
 *                 provide method to install trigger handler:
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

#include <map>
#include <string>
#include <functional>
#include <Theron/Theron.h>

#include "messagebuf.hpp"
#include "messagepack.hpp"

class ActorPod final: public Theron::Actor
{
    private:
        using MessagePackOperation = std::function<void(const MessagePack&, const Theron::Address &)>;
        // no need to keep the message pack itself
        // since when registering response operation, we always have the message pack avaliable
        // so we can put the pack copy in the lambda function capture list instead of here
        struct RespondMessageRecord
        {
            // we put an expire time here
            // to support automatically remove the registered response handler
            uint32_t ExpireTime;
            MessagePackOperation RespondOperation;

            RespondMessageRecord(uint32_t nExpireTime, const MessagePackOperation &rstOperation)
                : ExpireTime(nExpireTime)
                , RespondOperation(rstOperation)
            {}
        };

    private:
        // trigger is only for state update, so it won't accept any parameters w.r.t
        // message or time or xxx
        //
        // it will be invoked every time when message handling finished
        // for actors the only chance to update their state is via message driving.
        //
        // conceptually one actor could have more than one trigger
        // for that we should register / de-register those triggers to m_Trigger 
        // most likely here we use StateHook::Execute();
        //
        // trigger is provided at initialization and never change
        const std::function<void()> m_Trigger;

        // handler to handle every informing messages
        // informing messges means we didn't register an handler for it
        // this handler is provided at the initialization time and never change
        const MessagePackOperation m_Operation;

    private:
        // used by ValidID()
        // to create unique proper ID for an message expcecting response
        uint32_t m_ValidID;

        // for expire time check
        // zero expire time means we never expire any handler for current pod
        // we can put argument to specify the expire time of each handler but not necessary
        const uint32_t m_ExpireTime;

        // use std::map instead of std::unordered_map
        //
        // 1. we have to scan the map every time when new message comes to remove expired ones
        //    std::unordered_map is slow for scan the entire map
        //    I can maintain another std::priority_queue based on expire time
        //    but it's hard to remove those entry which executed before expire from the queue
        //
        // 2. std::map keeps entries in order by Resp number
        //    Resp number gives strict order of expire time, excellent freature by std::map
        //    then when checking expired ones, we start from std::map::begin() and stop at the fist non-expired one
        std::map<uint32_t, RespondMessageRecord> m_RespondMessageRecord;

    private:
        // actor information provided by BindPod()
        // actor itself don't create this UID / Name info
        uint32_t    m_UID;
        std::string m_Name;

    public:
        // actor with trigger provided externally
        explicit ActorPod(Theron::Framework *pFramework, const std::function<void()> &fnTrigger,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate, uint32_t nExpireTime = 3600 * 1000)
            : Theron::Actor(*pFramework)
            , m_Trigger(fnTrigger)
            , m_Operation(fnOperate)
            , m_ValidID(0)
            , m_ExpireTime(nExpireTime)
            , m_RespondMessageRecord()
            , m_UID(0)
            , m_Name("ActorPod")
        {
            RegisterHandler(this, &ActorPod::InnHandler);
        }

        // actor without trigger, we just put a empty handler here
        explicit ActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate, uint32_t nExpireTime = 3600 * 1000)
            : ActorPod(pFramework, std::function<void()>(), fnOperate, nExpireTime)
        {}

    public:
        ~ActorPod() = default;

    private:
        // get an ID to a message expcecting a response
        // when the responding message comes we use the Resp to find its responding handler
        // requirement for the ID:
        // 1. non-zero, zero ID means no response expected
        // 2. unique for registered handler in m_RespondMessageRecord at one time
        //    a number can be re-used, but we should make sure no mistake happen for ID -> Hanlder mapping
        uint32_t ValidID();

        // to register to Theron::Actor
        // works as a wrapper for (m_Operation, m_Trigger, m_RespondMessageRecord)
        // Theron::Actor accept Theron::Actor::InnHandler only instead of std::function<void(...)>
        void InnHandler(const MessagePack &, const Theron::Address);

    public:
        // just send a message, not a response, and won't exptect a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr)
        {
            return Forward(rstMB, rstAddr, 0);
        }

        // sending a response, won't exptect a reply
        bool Forward(const MessageBuf &, const Theron::Address &, uint32_t);

        // send a non-responding message and exptecting a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr,
                const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
        {
            return Forward(rstMB, rstAddr, 0, fnOPR);
        }

        // send a responding message and exptecting a reply
        bool Forward(const MessageBuf &, const Theron::Address &, uint32_t,
                const std::function<void(const MessagePack&, const Theron::Address &)> &);

    public:
        const char *Name() const
        {
            return m_Name.c_str();
        }

        uint32_t UID() const
        {
            return m_UID;
        }

    public:
        void BindPod(uint32_t nUID, const char *szName)
        {
            m_UID  = nUID;
            m_Name = szName;
        }

        void Detach()
        {
            DeregisterHandler(this, &ActorPod::InnHandler);
        }
};
