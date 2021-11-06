#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include "sysconst.hpp"
#include "protocoldef.hpp"

struct BuffRecord
{
    const char8_t * const name = nullptr;

    const uint32_t tps  = 0; // trigger per second
    const uint32_t time = 0;

    const uint32_t gfxID = SYS_TEXNIL;

    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};
