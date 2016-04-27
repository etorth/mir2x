/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 04/27/2016 00:01:13
 *
 *    Description: why I made actor as a plug, because I want it to be a one to zero/one
 *                 mapping as ServerObject -> Actor
 *
 *                 Then a server object can  plug to the actor system slot for concurrent
 *                 operation, but also some server objects don't need this functionality
 *
 *                 And what's more, if an object is both ServerObject and Actor, we need
 *                 MI, but I really don't want to use MI
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

#include "monoserver.hpp"
#include "messagepack.hpp"

template<typename MessageType = MessagePack>
class InnActorPod: public Theron::Actor
{
    protected:
        size_t m_RespondCount;
        std::unordered_map<uint32_t, MessageType> m_RespondMessageM;
        std::function<void(const MessageType &, const Theron::Address &)> m_Operate;

    public:
        explicit InnActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessageType &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_RespondCount(0)
            , m_Operate(fnOperate)
        {
            RegisterHandler(this, &InnActorPod::Handler);
        }

        virtual ~InnActorPod() = default;

    protected:
        using Theron::Actor::Send;

    public:
        bool Send(const MessageType &rstMSG,
                const Theron::Address &rstFromAddress, uint32_t *pRespond = nullptr)
        {
            // for send message we only need to mark the respond flag as non-zero
            // there will be an internal allocated index for it
            if(rstMSG.Respond() != 0){
                m_RespondCount++;
                auto pMSG = m_RespondMessageM.find(m_RespondCount);
                if(pMSG != m_RespondMessageM.end()){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "response requested message overflows");
                    // TODO
                    // this function won't return;
                    g_MonoServer->Restart();
                }

                m_RespondMessageM[m_RespondCount] = rstMSG;
                m_RespondMessageM[m_RespondCount].Respond(m_RespondCount);
            }

            bool bRet = Theron::Actor::Send(rstMSG, rstFromAddress);
            if(bRet && rstMSG.Respond() && pRespond){ *pRespond = m_RespondCount; }

            return bRet;
        }

    protected:
        void Handler(const MessageType &rstMPK, Theron::Address stFromAddr)
        {
            if(m_Operate){
                // in theron address is recommanded to copy rather than ref, but
                // here we use const ref when passing to m_Operate, because when
                // calling m_Operate(), address already has a copy when executaion
                // goes inside Handler(), so always there is a valid copy of
                // address for m_Operate()
                //
                // if m_Operate() has any other async\ed operation upon the address
                // inside, it should handler by itself.
                //
                // so there is only one copy of Handler() when callback invoked, and
                // don't worry, Theron::Address is actually only a pointer so copying
                // is really cheap
                //
                // for MessageType copying, since Theron itself using const ref, so I
                // use const ref here
                try{
                    m_Operate(rstMPK, stFromAddr);
                }catch(...){
                    // 1. assume monoserver is ready when invoking callback
                    // 2. AddLog() is well defined in multithread environment
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in ActorPod");
                }
            }
        }
};

using ActorPod = InnActorPod<MessagePack>;
