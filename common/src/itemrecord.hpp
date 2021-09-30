/*
 * =====================================================================================
 *
 *       Filename: itemrecord.hpp
 *        Created: 07/28/2017 17:12:29
 *    Description: for each item I have two GfxID:
 *                      PkgGfxID : when on ground and in inventory
 *                      UseGfxID : when drawing with player body
 *
 *                 why should I introduce all two different IDs?
 *
 *                 Best practice should be one unique GfxID for all items and retrieve
 *                 different set of animations from different archieves using the unique
 *                 GfxID.
 *
 *                 But in ghe graphical libraries, one item may share one GfxID for one
 *                 kind of usage but own its own resource for another usage. i.e. for the
 *                 诅咒屠龙 and 幸运屠龙, when drawing it with human body they are sharing
 *                 one set of animation, but when staying in inventory they have their own
 *                 different animations.
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

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

    const int weight = 1;
    const int pkgGfxID = 0;
    const uint32_t shape = 0;

    const struct EquipAttribute
    {
        const int duration = 0;

        const int  ac[2] = {0, 0};
        const int  dc[2] = {0, 0};
        const int mac[2] = {0, 0};
        const int mdc[2] = {0, 0};
        const int sac[2] = {0, 0};

        const int hit     = 0;
        const int dodge   = 0;
        const int speed   = 0;
        const int comfort = 0;

        const struct AddLoad
        {
            int hand      = 0;
            int head      = 0;
            int inventory = 0;
        }
        load {};

        const struct EquipReq
        {
            int ac    = 0;
            int mac   = 0;
            int sac   = 0;
            int level = 0;
            const char8_t * const job = nullptr;
        }
        req {};
    }
    equip {};

    const struct PotionAttribute
    {
        int hp   = 0;
        int mp   = 0;
        int time = 0;
    }
    potion {};

    const struct PoisonAttribute
    {
        int dose = 0;
    }
    poison {};

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
};
