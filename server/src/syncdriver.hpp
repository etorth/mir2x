#pragma once
#include <cstdint>
#include "receiver.hpp"
#include "actormsgpack.hpp"

class SyncDriver final
{
    private:
        uint64_t m_currID = 1;

    private:
        Receiver m_receiver;

    public:
        SyncDriver()
            : m_receiver()
        {}

    public:
        ~SyncDriver() = default;

    public:
        uint64_t UID() const
        {
            return m_receiver.UID();
        }

    public:
        ActorMsgPack forward(uint64_t, const ActorMsgBuf &, uint64_t = 0, uint64_t = 0);
};
