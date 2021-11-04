#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include "protocoldef.hpp"

struct BuffRecord
{
    const char8_t * const name = nullptr;

    const uint32_t gfxID = SYS_TEXNIL;

    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};
