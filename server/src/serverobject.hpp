#pragma once
#include <queue>
#include <atomic>
#include "uidf.hpp"
#include "actorpod.hpp"
#include "actormsgpack.hpp"
#include "delaycommand.hpp"
#include "statetrigger.hpp"

class ServerObject
{
    private:
        const uint64_t m_UID;
        const std::string m_UIDName;

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

        const char *UIDName() const
        {
            return hasActorPod() ? m_UIDName.c_str() : "UID_INACTIVE";
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
        void addDelay(uint32_t delayTick, std::function<void()> cmd)
        {
            m_delayCmdQ.addDelay(delayTick, std::move(cmd));
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

        template<typename T> void forwardNetPackage(uint64_t uid, uint8_t type, const T &t)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            forwardNetPackage(uid, type, &t, sizeof(t));
        }
};
