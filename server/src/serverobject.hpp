#pragma once
#include <queue>
#include <atomic>
#include "uidf.hpp"
#include "actorpod.hpp"
#include "actormsgpack.hpp"
#include "statetrigger.hpp"
#include "serverluacoroutinerunner.hpp"

class ServerObject
{
    protected:
        class LuaThreadRunner: public ServerLuaCoroutineRunner
        {
            public:
                LuaThreadRunner(ServerObject *);

            public:
                ServerObject *getSO() const
                {
                    return m_actorPod->getSO();
                }
        };

    private:
        friend class ActorPod;
        friend class ServerObjectLuaThreadRunner;

    private:
        const uint64_t m_UID;

    protected:
        ActorPod *m_actorPod = nullptr;

    protected:
        StateTrigger m_stateTrigger;

    public:
        ServerObject(uint64_t);

    public:
        virtual ~ServerObject();

    public:
        uint64_t UID() const
        {
            return rawUID();
        }

        uint64_t rawUID() const
        {
            return m_UID;
        }

    public:
        uint64_t activate(double metronomeFreq = 10.0);

    protected:
        virtual corof::awaitable<> onActivate()
        {
            return {};
        }

    protected:
        void deactivate();

    public:
        bool hasActorPod() const
        {
            return m_actorPod && m_actorPod->UID();
        }

    public:
        virtual corof::awaitable<> onActorMsg(const ActorMsgPack &) = 0;

    public:
        virtual void afterActorMsg()
        {
            m_stateTrigger.run();
        }

    public:
        corof::awaitable<bool> asyncWait(uint64_t tick)
        {
            switch(const auto mpk = co_await m_actorPod->wait(tick); mpk.type()){
                case AM_TIMEOUT:
                    {
                        co_return true;
                    }
                default:
                    {
                        co_return false;
                    }
            }
        }

    public:
        auto defer(std::function<corof::awaitable<>()> cmd)
        {
            m_stateTrigger.install([cmd = std::move(cmd)]() -> bool
            {
                cmd().resume();
                return true;
            });
        }

        auto addDelay(uint64_t delayTick, std::function<corof::awaitable<>()> cmd)
        {
            const auto token = m_actorPod->createWaitToken(delayTick);
            defer([token, cmd = std::move(cmd), thisptr = this]([[maybe_unused]] this auto self) -> corof::awaitable<>
            {
                co_await thisptr->m_actorPod->waitToken(token);
                co_await cmd();
            });
            return token;
        }

        void removeDelay(const std::pair<uint64_t, uint64_t> &token)
        {
            m_actorPod->cancelWaitToken(token);
        }

    protected:
        void forwardNetPackage(uint64_t, uint8_t, const void *, size_t);

    protected:
        void forwardNetPackage(uint64_t uid, uint8_t type)
        {
            forwardNetPackage(uid, type, nullptr, 0);
        }

        void forwardNetPackage(uint64_t uid, uint8_t type, const std::string &buf)
        {
            forwardNetPackage(uid, type, buf.data(), buf.length());
        }

        void forwardNetPackage(uint64_t uid, uint8_t type, const std::string_view &buf)
        {
            forwardNetPackage(uid, type, buf.data(), buf.size());
        }

        template<typename T> void forwardNetPackage(uint64_t uid, uint8_t type, const T &t)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            forwardNetPackage(uid, type, &t, sizeof(t));
        }
};
