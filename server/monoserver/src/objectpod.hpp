/*
 * =====================================================================================
 *
 *       Filename: objectpod.hpp
 *        Created: 04/20/2016 21:49:14
 *  Last Modified: 04/21/2016 01:26:25
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

#include "log.hpp"
#include "messagepack.hpp"

// TODO I have to put it here, kind of dirty
extern Theron::Framework *g_Framework;

template<MessageType = MessagePack>
class ActorPod: public Theron::Actor
{
    protected:
        std::function<void(const MessageType &, Theron::Address)> m_Operate;

    public:
        explicit ActorPod(
                const std::function<void(const MessageType &, Theron::Address)> & fnOperate,
                Theron::Framework *pFramework = g_Framework)
            : Theron::Actor(*pFramework)
            , m_Operate(fnOperate)
        {
            RegisterHandler(this, &ActorPod::Handler);
        }

    protected:
        void Handler(const MessageType &rstMPK, Theron::Address stFromAddr)
        {
            if(m_Operate){
                // here we copy Theron::Address two times, one from external
                // to Handler call stack, and one from Handler call stack to
                // m_Operate call stack.
                //
                // don't worry, Theron::Address is actually only a pointer so
                // copying is really cheap
                try{
                    m_Operate(rstMPK, stFromAddr)
                }catch(...){
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING, "exception caught in ActorPod handler");
                }
            }
        }
};

using ObjectPod = ActorPod<MessagePack>;
