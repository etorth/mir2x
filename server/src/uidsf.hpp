#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>

namespace uidsf
{
    size_t peerCount();
    size_t peerIndex();
    size_t pickPeerIndex(int, std::optional<size_t> = std::nullopt);

    uint64_t getMapBaseUID(uint32_t);
    uint64_t getPeerCoreUID();
    bool isLocalUID(uint64_t);
}
