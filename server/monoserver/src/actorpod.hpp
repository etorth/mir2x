/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 05/03/2016 15:09:01
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
 *                 The most important member function of ActorPod is Send(), but I am not
 *                 sure how should I design this interface, two ways:
 *
 *                 0. bool Send(MessagePack, Address)
 *                 1. bool Send(MessagePack, Address, Respond)
 *                 2. bool Send(MessageType, MessageBuffer, BufferLen, Address, Respond)
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
        using MessagePackOperation
            = std::function<void(const MessagePack&, const Theron::Address &)>;
        // no need to keep the message pack itself
        // since when registering response operation, we always have the message pack avaliable
        typedef struct _RespondMessageRecord {
            // MessagePack RespondMessagePack;
            MessagePackOperation    RespondOperation;

            // _RespondMessageRecord(const MessagePack & rstMPK,
            //         const MessagePackOperation &rstOperation)
            //     : RespondMessagePack(rstMPK)
            //     , RespondOperation(rstOperation)
            // {}
            _RespondMessageRecord(const MessagePackOperation &rstOperation)
                : RespondOperation(rstOperation)
            {}
        } RespondMessageRecord;

    protected:
        size_t m_ValidID;
        MessagePackOperation m_Operate;
        std::unordered_map<uint32_t, RespondMessageRecord> m_RespondMessageRecordM;

    public:
        explicit ActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessagePack &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_ValidID(0)
            , m_Operate(fnOperate)
        {
            RegisterHandler(this, &ActorPod::InnHandler);
        }

        virtual ~ActorPod() = default;

    protected:
        void InnHandler(const MessagePack &, Theron::Address);

    public:
        // send a responding message without asking for reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond = 0)
        {
            return Theron::Actor::Send<MessagePack>({rstMB, 0, nRespond}, rstAddr);
        }

        // send a non-responding message and exptecting a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr,
                const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
        {
            return Forward(rstMB, rstAddr, 0, fnOPR);
        }

        // send a responding message and exptecting a reply
        bool Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond,
                const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
        {
            // 1. get valid ID
            uint32_t nID = ValidID();

            // 2. send it
            bool bRet = Theron::Actor::Send<MessagePack>({rstMB, nID, nRespond}, rstAddr);

            // 3. if send succeed, then register the handler of responding message
            //    here we won't exam fnOPR's callability
            if(bRet){
                m_RespondMessageRecordM.emplace(std::make_pair(nID, fnOPR));
            }
            // 4. return whether we succeed
            return bRet;
        }

    private:
        uint32_t ValidID();
};
