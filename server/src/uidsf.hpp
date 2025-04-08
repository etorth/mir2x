#pragma once
#include <cstddef>
#include <cstdint>

namespace uidsf
{
    bool isLocalUID(uint64_t);
    size_t peerIndex(uint64_t);
    uint64_t getServiceCoreUID();
}
