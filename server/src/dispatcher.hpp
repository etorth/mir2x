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
        bool forward(uint64_t, const ActorMsgBuf &, uint32_t = 0);
};
