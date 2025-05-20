#pragma once
#include <cstdint>
#include "actormsgbuf.hpp"

class Dispatcher
{
    public:
        Dispatcher() = default;

    public:
        virtual ~Dispatcher() = default;

    public:
        bool post(uint64_t, const ActorMsgBuf &, uint64_t = 0);

    public:
        bool post(const std::pair<uint64_t, uint64_t> &fromAddr, const ActorMsgBuf &mbuf)
        {
            return post(fromAddr.first, mbuf, fromAddr.second);
        }
};
