#pragma once
#include <cstddef>
#include <cstdint>

namespace uidsf
{
    size_t peerIndex();
    size_t peerIndex(uint64_t);

    bool isLocalUID(uint64_t);
    uint64_t getServiceCoreUID();
}
