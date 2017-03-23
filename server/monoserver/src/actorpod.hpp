/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 03/22/2017 14:25:59
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
 *                 TODO & TBD
 *                 The most important member function of ActorPod is Forward(), but I am
 *                 not sure how should I design this interface, two ways:
 *
 *                 0. bool Forward(MessagePack, Address)
 *                 1. bool Forward(MessagePack, Address, Respond)
 *                 2. bool Forward(MessageType, MessageBuffer, BufferLen, Address, Respond)
 *
 *
 *                 for case-0 we put Respond inside MessagePack, make it as part of the
 *                 message, this is OK conceptually.
 *
 *                 Real issue is we support ``ID() / Respond()", the id is not allocated
 *                 by the caller, but by internal logic in ActorPod. However in the view
 *                 of the caller, it send a ``const" message, but if internal logic should
 *                 assign ID for the message, we can
 *                 1. append ID info to the message, then it's not ``const" conceptually
 *                 2. Make InternalMessagePack(MessagePack), this hurt performance
 *
 *                 so if we just send the information to the internal logic and let it
 *                 make the MessagePack, then we overcome the conceptual crisis, what to
 *                 supply:
 *                 0. this message is for responding?
 *                 1. message type
 *                 2. buffer of message body
 *                 3. buffer length
 *                 4. target actor address for sure
 *
 *
 *                 I decide to use Forward() instead of Send()
 *                 since for ActorPod, ``Forward()" is more close to its role
 *
 *                 and this helps to avoid the override of Theron::Actor::Send(), if you
 *                 use using Theron::Actor::Send, then any undefined ActorPod::Send()
 *                 will be redirect to Theron::Actor::Send() since it's a template
 *
 *                 Won't make Forward() virtual
 *
 *                 ^
 *                 |
 *                 +---- this problem is solved perfectly by
 *                       1. Forward()
 *                       2. class MessageBuf
 *                       3. class MessagePack
 *
 *                       Forward accept a MessageBuf and inside it it makes a
 *                       class MessagePack and add ID/Resp info
 *
 *
 *                 TODO & TBD: to support trigger functionality.
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
 *                 so the best place to put trigger is inside class ActorPod, this means
 *                 for Transponder and ReactObject, we can't use trigger before activate
 *                 it (actiate means call ``new ActorPod"), so we design ActorPod ctor
 *                 as
 *
 *                 ActorPod(Trigger, Operation);
 *
 *                 to put the trigger here. Then for Transponder and ReactObject, we
 *                 provide method to install trigger handler:
 *
 *                 Transponder::Install("ClearQueue", fnClearQueue);
 *                 Transponder::Uninstall("ClearQueue");
 *
 *                 ReactObject::Install("ClearQueue", fnClearQueue);
 *                 ReactObject::Uninstall("ClearQueue");
 *
 *                 we need a map:
 *
 *                      std::unordered_map<std::string, std::function<void()>
 *
 *                 to store all handler, install/uninstall handlers. This map can only
 *                 be in Transponder/ReactObject, since if we put it in ActorPod, then
 *                 before Actiate(), Transponder::m_ActorPod is nullptr, how could we
 *                 put it in?
 *
 *                 After Activate(), all interaction between ActorPod *should* be via
 *                 message, then if we insist on putting this map inside ActorPod, we
 *                 have to call:
 *                      m_ActorPod->Install("ClearQueue", fnClearQueue);
 *                 this violate the rule since we refer to internal state m_ActorPod.
 *
 *                 So we follow the pattern how we define m_Operate, put the virtual
 *                 function invocation to m_Operate and m_Trigger, then Transponder,
 *                 ReactObject define how we handle message operation, event trigger
 *
 *                 Drawback is we need to copy the code in Transponder and ReactObject
 *                 to avoid MI.
 *                 ^
 *                 |
 *                 +-- the method currently I used:
 *
 *                      1. define class StateHook, which invoked on state changed
 *                      2. define ReactObject / Transponder as
 *                            ReactObject::m_ActorPod = new ActorPod(
 *                                  [this](){ ReactObject::Operate();  },
 *                                  [this](){ ReactObject::m_StateHook::Execute(); }
 *                            );
 *                      3. m_ActorPod->Activate(), then we can only communicate to
 *                         actor with messages
 *
 *                 by this I can put trigger logic in StateHook and won't copy-paste
 *                 code between ReactObject / Transponder.
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

class ActorPod: public Theron::Actor
{
    private:
        using MessagePackOperation = std::function<void(const MessagePack&, const Theron::Address &)>;
        // no need to keep the message pack itself
        // since when registering response operation, we always have the message pack avaliable
        typedef struct _RespondMessageRecord
        {
            MessagePackOperation RespondOperation;

            _RespondMessageRecord(const MessagePackOperation &rstOperation)
                : RespondOperation(rstOperation)
            {}
        }RespondMessageRecord;

    protected:
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
        std::unordered_map<uint32_t, RespondMessageRecord> m_RespondMessageRecordM;

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    protected:
        uint32_t    m_UID;
        uint32_t    m_AddTime;
        std::string m_Name;
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
            , m_AddTime(0)
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

        virtual ~ActorPod() = default;

    protected:
        uint32_t ValidID();
        void InnHandler(const MessagePack &, const Theron::Address);

    public:
        // TODO
        // it's user's responability to prevent send message to itself, for Theron this
        // behaivor is undefined, but ActorPod won't check it

        // just send a message, not a response, and won't exptect a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr)
        {
            return Theron::Actor::Send<MessagePack>({rstMB, 0, 0}, rstAddr);
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

        uint32_t AddTime()
        {
            return m_AddTime;
        }

        void BindPod(uint32_t nUID, uint32_t nAddTime, const char *szName)
        {
            m_UID      = nUID;
            m_AddTime  = nAddTime;
            m_Name     = szName;
        }
#endif
};
