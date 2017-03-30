/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 03/30/2017 01:35:06
 *
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

#include <functional>
#include <unordered_map>
#include <Theron/Theron.h>

#include "messagebuf.hpp"
#include "messagepack.hpp"

class ActorPod final: public Theron::Actor
{
    private:
        using MessagePackOperation = std::function<void(const MessagePack&, const Theron::Address &)>;
        // no need to keep the message pack itself
        // since when registering response operation, we always have the message pack avaliable
        // so we can put the pack copy in the lambda function instead of here
        typedef struct _RespondMessageRecord
        {
            // we put an expire time here
            // to support automatically remove the registered response handler
            // but currently I don't use it
            uint32_t ExpireTime;
            MessagePackOperation RespondOperation;

            _RespondMessageRecord(const MessagePackOperation &rstOperation)
                : ExpireTime(0)
                , RespondOperation(rstOperation)
            {}
        }RespondMessageRecord;

    private:
        size_t m_ValidID;
        MessagePackOperation m_Operate;
        // TODO & TBD
        // trigger is only for state update, so it won't accept any parameters w.r.t
        // message or time or xxx
        // but it will be invoked every time when message handling finished, since
        // for actor, the only chance to update its state is via message driving.
        //
        // here trigger is only a function, everytime when a message pased, it should
        // be invoked, however what to do during the invocation is defined by the handler
        // when create the actor
        //
        // most likely here we use StateHook::Execute();
        std::function<void()> m_Trigger;
        std::unordered_map<uint32_t, RespondMessageRecord> m_RespondMessageRecord;

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    private:
        uint32_t    m_UID;  // value provided by BindPod()
        std::string m_Name; // value provided by BindPod()
#endif

    public:
        // actor with trigger
        explicit ActorPod(Theron::Framework *pFramework, const std::function<void()> &fnTrigger,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_ValidID(0)
            , m_Operate(fnOperate)
            , m_Trigger(fnTrigger)
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
            , m_UID(0)
            , m_Name("ActorPod")
#endif
        {
            RegisterHandler(this, &ActorPod::InnHandler);
        }

        // actor without trigger, we just put a empty handler here
        explicit ActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate)
            : ActorPod(pFramework, std::function<void()>(), fnOperate)
        {}

        // we don't make the dtor virtual
        // since I don't want it's to be derived
       ~ActorPod() = default;

    protected:
        uint32_t ValidID();
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

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    public:
        const char *Name()
        {
            return m_Name.c_str();
        }

        uint32_t UID()
        {
            return m_UID;
        }

        void BindPod(uint32_t nUID, const char *szName)
        {
            m_UID      = nUID;
            m_Name     = szName;
        }
#endif
};
