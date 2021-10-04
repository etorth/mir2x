#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include "protocoldef.hpp"

struct ItemRecord
{
    const char8_t * const name   = nullptr;
    const char8_t * const type   = nullptr;
    const char8_t * const rarity = nullptr;

    const int weight = 0;
    const int pkgGfxID = 0;
    const uint32_t shape = 0;

    const struct EquipAttribute
    {
        const int duration = 0;

        const int dc[2] = {0, 0};
        const int mc[2] = {0, 0};
        const int sc[2] = {0, 0};

        const int  ac[2] = {0, 0};
        const int mac[2] = {0, 0};

        const int hit = 0;

        const int dcDodge = 0;
        const int mcDodge = 0;

        const int speed   = 0;
        const int comfort = 0;

        const int hpRecover = 0;
        const int mpRecover = 0;

        const int luckCurse = 0;

        const struct AddElem
        {
            const int fire    = 0;
            const int ice     = 0;
            const int light   = 0;
            const int wind    = 0;
            const int holy    = 0;
            const int dark    = 0;
            const int phantom = 0;
        }
        elem {};

        const struct AddLoad
        {
            const int hand      = 0;
            const int body      = 0;
            const int inventory = 0;
        }
        load {};

        const struct EquipReq
        {
            const int dc = 0;
            const int mc = 0;
            const int sc = 0;
            const int level = 0;
            const char8_t * const job = nullptr;
        }
        req {};
    }
    equip {};

    const struct PotionAttribute
    {
        const int hp   = 0;
        const int mp   = 0;
        const int time = 0;
    }
    potion {};

    const struct DopeAttribute
    {
        const int  hp = 0;
        const int  mp = 0;

        const int dc = 0;
        const int mc = 0;
        const int sc = 0;

        const int speed = 0;
        const int time  = 0;
    }
    dope {};

    const struct PoisonAttribute
    {
        const int dose = 0;
    }
    poison {};

    const struct BookAttribute
    {
        const char8_t * job = nullptr;
        const int level = 0;
    }
    book {};

    const char8_t * const description = nullptr;

    operator bool() const
    {
        return name && std::u8string_view(name) != u8"";
    }

    constexpr bool packable() const
    {
        return false
            || std::u8string_view(type) == u8"恢复药水"
            || std::u8string_view(type) == u8"传送卷轴"
            || std::u8string_view(type) == u8"护身符"
            || std::u8string_view(type) == u8"药粉";
    }

    constexpr bool beltable() const
    {
        return false
            || std::u8string_view(type) == u8"恢复药水"
            || std::u8string_view(type) == u8"传送卷轴";
    }

    constexpr bool wearable(int wltype) const
    {
        switch(wltype){
            case WLG_DRESS   : return std::u8string_view(u8"衣服") == type;
            case WLG_HELMET  : return std::u8string_view(u8"头盔") == type;
            case WLG_WEAPON  : return std::u8string_view(u8"武器") == type;
            case WLG_SHOES   : return std::u8string_view(u8"鞋"  ) == type;
            case WLG_NECKLACE: return std::u8string_view(u8"项链") == type;
            case WLG_ARMRING0: return std::u8string_view(u8"手镯") == type;
            case WLG_ARMRING1: return std::u8string_view(u8"手镯") == type;
            case WLG_RING0   : return std::u8string_view(u8"戒指") == type;
            case WLG_RING1   : return std::u8string_view(u8"戒指") == type;
            case WLG_TORCH   : return std::u8string_view(u8"火把") == type;
            case WLG_CHARM   : return std::u8string_view(u8"魅力|护身符|药粉").find(type) != std::u8string_view::npos;
            default          : return false;
        }
    }

    constexpr bool isGold() const
    {
        return std::u8string_view(type) == u8"金币";
    }

    constexpr bool isDress() const
    {
        return std::u8string_view(type) == u8"衣服";
    }

    constexpr bool isWeapon() const
    {
        return std::u8string_view(type) == u8"武器";
    }
};
