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
 *                 ServerObject, we can't use trigger before activate it, define ctor of
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

#include "messagebuf.hpp"
#include "messagepack.hpp"

class ActorPod final
{
    private:
        friend class ActorPool;

    private:
        struct RespondHandler
        {
            uint32_t ExpireTime;
            std::function<void(const MessagePack &)> Operation;

            RespondHandler(uint32_t nExpireTime, std::function<void(const MessagePack &)> stOperation)
                : ExpireTime(nExpireTime)
                , Operation(std::move(stOperation))
            {}
        };

    private:
        const uint64_t m_UID;

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
        const std::function<void(const MessagePack &)> m_Operation;

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
        //    Resp number gives strict order of expire time, excellent feature by std::map
        //    then when checking expired ones, we start from std::map::begin() and stop at the fist non-expired one
        std::map<uint32_t, RespondHandler> m_RespondHandlerGroup;

    public:
        explicit ActorPod(uint64_t,
                const std::function<void()> &,
                const std::function<void(const MessagePack &)> &, uint32_t = 3600 * 1000);                            

    public:
        ~ActorPod();

    private:
        uint32_t GetValidID();

    private:
        void InnHandler(const MessagePack &);

    public:
        bool Forward(uint64_t nUID, const MessageBuf &rstMB)
        {
            return Forward(nUID, rstMB, 0);
        }

    public:
        bool Forward(uint64_t nUID, const MessageBuf &rstMB, std::function<void(const MessagePack &)> fnOPR)
        {
            return Forward(nUID, rstMB, 0, std::move(fnOPR));
        }

    public:
        bool Forward(uint64_t, const MessageBuf &, uint32_t);
        bool Forward(uint64_t, const MessageBuf &, uint32_t, std::function<void(const MessagePack &)>);

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    public:
        void Detach() const;

    public:
        uint32_t GetMessageCount() const
        {
            return 0;
        }
};
