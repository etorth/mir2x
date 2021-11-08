#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include <initializer_list>
#include "sysconst.hpp"
#include "protocoldef.hpp"

enum BuffModifierType: int
{
    BMOD_NONE  = 0,
    BMOD_BEGIN = 1,
    BMOD_HP    = 1,
    BMOD_HPMAX,
    BMOD_MP,
    BMOD_MPMAX,
    BMOD_END,
};

constexpr bool validBuffModifier(int bmod)
{
    return bmod >= BMOD_BEGIN && bmod < BMOD_END;
}

struct BuffModifierRecord
{
    const int type = 0;
    const int arg  = 0;
};

enum BuffTriggerType: int
{
    BTGR_NONE  = 0,
    BTGR_BEGIN = 1,

    BTGR_TIME   = 1 << 0,
    BTGR_MOVE   = 1 << 1,
    BTGR_ATTACK = 1 << 2,
    BTGR_HITTED = 1 << 3,

    BTGR_END,
};

constexpr bool validBuffTrigger(int btgr)
{
    return (btgr & (~((BTGR_END - 1) | (BTGR_END - 2)))) == 0;
}

struct BuffTriggerRecord
{
    const char8_t * const name = nullptr;

    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};

struct BuffTriggerRecordRef
{
    const char8_t * const name = nullptr;

    const int on  = 0;
    const int tps = 0;
    const int arg = 0;

    operator bool () const
    {
        return name && !std::u8string_view(name).empty();
    }
};

struct BuffRecord
{
    const char8_t * const name = nullptr;

    const int negative = 0;
    const int disperse = 0;

    const uint32_t time = 0;
    const uint32_t gfxID = SYS_TEXNIL;

    const std::initializer_list<BuffModifierRecord  > modList {};
    const std::initializer_list<BuffTriggerRecordRef> tgrList {};

    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};
