#pragma once
#include <queue>
#include <atomic>
#include "uidf.hpp"
#include "actorpod.hpp"
#include "actormsgpack.hpp"
#include "delaycommand.hpp"
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
        friend class ServerObjectLuaThreadRunner;

    private:
        const uint64_t m_UID;

    protected:
        ActorPod *m_actorPod = nullptr;

    protected:
        StateTrigger m_stateTrigger;

    protected:
        DelayCommandQueue m_delayCmdQ;

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
        uint64_t activate(double metronomeFreq = 10.0, uint64_t expireTime = 3600ULL * 1000 * 10000 * 1000);

    protected:
        virtual void onActivate() {}

    protected:
        void deactivate();

    public:
        bool hasActorPod() const
        {
            return m_actorPod && m_actorPod->UID();
        }

    public:
        virtual void operateAM(const ActorMsgPack &) = 0;

    public:
        auto addDelay(uint64_t delayTick, std::function<void()> cmd)
        {
            return m_delayCmdQ.addDelay(delayTick, std::move(cmd));
        }

        void removeDelay(const std::pair<uint32_t, uint64_t> &key)
        {
            m_delayCmdQ.removeDelay(key);
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
