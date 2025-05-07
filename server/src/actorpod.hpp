#pragma once
#include <array>
#include <string>
#include <cfloat>
#include <variant>
#include <coroutine>
#include <functional>
#include <unordered_map>

#include "corof.hpp"
#include "actormsgbuf.hpp"
#include "actormsgpack.hpp"
#include "actormonitor.hpp"

class ServerObject;
class ActorPod final
{
    private:
        friend class ActorPool;

    private:
        struct RegisterContinuationAwaiter
        {
            ActorPod *         const actor;
            std::optional<int> const seqID;

            bool await_ready() const
            {
                return !seqID.has_value();
            }

            void await_suspend(std::coroutine_handle<corof::awaitable<ActorMsgPack>::promise_type> handle)
            {
                if(seqID.has_value()){
                    actor->m_respondCBList.emplace(seqID.value(), handle);
                }
            }

            void await_resume() const noexcept {}
        };

    private:
        const uint64_t m_UID;

    private:
        ServerObject * const m_SO;

    private:
        std::array<std::function<corof::awaitable<>(const ActorMsgPack &)>, AM_END> m_msgOpList;

    private:
        // actorpool automatically send METRONOME to actor
        // this can control the freq if we want to setup actor in standby/active mode, zero or negative means not send METRONOME
        double m_updateFreq;

    private:
        // used by rollSeqID()
        // to create unique proper ID for an message expcecting response
        uint64_t m_nextSeqID = 1;

    private:
        std::unordered_map<uint64_t, std::variant<std::coroutine_handle<corof::awaitable<ActorMsgPack>::promise_type>, std::function<void(const ActorMsgPack &)>>> m_respondCBList;

    private:
        ActorPodMonitor m_podMonitor;

    private:
        uint32_t m_channID = 0;

    public:
        explicit ActorPod(uint64_t, ServerObject *, double);

    public:
        ~ActorPod();

    private:
        uint64_t rollSeqID();

    private:
        void innHandler(const ActorMsgPack &);

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    public:
        void attach();
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
        void registerOp(int type, std::function<corof::awaitable<>(const ActorMsgPack &)> op)
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

    private:
        std::optional<uint64_t> doPost(const std::pair<uint64_t, uint64_t> &, ActorMsgBuf, bool);

    public:
        bool post(const std::pair<uint64_t, uint64_t> &addr, ActorMsgBuf mbuf)
        {
            return doPost(addr, std::move(mbuf), false).has_value();
        }

        bool post(uint64_t addr, ActorMsgBuf mbuf)
        {
            return post({addr, 0}, std::move(mbuf));
        }

    public:
        corof::awaitable<ActorMsgPack> send(const std::pair<uint64_t, uint64_t> &addr, ActorMsgBuf mbuf)
        {
            co_await RegisterContinuationAwaiter
            {
                .actor = this,
                .seqID = doPost(addr, std::move(mbuf), true),
            };
        }

        corof::awaitable<ActorMsgPack> send(uint64_t addr, ActorMsgBuf mbuf)
        {
            co_await RegisterContinuationAwaiter
            {
                .actor = this,
                .seqID = doPost({addr, 0}, std::move(mbuf), true),
            };
        }

    public:
        std::pair<uint64_t, uint64_t> createWaitToken(uint64_t, std::function<void(const ActorMsgPack &)>);
        bool cancelWaitToken(const std::pair<uint64_t, uint64_t> &);

    public:
        corof::awaitable<ActorMsgPack> wait(uint64_t tick)
        {
            co_await RegisterContinuationAwaiter
            {
                .actor = this,
                .seqID = createWaitToken(tick, nullptr).second,
            };
        }

        corof::awaitable<ActorMsgPack> waitToken(const std::pair<uint64_t, uint64_t> &token)
        {
            co_await RegisterContinuationAwaiter
            {
                .actor = this,
                .seqID = token.second,
            };
        }
};
