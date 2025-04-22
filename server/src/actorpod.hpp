#pragma once
#include <array>
#include <string>
#include <cfloat>
#include <functional>
#include <unordered_map>

#include "actormsgbuf.hpp"
#include "actormsgpack.hpp"
#include "actormonitor.hpp"

class ServerObject;
class ActorPod final
{
    private:
        friend class ActorPool;

    private:
        const uint64_t m_UID;

    private:
        ServerObject * const m_SO;

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
        std::array<std::function<void(const ActorMsgPack &)>, AM_END> m_msgOpList;

    private:
        // actorpool automatically send METRONOME to actor
        // this can control the freq if we want to setup actor in standby/active mode, zero or negative means not send METRONOME
        double m_updateFreq;

    private:
        // used by rollSeqID()
        // to create unique proper ID for an message expcecting response
        uint64_t m_nextSeqID = 1;

    private:
        std::unordered_map<uint64_t, std::function<void(const ActorMsgPack &)>> m_respondCBList;

    private:
        ActorPodMonitor m_podMonitor;

    private:
        uint32_t m_channID = 0;

    public:
        explicit ActorPod(
                uint64_t,                                   // UID
                ServerObject *,                             // SO
                std::function<void()>,                      // trigger
                std::function<void(const ActorMsgPack &)>,  // msgHandler
                double);                                    // metronome frequency in HZ

    public:
        ~ActorPod();

    private:
        uint64_t rollSeqID();

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
        bool forward(uint64_t, const ActorMsgBuf &, uint64_t);
        bool forward(uint64_t, const ActorMsgBuf &, uint64_t, std::function<void(const ActorMsgPack &)>);

    public:
        bool forward(const std::pair<uint64_t, uint64_t> &respAddr, const ActorMsgBuf &mbuf)
        {
            return forward(respAddr.first, mbuf, respAddr.second);
        }

        bool forward(const std::pair<uint64_t, uint64_t> &respAddr, const ActorMsgBuf &mbuf, std::function<void(const ActorMsgPack &)> fnOPR)
        {
            return forward(respAddr.first, mbuf, respAddr.second, std::move(fnOPR));
        }

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

    public:
        ServerObject *getSO() const
        {
            return m_SO;
        }

    public:
        void registerOp(int type, std::function<void(const ActorMsgPack &)> op)
        {
            if(op){
                m_msgOpList.at(type) = std::move(op);
            }
        }

    public:
        void postNet(          uint8_t, const void *, size_t, uint64_t);
        void postNet(uint32_t, uint8_t, const void *, size_t, uint64_t);

    public:
        void  bindNet(uint32_t);
        void closeNet();
};
