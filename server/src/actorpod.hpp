/*
 * =====================================================================================
 *
 *       Filename: actorpod.hpp
 *        Created: 04/20/2016 21:49:14
 *    Description:
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
#include <array>
#include <string>
#include <cfloat>
#include <functional>

#include "actormsgbuf.hpp"
#include "actormsgpack.hpp"
#include "actormonitor.hpp"

class ActorPod final
{
    private:
        friend class ActorPool;

    private:
        struct RespondCallback
        {
            uint64_t expireTime;
            std::function<void(const ActorMsgPack &)> op;

            RespondCallback(uint64_t argExpireTime, std::function<void(const ActorMsgPack &)> argOp)
                : expireTime(argExpireTime)
                , op(std::move(argOp))
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
        // for that we should register / de-register those triggers to m_trigger
        // most likely here we use StateHook::Execute();
        //
        // trigger is provided at initialization and never change
        const std::function<void()> m_trigger;

        // handler to handle every informing messages
        // informing messges means we didn't register an handler for it
        // this handler is provided at the initialization time and never change
        const std::function<void(const ActorMsgPack &)> m_operation;

    private:
        // actorpool automatically send METRONOME to actor
        // this can control the freq if we want to setup actor in standby/active mode, zero or negative means not send METRONOME
        double m_updateFreq;

    private:
        // used by rollSeqID()
        // to create unique proper ID for an message expcecting response
        uint32_t m_nextSeqID = 1;

        // for expire time check in nanoseconds
        // zero expire time means we never expire any handler for current pod
        // we can put argument to specify the expire time of each handler but not necessary
        const uint64_t m_expireTime;

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
        std::map<uint32_t, RespondCallback> m_respondCBList;

    private:
        ActorPodMonitor m_podMonitor;

    public:
        explicit ActorPod(uint64_t,                         // UID
                std::function<void()>,                      // trigger
                std::function<void(const ActorMsgPack &)>,  // msgHandler
                double,                                     // metronome frequency in HZ
                uint64_t);                                  // timeout in nanoseconds

    public:
        ~ActorPod();

    private:
        uint32_t rollSeqID();

    private:
        void innHandler(const ActorMsgPack &);

    public:
        bool forward(uint64_t nUID, const ActorMsgBuf &mbuf)
        {
            return forward(nUID, mbuf, 0);
        }

    public:
        bool forward(uint64_t nUID, const ActorMsgBuf &mbuf, std::function<void(const ActorMsgPack &)> fnOPR)
        {
            return forward(nUID, mbuf, 0, std::move(fnOPR));
        }

    public:
        bool forward(uint64_t, const ActorMsgBuf &, uint32_t);
        bool forward(uint64_t, const ActorMsgBuf &, uint32_t, std::function<void(const ActorMsgPack &)>);

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    public:
        void attach(std::function<void()>);
        void detach(std::function<void()>) const;

    public:
        static bool checkUIDValid(uint64_t);

    public:
        void PrintMonitor() const;

    public:
        ActorPodMonitor dumpPodMonitor() const
        {
            return m_podMonitor;
        }

    public:
        void setMetronomeFreq(double freq)
        {
            // TODO enhance it to do more check
            // should only call this function inside message handler
            m_updateFreq = regMetronomeFreq(freq);
        }

        double getMetronomeFreq() const
        {
            return m_updateFreq;
        }

    private:
        static double regMetronomeFreq(double freq)
        {
            if(freq <= 0.0){
                return -1.0;
            }
            else if(freq < DBL_EPSILON){
                return DBL_EPSILON;
            }
            else{
                return freq;
            }
        }
};
