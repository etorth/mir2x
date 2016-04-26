/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 04/24/2016 00:45:17
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
#include <Theron/Theron.h>

#include "monoserver.hpp"
#include "messagepack.hpp"

template<typename MessageType = MessagePack>
class InnActorPod: public Theron::Actor
{
    protected:
        std::function<void(const MessageType &, const Theron::Address &)> m_Operate;

    public:
        explicit InnActorPod(Theron::Framework *pFramework,
                const std::function<void(const MessageType &, const Theron::Address &)> &fnOperate)
            : Theron::Actor(*pFramework)
            , m_Operate(fnOperate)
        {
            RegisterHandler(this, &InnActorPod::Handler);
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
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in actor pod");
                }
            }
        }
};

using ActorPod = InnActorPod<MessagePack>;
