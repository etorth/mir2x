#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include <initializer_list>
#include "sysconst.hpp"
#include "protocoldef.hpp"

enum BuffActTriggerType: int
{
    BATGR_NONE  = 0,
    BATGR_BEGIN = 1,

    BATGR_TIME   = 1 << 0,
    BATGR_MOVE   = 1 << 1,
    BATGR_ATTACK = 1 << 2,
    BATGR_HITTED = 1 << 3,

    BATGR_END,
};

constexpr bool validBuffActTrigger(int btgr)
{
    return (btgr & (~((BATGR_END - 1) | (BATGR_END - 2)))) == 0;
}

struct AttackModifierRecord
{
    const char8_t * const name = nullptr;
    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};

struct SpellModifierRecord
{
    const char8_t * const name = nullptr;
    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }
};

struct BuffActRecord
{
    const char8_t * const name = nullptr;
    const char8_t * const type = nullptr; // 光环/控制/触发/属性修改/攻击修改/施法修改

    const struct BuffActAuraParam
    {
        const char8_t * const buff = nullptr;
        const int self = 0;
    }
    aura {};

    const struct BuffActControllerParam
    {
        const struct CanDoParam
        {
            const int move   : 2 = 0;
            const int attack : 2 = 0;
            const int spell  : 2 = 0;
            const int hitted : 2 = 0;
            const int item   : 2 = 0;
        }
        can {};
    }
    controller {};

    const struct BuffActTriggerParam
    {
    }
    trigger {};

    const struct BuffActAttackModifierParam
    {
        const char8_t * const buff     = nullptr;
        const char8_t * const modifier = nullptr;
    }
    attackModifier {};

    const struct BuffActSpellModifierParam
    {
        const char8_t * const buff     = nullptr;
        const char8_t * const modifier = nullptr;
    }
    spellModifier {};

    const struct BuffActAttributeModifierParam
    {
        const char8_t * const name = nullptr;
    }
    attributeModifier {};

    operator bool () const
    {
        return true
            && name && !std::u8string_view(name).empty()
            && type && !std::u8string_view(type).empty();
    }

    constexpr bool isBuffAct(const char8_t *actName) const
    {
        return actName && std::u8string_view(name) == actName;
    }

    constexpr bool isAura() const
    {
        return type && std::u8string_view(type) == u8"光环";
    }

    constexpr bool isController() const
    {
        return type && std::u8string_view(type) == u8"控制";
    }

    constexpr bool isTrigger() const
    {
        return type && std::u8string_view(type) == u8"触发";
    }

    constexpr bool isAttributeModifier() const
    {
        return type && std::u8string_view(type) == u8"属性修改";
    }

    constexpr bool isAttackModifier() const
    {
        return type && std::u8string_view(type) == u8"攻击修改";
    }

    constexpr bool isSpellModifier() const
    {
        return type && std::u8string_view(type) == u8"施法修改";
    }
};

struct BuffRecord
{
    const char8_t * const name = nullptr;

    const int favor       : 2 = 0; // -1: debuff, 0: neutral, 1: buff
    const int stackable   : 8 = 0;
    const int dispellable : 2 = 0;

    const struct IconParam
    {
        const int show : 2 = 0;
        const uint32_t gfxID = SYS_TEXNIL;
    }
    icon {};

    struct BuffActRecordRef
    {
        const char8_t * const name = nullptr;

        // every buff has an actList and each act in the list has different duration
        // duration in ms < 0: unlimited
        //                  0: one-shot act
        //                > 0: limited timeout
        const int duration = -1;

        const struct BuffActAuraParam
        {
            const int radius = 0;
            const struct LingerParam
            {
                const int in  = 0;
                const int out = 0;
            }
            linger {};
        }
        aura {};

        const struct BuffActControllerParam
        {
        }
        controller {};

        const struct BuffActTriggerParam
        {
            const int on = 0;
            const int tps = 0;
        }
        trigger {};

        const struct BuffActAttackModifierParam
        {
            const int prob = 0;
        }
        attackModifier {};

        const struct BuffActSpellModifierParam
        {
        }
        spellModifier {};

        const struct BuffActAttributeModifierParam
        {
            const int value = 0;
            const int percentage = 0;
        }
        attributeModifier {};

        constexpr bool isBuffActRef(const char8_t *actName) const
        {
            return name && actName && std::u8string_view(name) == actName;
        }

        operator bool() const
        {
            return name && !std::u8string_view(name).empty();
        }
    };
    const std::initializer_list<BuffActRecordRef> actList {};

    operator bool() const
    {
        return name && !std::u8string_view(name).empty();
    }

    constexpr bool isBuff(const char8_t *buffName) const
    {
        return name && buffName && std::u8string_view(name) == buffName;
    }

    constexpr bool hasBuffAct(const char8_t *actName) const
    {
        for(const auto &baref: actList){
            if(baref.isBuffActRef(actName)){
                return true;
            }
        }
        return false;
    }
};
